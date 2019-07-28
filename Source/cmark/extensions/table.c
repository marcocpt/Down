#include <cmark-gfm-extension_api.h>
#include <html.h>
#include <inlines.h>
#include <parser.h>
#include <references.h>
#include <string.h>
#include <render.h>

#include "ext_scanners.h"
#include "strikethrough.h"
#include "table.h"
#include "cmark-gfm-core-extensions.h"

cmark_node_type CMARK_NODE_TABLE, CMARK_NODE_TABLE_ROW,
    CMARK_NODE_TABLE_CELL;

/// 包含列数、软中断偏移和单元格链表(data 类型为 node_cell)
///
/// 主要用于生成 CMARK_NODE_TABLE_CELL ，不存储
/// 获取单元格字符串：`p ((node_cell *)<#table_row *row#>->cells->data)->buf->ptr`
typedef struct {
  uint16_t n_columns;       /**< 一行中单元格的列个数 */
  int paragraph_offset;     /**< 段落偏移。当一行中有软中断时，保存软中断的偏移位置 */
  cmark_llist *cells;       /**< 一行中的 node_cell 链表 */
} table_row;


typedef struct {
  uint16_t n_columns;       /**< 表格节点中的列个数 */
  uint8_t *alignments;      /**< 表格节点中所有列的对齐方式, 例："rclr"  */
} node_table;

/// 存储 is_header
typedef struct {
  bool is_header;           /**< 是表头 */
} node_table_row;

typedef struct {
  cmark_strbuf *buf;        /**< 存字符串 */
  int start_offset;         /**< 在行中单元格内容的起始偏移位置，包括不可见字符 */
  int end_offset;           /**< 在行中单元格内容的结束偏移位置 */
  int internal_offset;      /**< 记录第一个可见字符到 '|' 间的偏移 */
} node_cell;

// MARK: - -private start-
/// 释放表格的单元格
static void free_table_cell(cmark_mem *mem, void *data) {
  node_cell *cell = (node_cell *)data;
  cmark_strbuf_free((cmark_strbuf *)cell->buf);
  mem->free(cell->buf);
  mem->free(cell);
}

/// 释放表格的行
static void free_table_row(cmark_mem *mem, table_row *row) {
  if (!row)
    return;

  cmark_llist_free_full(mem, row->cells, (cmark_free_func)free_table_cell);

  mem->free(row);
}

/// 释放表格节点
static void free_node_table(cmark_mem *mem, void *ptr) {
  node_table *t = (node_table *)ptr;
  mem->free(t->alignments);
  mem->free(t);
}

/// 释放表格行节点
static void free_node_table_row(cmark_mem *mem, void *ptr) {
  mem->free(ptr);
}

/// 获得表格的列数。通过使用节点的 as.opaque 属性获取
static int get_n_table_columns(cmark_node *node) {
  if (!node || node->type != CMARK_NODE_TABLE)
    return -1;

  return (int)((node_table *)node->as.opaque)->n_columns;
}

/// 设置表格的列数。数据存储在 `((node_table *)node->as.opaque)->n_columns`
static int set_n_table_columns(cmark_node *node, uint16_t n_columns) {
  if (!node || node->type != CMARK_NODE_TABLE)
    return 0;

  ((node_table *)node->as.opaque)->n_columns = n_columns;
  return 1;
}

/// 获得表格的对齐方式
static uint8_t *get_table_alignments(cmark_node *node) {
  if (!node || node->type != CMARK_NODE_TABLE)
    return 0;

  return ((node_table *)node->as.opaque)->alignments;
}

/// 设置表格的对齐方式
static int set_table_alignments(cmark_node *node, uint8_t *alignments) {
  if (!node || node->type != CMARK_NODE_TABLE)
    return 0;

  ((node_table *)node->as.opaque)->alignments = alignments;
  return 1;
}

