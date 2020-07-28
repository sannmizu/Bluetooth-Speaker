/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-21 15:49:07
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:47:39
 */ 
#include "nodes/slidewnd.h"
#include <stdlib.h>
#include <string.h>

slide_wnd* create_slide_wnd(int size) {
    if (size <= 0) {
        size = WINDOW_SIZE;
    }
    slide_wnd* wnd = (slide_wnd *) malloc(sizeof(slide_wnd));
    if (wnd == NULL) {
        return NULL;
    }
    wnd->buffer = (char *) malloc(sizeof(char) * size);
    wnd->alloc_size = size;
    wnd->buffer_len = 0;
    wnd->head = 0;
    wnd->tail = 0;
    return wnd;
}

int slide_wnd_push(slide_wnd* wnd, char* src, int len) {
    if (wnd == NULL || len > wnd->alloc_size - wnd->buffer_len) {
        return 0;
    }
    int free_len = wnd->alloc_size - wnd->buffer_len;
    //    right    head   used   tail   left
    //              |              |
    // -------------===============-----------
    //    used     tail   left   head   used
    //              |              |
    // =============---------------===========
    int left_harf = wnd->tail - wnd->head >= 0 ? wnd->alloc_size - wnd->tail : free_len;
    // int right_harf = free_len - left_harf;
    if (left_harf >= len) {
        memcpy(wnd->buffer + wnd->tail, src, len);
        wnd->tail = (wnd->tail + len) % wnd->alloc_size;
    } else {
        memcpy(wnd->buffer + wnd->tail, src, left_harf);
        memcpy(wnd->buffer, src + left_harf, len - left_harf);
        wnd->tail = len - left_harf;
    }
    return len;
}

int slide_wnd_pop(slide_wnd* wnd, char* dest, int len) {
    if (wnd == NULL || len <= 0 || wnd->buffer_len == 0) {
        return 0;
    }
    int used_len = wnd->buffer_len;
    int pop_len = pop_len <= used_len ? pop_len : used_len;
    //    free    head    left   tail   free
    //              |              |
    // -------------===============-----------
    //    right    tail   free   head   left
    //              |              |
    // =============---------------===========
    int left_harf = wnd->tail - wnd->head >= 0 ? used_len : wnd->alloc_size - wnd->head;
    // int right_harf = used_len - left_harf;
    if (left_harf >= pop_len) {
        if (dest != NULL) {
            memcpy(dest, wnd->buffer + wnd->head, pop_len);
        }
        wnd->head = (wnd->head + pop_len) % wnd->alloc_size;
    } else {
        if (dest != NULL) {
            memcpy(dest, wnd->buffer + wnd->head, left_harf);
            memcpy(dest + left_harf, wnd->buffer + wnd->head, pop_len - left_harf);
        }
        wnd->head = pop_len - left_harf;
    }
    wnd->buffer_len -= pop_len;
    return pop_len;
}

int slide_wnd_seek(slide_wnd* wnd, int offset, char* dest) {
    if (wnd == NULL || wnd->buffer_len < offset || dest == NULL) {
        return -1;
    }
    *dest = wnd->buffer[offset];
    return 0;
}

void free_slide_wnd(slide_wnd* wnd) {
    if (wnd == NULL) {
        return;
    }
    if (wnd->buffer != NULL) {
        free(wnd->buffer);
        wnd->buffer = NULL;
    }
    free(wnd);
    wnd = NULL;
}