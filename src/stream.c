/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-24 22:32:07
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 23:53:53
 */ 
#include <stdlib.h>

#include "stream.h"
#include "nodes/music.h"
#include "nodes/memory.h"
#include "nodes/slidewnd.h"
#include "nodes/list.h"

music_protocol* read_a_piece(slide_wnd *wnd) {
    music_protocol* result = NULL;
    int result_len;
    music_protocol_head head;
    int piece_len;

    if (wnd->buffer_len <= MUSIC_HEAD_LEN) {
        return NULL;
    }
    head = music_fill_head(wnd->buffer);
    result_len = head.len + MUSIC_HEAD_LEN;
    if (result_len > wnd->buffer_len) {
        // 数据不完整，等待
        return NULL;
    }

    switch (head.type) {
    case MUSIC_CONTROL:  // 控制
        result = (music_protocol *) malloc(result_len);
        slide_wnd_pop(wnd, (char *) result, result_len);
        if (0 != music_signal_check((music_signal *) result->data, head.len)) {
            free(result);
            result = NULL;
        }
        break;
    case MUSIC_DATA:  // 数据
        result = malloc(result_len);
        slide_wnd_pop(wnd, (char *) result, result_len);
        break;
    default:
        // error
        break;
    }
    return result;
}

// 要求list已经排序，且都是一组的
bt_buffer* music_union(List* mpl) {
    bt_buffer* result = newBuffer();
    ListCell* mplc = list_head(mpl);
    while (mplc != NULL) {
        music_protocol* per = (music_protocol *)(mplc->data);
        appendBuffer(per->data, per->head.len, result);
        mplc = lnext(mplc);
    }
    return result;
}