/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-21 22:33:49
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 23:56:21
 */ 
#include "nodes/memory.h"
#include <stdlib.h>
#include <string.h>

bt_buffer* newBuffer() {
    bt_buffer* buffer = (bt_buffer *) malloc(sizeof(bt_buffer));
    buffer->alloc_size = BUFFER_BLOCK_SIZE;
    buffer->buffer = 0;
    buffer->buffer = malloc(buffer->alloc_size);
    return buffer;
}

void appendBuffer(void* src, int len, bt_buffer* buffer) {
    if ((buffer->buffer_len + len) >= buffer->alloc_size) {
        do {
            buffer->alloc_size *= 2;
        } while ((buffer->buffer_len + len) >= buffer->alloc_size);
        buffer->buffer = realloc(buffer->buffer, buffer->alloc_size);
    }
    memcpy(buffer->buffer + buffer->buffer_len, src, len);
    buffer->buffer_len += len;
}

void freeBuffer(bt_buffer* buffer) {
    if (buffer != NULL) {
        if (buffer->buffer != NULL) {
            free(buffer->buffer);
            buffer->buffer = NULL;
        }
        free(buffer);
    }
}