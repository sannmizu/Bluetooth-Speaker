/*
 * @Descripttion : 数据流处理库
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-24 22:11:51
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 23:51:32
 */ 
#include "nodes/list_types.h"
#include "nodes/memory_types.h"
#include "nodes/music_types.h"
#include "nodes/slidewnd_types.h"

extern music_protocol* read_a_piece(slide_wnd *wnd);
extern bt_buffer* music_union(List* mpl);