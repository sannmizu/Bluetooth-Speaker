/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-21 15:37:35
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:27:28
 */ 
#ifndef SLIDE_WND_H
#define SLIDE_WND_H

#include "slidewnd_types.h"

#define WINDOW_SIZE 32

extern slide_wnd* create_slide_wnd(int size);
extern int slide_wnd_push(slide_wnd* wnd, char* src, int len);
extern int slide_wnd_pop(slide_wnd* wnd, char* dest, int len);
extern int slide_wnd_seek(slide_wnd* wnd, int offset, char* dest);
extern void free_slide_wnd(slide_wnd* wnd);
#endif