/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-24 22:14:58
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:17:48
 */ 
#ifndef SLIDE_WND_TYPES_H
#define SLIDE_WND_TYPES_H

typedef struct slide_wnd {
    char* buffer;
    int alloc_size;
    int buffer_len;
    int head;
    int tail;
} slide_wnd;

#endif