/// 转义 string 中的 '\|'
static cmark_strbuf *unescape_pipes(cmark_mem *mem, unsigned char *string, bufsize_t len)
{
  cmark_strbuf *res = (cmark_strbuf *)mem->calloc(1, sizeof(cmark_strbuf));
  bufsize_t r, w;

  cmark_strbuf_init(mem, res, len + 1);
  cmark_strbuf_put(res, string, len);
  cmark_strbuf_putc(res, '\0');
    
  // 当遇到字符 '\|' 时，去掉 '\'
  for (r = 0, w = 0; r < len; ++r) {
    if (res->ptr[r] == '\\' && res->ptr[r + 1] == '|')
      r++;

    res->ptr[w++] = res->ptr[r];
  }

  cmark_strbuf_truncate(res, w);

  return res;
}

/**
 将 string 的内容转换为 table_row 输出。

 @param self cmark_syntax_extension 未使用
 @param parser 解析器
 @param string 待转换的字符串
 @param len string 的长度
 @return string 生成的 table_row
 */
static table_row *row_from_string(cmark_syntax_extension *self,
                                  cmark_parser *parser, unsigned char *string,
                                  int len) {
  table_row *row = NULL;        /**< 返回值 */
  bufsize_t cell_matched = 1;   /**< 匹配单元格中非空字符开始到末尾或 ’|‘ 间的单元格长度 */
  bufsize_t pipe_matched = 1;   /**< 匹配 ’|‘ 到下一个非空字符的长度 */
  bufsize_t offset;
  int cell_end_offset;

  row = (table_row *)parser->mem->calloc(1, sizeof(table_row));
  row->n_columns = 0;
  row->cells = NULL;

  offset = scan_table_cell_end(string, len, 0);

  // Parse the cells of the row. Stop if we reach the end of the input, or if we
  // cannot detect any more cells.
  while (offset < len && (cell_matched || pipe_matched)) {
    cell_matched = scan_table_cell(string, len, offset);
    pipe_matched = scan_table_cell_end(string, len, offset + cell_matched);

    if (cell_matched || pipe_matched) {
      cell_end_offset = offset + cell_matched - 1;
        
      // 如果遇到软中断，就丢弃（会处理为段落），并记录偏移到 paragraph_offset
      if (string[cell_end_offset] == '\n' || string[cell_end_offset] == '\r') {
        row->paragraph_offset = cell_end_offset;

        cmark_llist_free_full(parser->mem, row->cells, (cmark_free_func)free_table_cell);
        row->cells = NULL;
        row->n_columns = 0;
      } else {
        cmark_strbuf *cell_buf = unescape_pipes(parser->mem, string + offset,
            cell_matched);
        cmark_strbuf_trim(cell_buf);

        node_cell *cell = (node_cell *)parser->mem->calloc(1, sizeof(*cell));
        cell->buf = cell_buf;
        cell->start_offset = offset;
        cell->end_offset = cell_end_offset;
        // 移除每个单元格中 '|' 后的空字符
        while (cell->start_offset > 0 && string[cell->start_offset - 1] != '|') {
          --cell->start_offset;
          ++cell->internal_offset;
        }

        row->n_columns += 1;
        row->cells = cmark_llist_append(parser->mem, row->cells, cell);
      }
    }

    offset += cell_matched + pipe_matched;
    // 处理行尾
    if (!pipe_matched) {
      pipe_matched = scan_table_row_end(string, len, offset);
      offset += pipe_matched;
    }
  }

  if (offset != len || !row->n_columns) {
    free_table_row(parser->mem, row);
    row = NULL;
  }

  return row;
}

/// 插入表格头部段落。在表格中有软中断时才调用
static void try_inserting_table_header_paragraph(cmark_parser *parser,
                                                 cmark_node *parent_container,
                                                 unsigned char *parent_string,
                                                 int paragraph_offset) {
  cmark_node *paragraph;
  cmark_strbuf *paragraph_content;

  paragraph = cmark_node_new_with_mem(CMARK_NODE_PARAGRAPH, parser->mem);

  paragraph_content = unescape_pipes(parser->mem, parent_string, paragraph_offset);
  cmark_strbuf_trim(paragraph_content);
  cmark_node_set_string_content(paragraph, (char *) paragraph_content->ptr);
  cmark_strbuf_free(paragraph_content);
  parser->mem->free(paragraph_content);
  // 插不进去就释放掉
  if (!cmark_node_insert_before(parent_container, paragraph)) {
    parser->mem->free(paragraph);
  }
}

