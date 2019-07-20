#ifndef CMARK_AST_H
#define CMARK_AST_H

#include <stdio.h>
#include "references.h"
#include "node.h"
#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LINK_LABEL_LENGTH 1000

struct cmark_parser {
  struct cmark_mem *mem;
  struct cmark_reference_map *refmap;
  struct cmark_node *root;
  struct cmark_node *current;
  int line_number;                      /**< 总行数 */
  bufsize_t offset;                     /**< byte position in input */
  bufsize_t column;                     /**< a virtual column numberthat takes into account tabs. (Multibyte characters are not takeninto account, because the Markdown line prefixes we are interested inanalyzing are entirely ASCII.) */
  bufsize_t first_nonspace;             /**< 第一个非j空格字符位 */
  bufsize_t first_nonspace_column;
  bufsize_t thematic_break_kill_pos;
  int indent;
  bool blank;                           /**< 是否为空行 */
  bool partially_consumed_tab;
  cmark_strbuf curline;                 /**< 当前在解析的行 */
  bufsize_t last_line_length;
  cmark_strbuf linebuf;
  int options;
  bool last_buffer_ended_with_cr;       /**< buffer 以 'cr' 结尾 */
};

#ifdef __cplusplus
}
#endif

#endif
