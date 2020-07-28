/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-24 22:14:16
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 23:56:35
 */ 
#ifndef SANN_LIST_TYPES_H
#define SANN_LIST_TYPES_H

typedef struct ListCell ListCell;

typedef struct List {
    int         length;
    ListCell   *head;
    ListCell   *tail;
} List;

struct ListCell {
    void       *data;
    ListCell   *next;
};

#define NIL						((List *) NULL)

#endif