/**
 private 尝试解析表格标记及表头行，成功则添加到 parent_container 中

 @param self cmark_syntax_extension
 @param parser 解析器
 @param parent_container 父容器节点
 @param input 输入的字符串
 @param len input长度
 @return parent_container
 */
static cmark_node *try_opening_table_header(cmark_syntax_extension *self,
                                            cmark_parser *parser,
                                            cmark_node *parent_container,
                                            unsigned char *input, int len) {
  // 匹配表格标记的长度
  bufsize_t matched =
      scan_table_start(input, len, cmark_parser_get_first_nonspace(parser));
  cmark_node *table_header;         // 表的头结点，父类结点为 table，子类结点为 table_cell
  table_row *header_row = NULL;     // 表头行（单元格第一行）
  table_row *marker_row = NULL;     // 表格标记行 "|---|---|"
  node_table_row *ntr;              // 存储是否为表头
  const char *parent_string;
  uint16_t i;

  if (!matched)
    return parent_container;

  parent_string = cmark_node_get_string_content(parent_container);

  cmark_arena_push();

  header_row = row_from_string(self, parser, (unsigned char *)parent_string,
                               (int)strlen(parent_string));

  if (!header_row) {
    free_table_row(parser->mem, header_row);
    cmark_arena_pop();
    return parent_container;
  }

  marker_row = row_from_string(self, parser,
                               input + cmark_parser_get_first_nonspace(parser),
                               len - cmark_parser_get_first_nonspace(parser));

  assert(marker_row);
  // 表格头的列数与表格标记的列数不匹配就释放
  if (header_row->n_columns != marker_row->n_columns) {
    free_table_row(parser->mem, header_row);
    free_table_row(parser->mem, marker_row);
    cmark_arena_pop();
    return parent_container;
  }

  if (cmark_arena_pop()) {
    header_row = row_from_string(self, parser, (unsigned char *)parent_string,
                                 (int)strlen(parent_string));
    marker_row = row_from_string(self, parser,
                                 input + cmark_parser_get_first_nonspace(parser),
                                 len - cmark_parser_get_first_nonspace(parser));
  }

  if (!cmark_node_set_type(parent_container, CMARK_NODE_TABLE)) {
    free_table_row(parser->mem, header_row);
    free_table_row(parser->mem, marker_row);
    return parent_container;
  }
    
  // 如果行中有软中断，将前面的其转换为段落插入
  if (header_row->paragraph_offset) {
    try_inserting_table_header_paragraph(parser, parent_container, (unsigned char *)parent_string,
                                         header_row->paragraph_offset);
  }

  cmark_node_set_syntax_extension(parent_container, self);

  parent_container->as.opaque = parser->mem->calloc(1, sizeof(node_table));

  set_n_table_columns(parent_container, header_row->n_columns);

  uint8_t *alignments =
      (uint8_t *)parser->mem->calloc(header_row->n_columns, sizeof(uint8_t));
  cmark_llist *it = marker_row->cells;
    
  // 设置表格排列方式
  for (i = 0; it; it = it->next, ++i) {
    node_cell *node = (node_cell *)it->data;
    bool left = node->buf->ptr[0] == ':', right = node->buf->ptr[node->buf->size - 1] == ':';

    if (left && right)
      alignments[i] = 'c';
    else if (left)
      alignments[i] = 'l';
    else if (right)
      alignments[i] = 'r';
  }
  set_table_alignments(parent_container, alignments);

  table_header =
      cmark_parser_add_child(parser, parent_container, CMARK_NODE_TABLE_ROW,
                             parent_container->start_column);
  cmark_node_set_syntax_extension(table_header, self);
  table_header->end_column = parent_container->start_column + (int)strlen(parent_string) - 2;
  table_header->start_line = table_header->end_line = parent_container->start_line;

  table_header->as.opaque = ntr = (node_table_row *)parser->mem->calloc(1, sizeof(node_table_row));
  ntr->is_header = true;

  {
    cmark_llist *tmp;
    // 将单元格转换为节点，添加到 parent_container 中
    for (tmp = header_row->cells; tmp; tmp = tmp->next) {
      node_cell *cell = (node_cell *) tmp->data;
      cmark_node *header_cell = cmark_parser_add_child(parser, table_header,
          CMARK_NODE_TABLE_CELL, parent_container->start_column + cell->start_offset);
      header_cell->start_line = header_cell->end_line = parent_container->start_line;
      header_cell->internal_offset = cell->internal_offset;
      header_cell->end_column = parent_container->start_column + cell->end_offset;
      cmark_node_set_string_content(header_cell, (char *) cell->buf->ptr);
      cmark_node_set_syntax_extension(header_cell, self);
    }
  }

  cmark_parser_advance_offset(
      parser, (char *)input,
      (int)strlen((char *)input) - 1 - cmark_parser_get_offset(parser), false);

  free_table_row(parser->mem, header_row);
  free_table_row(parser->mem, marker_row);
  return parent_container;
}


