#ifndef CMARK_GFM_TABLE_H
#define CMARK_GFM_TABLE_H

#include "cmark-gfm-core-extensions.h"


extern cmark_node_type CMARK_NODE_TABLE, CMARK_NODE_TABLE_ROW,
    CMARK_NODE_TABLE_CELL;

/// 为结构体的成员赋值（添加函数），并创建新的 cmark_node_type
cmark_syntax_extension *create_table_extension(void);

#endif
