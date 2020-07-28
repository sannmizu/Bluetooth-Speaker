/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-24 22:14:42
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:16:58
 */ 
#ifndef SANN_MEMORY_TYPES_H
#define SANN_MEMORY_TYPES_H

#define BUFFER_BLOCK_SIZE 1024
typedef struct bt_buffer {
    void* buffer;
    int alloc_size;
    int buffer_len;
} bt_buffer;

#endif