/**
 尝试解析表格行

 @param self cmark_syntax_extension
 @param parser 解析器
 @param parent_container 父容器，子节点会加入其中
 @param input 待解析的字符串
 @param len input 的长度
 @return 解析完的表格行结点
 */
static cmark_node *try_opening_table_row(cmark_syntax_extension *self,
                                         cmark_parser *parser,
                                         cmark_node *parent_container,
                                         unsigned char *input, int len) {
  cmark_node *table_row_block;      /**< 返回值 */
  table_row *row;                  /**< 使用 input 生成的 table_row */

  if (cmark_parser_is_blank(parser))
    return NULL;

  table_row_block =
      cmark_parser_add_child(parser, parent_container, CMARK_NODE_TABLE_ROW,
                             parent_container->start_column);
  cmark_node_set_syntax_extension(table_row_block, self);
  table_row_block->end_column = parent_container->end_column;
  table_row_block->as.opaque = parser->mem->calloc(1, sizeof(node_table_row));

  row = row_from_string(self, parser, input + cmark_parser_get_first_nonspace(parser),
      len - cmark_parser_get_first_nonspace(parser));

  {
    cmark_llist *tmp;
    int i, table_columns = get_n_table_columns(parent_container);

    for (tmp = row->cells, i = 0; tmp && i < table_columns; tmp = tmp->next, ++i) {
      node_cell *cell = (node_cell *) tmp->data;
      cmark_node *node = cmark_parser_add_child(parser, table_row_block,
          CMARK_NODE_TABLE_CELL, parent_container->start_column + cell->start_offset);
      node->internal_offset = cell->internal_offset;
      node->end_column = parent_container->start_column + cell->end_offset;
      cmark_node_set_string_content(node, (char *) cell->buf->ptr);
      cmark_node_set_syntax_extension(node, self);
    }

    for (; i < table_columns; ++i) {
      cmark_node *node = cmark_parser_add_child(
          parser, table_row_block, CMARK_NODE_TABLE_CELL, 0);
      cmark_node_set_syntax_extension(node, self);
    }
  }

  free_table_row(parser->mem, row);

  cmark_parser_advance_offset(parser, (char *)input,
                              len - 1 - cmark_parser_get_offset(parser), false);

  return table_row_block;
}
// MARK: -private end-

// MARK: - -extension start-
/**
 尝试展开表格块，成功则添加到 parent_container 中

 @param self cmark_syntax_extension
 @param indented 已经解析缩进
 @param parser 解析器
 @param parent_container 父类容器结点
 @param input 输入文本
 @param len input 长度
 @return 依据父节点类型决定。
 */
static cmark_node *try_opening_table_block(cmark_syntax_extension *self,
                                           int indented, cmark_parser *parser,
                                           cmark_node *parent_container,
                                           unsigned char *input, int len) {
  cmark_node_type parent_type = cmark_node_get_type(parent_container);
  // 父节点是段落才能加表头节点，是表才能加行
  if (!indented && parent_type == CMARK_NODE_PARAGRAPH) {
    return try_opening_table_header(self, parser, parent_container, input, len);
  } else if (!indented && parent_type == CMARK_NODE_TABLE) {
    return try_opening_table_row(self, parser, parent_container, input, len);
  }

  return NULL;
}

