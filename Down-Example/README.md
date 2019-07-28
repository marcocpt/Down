[TOC]

# struct cmark_node

定义：

```c
struct cmark_node {
  cmark_strbuf content;             /**< 存储内容的字符串 */

  struct cmark_node *next;              /**< 下一个节点（平行） */
  struct cmark_node *prev;              /**< 上一个节点（平行） */
  struct cmark_node *parent;            /**< 父节点 */
  struct cmark_node *first_child;       /**< 第一个子节点 */
  struct cmark_node *last_child;        /**< 最后一个子节点 */

  void *user_data;
  cmark_free_func user_data_free_func;

  int start_line;                       /**< 起始行 */
  int start_column;                     /**< 起始列（在行中的偏移） */
  int end_line;                         /**< 结束行 */
  int end_column;                       /**< 结束列（在行中的偏移 */
  int internal_offset;
  uint16_t type;                        /**< 节点的类型 cmark_node_type */
  uint16_t flags;                       /**< cmark_node__internal_flags */

  cmark_syntax_extension *extension;

  union {
    cmark_chunk literal;
    cmark_list list;
    cmark_code code;
    cmark_heading heading;
    cmark_link link;
    cmark_custom custom;
    int html_block_type;
    void *opaque;                      /**< 自定义数据。在 table.c 中用来存储 node_table, node_table_row 或 node_cell */
  } as;
};
```



#  table 扩展

基本结构如下：

- ↳ Table
	- ↳ TableRow
	  - ↳ TableCell
	    - ↳ Text - 表头 1
	  - ↳ TableCell
	    - ↳ Text - foo 
	    -  ↳ Code - let a = 1
	-  ↳ TableRow
	  - ↳ TableCell
	    - ↳ Text - 表行 1



## 结构体

### table_row

```c
/// 包含列数、软中断偏移和单元格链表(data 类型为 node_cell)
///
/// 主要用于生成 CMARK_NODE_TABLE_CELL ，不存储
/// 获取单元格字符串：`p ((node_cell *)<#table_row *row#>->cells->data)->buf->ptr`
typedef struct {
  uint16_t n_columns;       /**< 一行中单元格的列个数 */
  int paragraph_offset;     /**< 段落偏移。当一行中有软中断时，保存软中断的偏移位置 */
  cmark_llist *cells;       /**< 一行中的 node_cell 链表 */
} table_row;
```

- 获取函数：**row_from_string**

  ```c
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
                                    int len)
  ```

  

### node_table

保存在 `cmark_node->as.opaque` 中

```c
typedef struct {
  uint16_t n_columns;       /**< 表格节点中的列个数 */
  uint8_t *alignments;      /**< 表格节点中所有列的对齐方式, 例："rclr"  */
} node_table;
```



### node_table_row

保存在 `cmark_node->as.opaque` 中

```c
/// 存储 is_header
typedef struct {
  bool is_header;           /**< 是表头 */
} node_table_row;
```

### node_cell

保存在 `cmark_node->as.opaque` 中

```c
typedef struct {
  cmark_strbuf *buf;        /**< 存字符串 */
  int start_offset;         /**< 在行中单元格内容的起始偏移位置，包括不可见字符 */
  int end_offset;           /**< 在行中单元格内容的结束偏移位置 */
  int internal_offset;      /**< 记录第一个可见字符到 '|' 间的偏移 */
} node_cell;
```

## 函数

### create_table_extension

```c
/// 为结构体的成员赋值（添加函数），并创建新的 cmark_node_type
cmark_syntax_extension *create_table_extension(void) {
  cmark_syntax_extension *self = cmark_syntax_extension_new("table");
	
  /** cmark will call the function provided through
  * 'cmark_syntax_extension_set_match_block_func' when it
  * iterates over an open block created by this extension,
  * to determine  whether it could contain the new line.
  * If no function was provided, cmark will close the block.
 	*/
  cmark_syntax_extension_set_match_block_func(self, matches);
  
  /** if and only if the new line doesn't match any
  * of the standard syntax rules, cmark will call the function
  * provided through 'cmark_syntax_extension_set_open_block_func'
  * to let the extension determine whether that new line matches
  * one of its syntax rules.
  */
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
```



## Table

- 节点类型：	CMARK_NODE_TABLE （0x800c 32780)`
- 节点字符串："table"

## TableRow

- 节点类型：	CMARK_NODE_TABLE_ROW （0x800d 32781)
- 节点字符串："table_header"，"table_row"

## TableCell

- 节点类型：	CMARK_NODE_TABLE_CELL （0x800e 32782)`
- 节点字符串：”table_cell”
- 字符内容：    `p cmark_node_get_string_content(<#cmark_node *node#>)`

## TableCell 的子节点

- 节点类型：	内联类型
- 节点字符串：
- 字符内容：根据**父节点**和节点属性的**行、列**位置获取



## API

获取**节点类型**：`p cmark_node_get_type(<#cmark_node *node#>)`

获取**节点字符串**：`p cmark_node_get_type_string(<#cmark_node *node#>)`

获取**是表头**：`p cmark_gfm_extensions_get_table_row_is_header(<#cmark_node *node#>)`

