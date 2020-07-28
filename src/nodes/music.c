/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-21 21:57:49
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-23 15:02:22
 */ 
#include "nodes/music.h"
#include <string.h>

music_protocol_head music_fill_head(char src[MUSIC_HEAD_LEN]) {
    music_protocol_head result;
    memset(&result, 0, sizeof(result));
    if (src[0] != MUSIC_DATA && src[0] != MUSIC_CONTROL) {
        return result;
    }
    memcpy(&result, src, MUSIC_HEAD_LEN);
    return result;
}

int music_signal_check(music_signal *ms, int len) {
    unsigned int cknum = 0;
    uint8_t *raw = (uint8_t *) ms;
    while (len > 1) {
        cknum += *raw++;
        len -= sizeof(uint8_t);
    }
    while (cknum >> 8) {
        cknum = (cknum & 0xff) + (cknum >> 8);
    }
    return (int) (~cknum);
}