/**
 parent_container 能够包含 input 转换的子节点，返回1；否则返回0。

 @param self cmark_syntax_extension 未使用
 @param parser 解析器
 @param input 输入的文本
 @param len input 的长度
 @param parent_container 父容器
 @return 匹配成功返回1，否则返回0
 */
static int matches(cmark_syntax_extension *self, cmark_parser *parser,
                   unsigned char *input, int len,
                   cmark_node *parent_container) {
  int res = 0;  // 返回值

  if (cmark_node_get_type(parent_container) == CMARK_NODE_TABLE) {
    cmark_arena_push();
    table_row *new_row = row_from_string(
        self, parser, input + cmark_parser_get_first_nonspace(parser),
        len - cmark_parser_get_first_nonspace(parser));
    if (new_row && new_row->n_columns)
      res = 1;
    free_table_row(parser->mem, new_row);
    cmark_arena_pop();
  }

  return res;
}

/// 获得 node 的类型字符串
static const char *get_type_string(cmark_syntax_extension *self,
                                   cmark_node *node) {
  if (node->type == CMARK_NODE_TABLE) {
    return "table";
  } else if (node->type == CMARK_NODE_TABLE_ROW) {
    if (((node_table_row *)node->as.opaque)->is_header)
      return "table_header";
    else
      return "table_row";
  } else if (node->type == CMARK_NODE_TABLE_CELL) {
    return "table_cell";
  }

  return "<unknown>";
}

/// node 是否能够包含 child_type。
///
/// cmark_syntax_extension->can_contain_func
static int can_contain(cmark_syntax_extension *extension, cmark_node *node,
                       cmark_node_type child_type) {
  if (node->type == CMARK_NODE_TABLE) {
    return child_type == CMARK_NODE_TABLE_ROW;
  } else if (node->type == CMARK_NODE_TABLE_ROW) {
    return child_type == CMARK_NODE_TABLE_CELL;
  } else if (node->type == CMARK_NODE_TABLE_CELL) {
    return child_type == CMARK_NODE_TEXT || child_type == CMARK_NODE_CODE ||
           child_type == CMARK_NODE_EMPH || child_type == CMARK_NODE_STRONG ||
           child_type == CMARK_NODE_LINK || child_type == CMARK_NODE_IMAGE ||
           child_type == CMARK_NODE_STRIKETHROUGH ||
           child_type == CMARK_NODE_HTML_INLINE ||
           child_type == CMARK_NODE_FOOTNOTE_REFERENCE;
  }
  return false;
}

/// 扩展语法的节点类型是否能包含内联。
///
/// cmark_syntax_extension->contains_inlines_func
static int contains_inlines(cmark_syntax_extension *extension,
                            cmark_node *node) {
  return node->type == CMARK_NODE_TABLE_CELL;
}

static void commonmark_render(cmark_syntax_extension *extension,
                              cmark_renderer *renderer, cmark_node *node,
                              cmark_event_type ev_type, int options) {
  bool entering = (ev_type == CMARK_EVENT_ENTER);
  // table 与其他之间需要有空行(至少两个 cr)；row 之间需要有 cr （至少一个 cr）
  if (node->type == CMARK_NODE_TABLE) {
    renderer->blankline(renderer);
  } else if (node->type == CMARK_NODE_TABLE_ROW) {
    if (entering) {
      renderer->cr(renderer);
      renderer->out(renderer, node, "|", false, LITERAL);
    }
  } else if (node->type == CMARK_NODE_TABLE_CELL) {
    if (entering) {
      renderer->out(renderer, node, " ", false, LITERAL);
    } else {
      renderer->out(renderer, node, " |", false, LITERAL);
      if (((node_table_row *)node->parent->as.opaque)->is_header &&
          !node->next) {
        int i;
        uint8_t *alignments = get_table_alignments(node->parent->parent);
        uint16_t n_cols =
            ((node_table *)node->parent->parent->as.opaque)->n_columns;
        renderer->cr(renderer);
        renderer->out(renderer, node, "|", false, LITERAL);
        for (i = 0; i < n_cols; i++) {
          switch (alignments[i]) {
          case 0:   renderer->out(renderer, node, " --- |", false, LITERAL); break;
          case 'l': renderer->out(renderer, node, " :-- |", false, LITERAL); break;
          case 'c': renderer->out(renderer, node, " :-: |", false, LITERAL); break;
          case 'r': renderer->out(renderer, node, " --: |", false, LITERAL); break;
          }
        }
        renderer->cr(renderer);
      }
    }
  } else {
    assert(false);
  }
}

