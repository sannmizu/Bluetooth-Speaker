/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-21 22:33:58
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:44:19
 */ 
#ifndef SANN_MEMORY_H
#define SANN_MEMORY_H

#include "memory_types.h"

extern bt_buffer* newBuffer();
extern void appendBuffer(void* src, int len, bt_buffer* buffer);
extern void freeBuffer(bt_buffer* buffer);

#endif