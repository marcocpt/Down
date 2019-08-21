#ifndef CMARK_NODE_H
#define CMARK_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#include "cmark-gfm.h"
#include "cmark-gfm-extension_api.h"
#include "buffer.h"
#include "chunk.h"

typedef struct {
  cmark_list_type list_type;
  int marker_offset;
  int padding;
  int start;
  cmark_delim_type delimiter;
  unsigned char bullet_char;
  bool tight;
  bool checked; // For task list extension
} cmark_list;

typedef struct {
  cmark_chunk info;
  cmark_chunk literal;
  uint8_t fence_length;
  uint8_t fence_offset;
  unsigned char fence_char;
  int8_t fenced;
} cmark_code;

typedef struct {
  int level;
  bool setext;
} cmark_heading;

typedef struct {
  cmark_chunk url;
  cmark_chunk title;
} cmark_link;

typedef struct {
  cmark_chunk on_enter;
  cmark_chunk on_exit;
} cmark_custom;

enum cmark_node__internal_flags {
  CMARK_NODE__OPEN = (1 << 0),              /**< 是 open 状态 */
  CMARK_NODE__LAST_LINE_BLANK = (1 << 1),   /**< 最后的行已经消耗 */
  CMARK_NODE__LAST_LINE_CHECKED = (1 << 2), /**< 最后的行已经检查 */
};

struct cmark_node {
  cmark_strbuf content;             /**< 存储内容的字符串 */

  struct cmark_node *next;              /**< 下一个节点（平行） */
  struct cmark_node *prev;              /**< 上一个节点（平行） */
  struct cmark_node *parent;            /**< 父节点 */
  struct cmark_node *first_child;       /**< 第一个子节点 */
  struct cmark_node *last_child;        /**< 最后一个子节点 */

  void *user_data;
  cmark_free_func user_data_free_func;

  int start_local;                      /**< 在文本中的起始位置 */
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

/// 获取节点的 content.mem 属性
static CMARK_INLINE cmark_mem *cmark_node_mem(cmark_node *node) {
  return node->content.mem;
}
CMARK_GFM_EXPORT int cmark_node_check(cmark_node *node, FILE *out);

/// 节点类型为块
static CMARK_INLINE bool CMARK_NODE_TYPE_BLOCK_P(cmark_node_type node_type) {
	return (node_type & CMARK_NODE_TYPE_MASK) == CMARK_NODE_TYPE_BLOCK;
}

static CMARK_INLINE bool CMARK_NODE_BLOCK_P(cmark_node *node) {
	return node != NULL && CMARK_NODE_TYPE_BLOCK_P((cmark_node_type) node->type);
}

/// 节点类型为内联
static CMARK_INLINE bool CMARK_NODE_TYPE_INLINE_P(cmark_node_type node_type) {
	return (node_type & CMARK_NODE_TYPE_MASK) == CMARK_NODE_TYPE_INLINE;
}

static CMARK_INLINE bool CMARK_NODE_INLINE_P(cmark_node *node) {
	return node != NULL && CMARK_NODE_TYPE_INLINE_P((cmark_node_type) node->type);
}

/// node 能够包含 child_type 类型
///
/// 如果 node 是扩展的，就调用扩展的 can_contain_func 函数
CMARK_GFM_EXPORT bool cmark_node_can_contain_type(cmark_node *node, cmark_node_type child_type);

#ifdef __cplusplus
}
#endif

#endif