static void latex_render(cmark_syntax_extension *extension,
                         cmark_renderer *renderer, cmark_node *node,
                         cmark_event_type ev_type, int options) {
  bool entering = (ev_type == CMARK_EVENT_ENTER);

  if (node->type == CMARK_NODE_TABLE) {
    if (entering) {
      int i;
      uint16_t n_cols;
      uint8_t *alignments = get_table_alignments(node);

      renderer->cr(renderer);
      renderer->out(renderer, node, "\\begin{table}", false, LITERAL);
      renderer->cr(renderer);
      renderer->out(renderer, node, "\\begin{tabular}{", false, LITERAL);

      n_cols = ((node_table *)node->as.opaque)->n_columns;
      for (i = 0; i < n_cols; i++) {
        switch(alignments[i]) {
        case 0:
        case 'l':
          renderer->out(renderer, node, "l", false, LITERAL);
          break;
        case 'c':
          renderer->out(renderer, node, "c", false, LITERAL);
          break;
        case 'r':
          renderer->out(renderer, node, "r", false, LITERAL);
          break;
        }
      }
      renderer->out(renderer, node, "}", false, LITERAL);
      renderer->cr(renderer);
    } else {
      renderer->out(renderer, node, "\\end{tabular}", false, LITERAL);
      renderer->cr(renderer);
      renderer->out(renderer, node, "\\end{table}", false, LITERAL);
      renderer->cr(renderer);
    }
  } else if (node->type == CMARK_NODE_TABLE_ROW) {
    if (!entering) {
      renderer->cr(renderer);
    }
  } else if (node->type == CMARK_NODE_TABLE_CELL) {
    if (!entering) {
      if (node->next) {
        renderer->out(renderer, node, " & ", false, LITERAL);
      } else {
        renderer->out(renderer, node, " \\\\", false, LITERAL);
      }
    }
  } else {
    assert(false);
  }
}

static const char *xml_attr(cmark_syntax_extension *extension,
                            cmark_node *node) {
  if (node->type == CMARK_NODE_TABLE_CELL) {
    if (cmark_gfm_extensions_get_table_row_is_header(node->parent)) {
      uint8_t *alignments = get_table_alignments(node->parent->parent);
      int i = 0;
      cmark_node *n;
      for (n = node->parent->first_child; n; n = n->next, ++i)
        if (n == node)
          break;
      switch (alignments[i]) {
      case 'l': return " align=\"left\"";
      case 'c': return " align=\"center\"";
      case 'r': return " align=\"right\"";
      }
    }
  }

  return NULL;
}

static void man_render(cmark_syntax_extension *extension,
                       cmark_renderer *renderer, cmark_node *node,
                       cmark_event_type ev_type, int options) {
  bool entering = (ev_type == CMARK_EVENT_ENTER);

  if (node->type == CMARK_NODE_TABLE) {
    if (entering) {
      int i;
      uint16_t n_cols;
      uint8_t *alignments = get_table_alignments(node);

      renderer->cr(renderer);
      renderer->out(renderer, node, ".TS", false, LITERAL);
      renderer->cr(renderer);
      renderer->out(renderer, node, "tab(@);", false, LITERAL);
      renderer->cr(renderer);

      n_cols = ((node_table *)node->as.opaque)->n_columns;

      for (i = 0; i < n_cols; i++) {
        switch (alignments[i]) {
        case 'l':
          renderer->out(renderer, node, "l", false, LITERAL);
          break;
        case 0:
        case 'c':
          renderer->out(renderer, node, "c", false, LITERAL);
          break;
        case 'r':
          renderer->out(renderer, node, "r", false, LITERAL);
          break;
        }
      }

      if (n_cols) {
        renderer->out(renderer, node, ".", false, LITERAL);
        renderer->cr(renderer);
      }
    } else {
      renderer->out(renderer, node, ".TE", false, LITERAL);
      renderer->cr(renderer);
    }
  } else if (node->type == CMARK_NODE_TABLE_ROW) {
    if (!entering) {
      renderer->cr(renderer);
    }
  } else if (node->type == CMARK_NODE_TABLE_CELL) {
    if (!entering && node->next) {
      renderer->out(renderer, node, "@", false, LITERAL);
    }
  } else {
    assert(false);
  }
}
// MARK: -extension end-

