/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-20 20:57:48
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:33:08
 */ 
#ifndef SANN_MUSIC_H
#define SANN_MUSIC_H
#include "music_types.h"

extern music_protocol_head music_fill_head(char src[MUSIC_HEAD_LEN]);
extern int music_signal_check(music_signal *ms, int len);

#endif