// MARK: - -private start-
static void html_table_add_align(cmark_strbuf* html, const char* align, int options) {
  if (options & CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES) {
    cmark_strbuf_puts(html, " style=\"text-align: ");
    cmark_strbuf_puts(html, align);
    cmark_strbuf_puts(html, "\"");
  } else {
    cmark_strbuf_puts(html, " align=\"");
    cmark_strbuf_puts(html, align);
    cmark_strbuf_puts(html, "\"");
  }
}

struct html_table_state {
  unsigned need_closing_table_body : 1;
  unsigned in_table_header : 1;
};

static void html_render(cmark_syntax_extension *extension,
                        cmark_html_renderer *renderer, cmark_node *node,
                        cmark_event_type ev_type, int options) {
  bool entering = (ev_type == CMARK_EVENT_ENTER);
  cmark_strbuf *html = renderer->html;
  cmark_node *n;

  // XXX: we just monopolise renderer->opaque.
  struct html_table_state *table_state =
      (struct html_table_state *)&renderer->opaque;

  if (node->type == CMARK_NODE_TABLE) {
    if (entering) {
      cmark_html_render_cr(html);
      cmark_strbuf_puts(html, "<table");
      cmark_html_render_sourcepos(node, html, options);
      cmark_strbuf_putc(html, '>');
      table_state->need_closing_table_body = false;
    } else {
      if (table_state->need_closing_table_body) {
        cmark_html_render_cr(html);
        cmark_strbuf_puts(html, "</tbody>");
        cmark_html_render_cr(html);
      }
      table_state->need_closing_table_body = false;
      cmark_html_render_cr(html);
      cmark_strbuf_puts(html, "</table>");
      cmark_html_render_cr(html);
    }
  } else if (node->type == CMARK_NODE_TABLE_ROW) {
    if (entering) {
      cmark_html_render_cr(html);
      if (((node_table_row *)node->as.opaque)->is_header) {
        table_state->in_table_header = 1;
        cmark_strbuf_puts(html, "<thead>");
        cmark_html_render_cr(html);
      } else if (!table_state->need_closing_table_body) {
        cmark_strbuf_puts(html, "<tbody>");
        cmark_html_render_cr(html);
        table_state->need_closing_table_body = 1;
      }
      cmark_strbuf_puts(html, "<tr");
      cmark_html_render_sourcepos(node, html, options);
      cmark_strbuf_putc(html, '>');
    } else {
      cmark_html_render_cr(html);
      cmark_strbuf_puts(html, "</tr>");
      if (((node_table_row *)node->as.opaque)->is_header) {
        cmark_html_render_cr(html);
        cmark_strbuf_puts(html, "</thead>");
        table_state->in_table_header = false;
      }
    }
  } else if (node->type == CMARK_NODE_TABLE_CELL) {
    uint8_t *alignments = get_table_alignments(node->parent->parent);
    if (entering) {
      cmark_html_render_cr(html);
      if (table_state->in_table_header) {
        cmark_strbuf_puts(html, "<th");
      } else {
        cmark_strbuf_puts(html, "<td");
      }

      int i = 0;
      for (n = node->parent->first_child; n; n = n->next, ++i)
        if (n == node)
          break;

      switch (alignments[i]) {
      case 'l': html_table_add_align(html, "left", options); break;
      case 'c': html_table_add_align(html, "center", options); break;
      case 'r': html_table_add_align(html, "right", options); break;
      }

      cmark_html_render_sourcepos(node, html, options);
      cmark_strbuf_putc(html, '>');
    } else {
      if (table_state->in_table_header) {
        cmark_strbuf_puts(html, "</th>");
      } else {
        cmark_strbuf_puts(html, "</td>");
      }
    }
  } else {
    assert(false);
  }
}
// MARK: -private end-

// MARK: - -extension start-
static void opaque_alloc(cmark_syntax_extension *self, cmark_mem *mem, cmark_node *node) {
  if (node->type == CMARK_NODE_TABLE) {
    node->as.opaque = mem->calloc(1, sizeof(node_table));
  } else if (node->type == CMARK_NODE_TABLE_ROW) {
    node->as.opaque = mem->calloc(1, sizeof(node_table_row));
  } else if (node->type == CMARK_NODE_TABLE_CELL) {
    node->as.opaque = mem->calloc(1, sizeof(node_cell));
  }
}

static void opaque_free(cmark_syntax_extension *self, cmark_mem *mem, cmark_node *node) {
  if (node->type == CMARK_NODE_TABLE) {
    free_node_table(mem, node->as.opaque);
  } else if (node->type == CMARK_NODE_TABLE_ROW) {
    free_node_table_row(mem, node->as.opaque);
  }
}

/// 判断在 node 中的 c 是否为逃逸字符
static int escape(cmark_syntax_extension *self, cmark_node *node, int c) {
  return
    node->type != CMARK_NODE_TABLE &&
    node->type != CMARK_NODE_TABLE_ROW &&
    node->type != CMARK_NODE_TABLE_CELL &&
    c == '|';
}

cmark_syntax_extension *create_table_extension(void) {
  cmark_syntax_extension *self = cmark_syntax_extension_new("table");

  cmark_syntax_extension_set_match_block_func(self, matches);
  cmark_syntax_extension_set_open_block_func(self, try_opening_table_block);
  cmark_syntax_extension_set_get_type_string_func(self, get_type_string);
  cmark_syntax_extension_set_can_contain_func(self, can_contain);
  cmark_syntax_extension_set_contains_inlines_func(self, contains_inlines);
  cmark_syntax_extension_set_commonmark_render_func(self, commonmark_render);
  cmark_syntax_extension_set_plaintext_render_func(self, commonmark_render);
  cmark_syntax_extension_set_latex_render_func(self, latex_render);
  cmark_syntax_extension_set_xml_attr_func(self, xml_attr);
  cmark_syntax_extension_set_man_render_func(self, man_render);
  cmark_syntax_extension_set_html_render_func(self, html_render);
  cmark_syntax_extension_set_opaque_alloc_func(self, opaque_alloc);
  cmark_syntax_extension_set_opaque_free_func(self, opaque_free);
  cmark_syntax_extension_set_commonmark_escape_func(self, escape);
  CMARK_NODE_TABLE = cmark_syntax_extension_add_node(0);
  CMARK_NODE_TABLE_ROW = cmark_syntax_extension_add_node(0);
  CMARK_NODE_TABLE_CELL = cmark_syntax_extension_add_node(0);

  return self;
}
// MARK: -extension end-

// MARK: - -API-
uint16_t cmark_gfm_extensions_get_table_columns(cmark_node *node) {
  if (node->type != CMARK_NODE_TABLE)
    return 0;

  return ((node_table *)node->as.opaque)->n_columns;
}

uint8_t *cmark_gfm_extensions_get_table_alignments(cmark_node *node) {
  if (node->type != CMARK_NODE_TABLE)
    return 0;

  return ((node_table *)node->as.opaque)->alignments;
}

int cmark_gfm_extensions_set_table_columns(cmark_node *node, uint16_t n_columns) {
  return set_n_table_columns(node, n_columns);
}

int cmark_gfm_extensions_set_table_alignments(cmark_node *node, uint16_t ncols, uint8_t *alignments) {
  uint8_t *a = (uint8_t *)cmark_node_mem(node)->calloc(1, ncols);
  memcpy(a, alignments, ncols);
  return set_table_alignments(node, a);
}

int cmark_gfm_extensions_get_table_row_is_header(cmark_node *node)
{
  if (!node || node->type != CMARK_NODE_TABLE_ROW)
    return 0;

  return ((node_table_row *)node->as.opaque)->is_header;
}

int cmark_gfm_extensions_set_table_row_is_header(cmark_node *node, int is_header)
{
  if (!node || node->type != CMARK_NODE_TABLE_ROW)
    return 0;

  ((node_table_row *)node->as.opaque)->is_header = (is_header != 0);
  return 1;
}
