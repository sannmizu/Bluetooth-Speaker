/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-22 20:43:20
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 23:44:56
 */ 
#include "pipe.h"
#include "nodes/list.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct pipe_t {
    int pid;
    music_protocol **pArray;
    int pArray_size;
    int loc;
    int loc_inside;
    pthread_rwlock_t rwlock;
    volatile int closed;
} pipe_t;

List *pipes = NIL;
pthread_rwlock_t pipes_rwlock;
List *control_list = NIL;
pthread_rwlock_t control_rwlock;

static void close_pipes();

static ListCell* list_find(int pid) {
    if (pipes == NIL) {
        return NULL;
    }

    ListCell* cell = list_head(pipes);
    while (cell != NULL) {
        if (lfirst(cell) != NULL && ((pipe_t *) lfirst(cell))->pid == pid) {
            break;
        }
        cell = lnext(cell);
    }
    return cell;
}

static pipe_t* create_pipe_internal(int pid, int size) {
    pipe_t *pipe = (pipe_t *) malloc(sizeof(pipe_t));
    pipe->pid = pid;
    pipe->pArray = (music_protocol **) malloc(sizeof(void *) * size);
    pipe->pArray_size = size;
    pipe->loc = 0;
    pipe->loc_inside = 0;
    pipe->closed = 0;
    memset(pipe->pArray, 0, pipe->pArray_size * sizeof(music_protocol *));
    pthread_rwlock_init(&pipe->rwlock, NULL);

    pthread_rwlock_wrlock(&pipes_rwlock);
    if (list_find(pid) != NULL) {
        pthread_rwlock_destroy(&pipe->rwlock);
        free(pipe->pArray);
        free(pipe);
        return NULL;
    }
    lappend(pipes, (void *) pipe);
    pthread_rwlock_unlock(&pipes_rwlock);
    
    return pipe;
}

static int pArray_set(pipe_t *pipe, music_protocol *p, int loc) {
    if (loc < 0) {
        return -1;
    }
    int old_size = pipe->pArray_size;
    if (pipe->pArray_size < loc) {
        do {
            pipe->pArray_size *= 2;
        } while (pipe->pArray_size < loc);
        pipe->pArray = (music_protocol **) realloc(pipe->pArray, sizeof(music_protocol*) * pipe->pArray_size);
    }
    memset(pipe->pArray + old_size, 0, (pipe->pArray_size - old_size) * sizeof(music_protocol*));
    pipe->pArray[loc] = p;
    return loc;
}

static music_protocol* pArray_get(pipe_t *pipe, int loc) {
    if (loc < 0) {
        return NULL;
    }
    if (pipe->pArray_size < loc) {
        return NULL;
    }
    return pipe->pArray[loc];
}

int init_pipe() {
    int ret;
    pthread_rwlock_init(&pipes_rwlock, NULL);
    pthread_rwlock_init(&control_rwlock, NULL);
}

int destroy_pipe() {
    if (pipes != NIL) {
        close_pipes();
    }
    if (control_list != NIL) {
        close_control_pipe();
    }
    pthread_rwlock_destroy(&pipes_rwlock);
    pthread_rwlock_destroy(&control_rwlock);
}

int open_pipe(int pid) {
    pipe_t* pipe = create_pipe_internal(pid, 32);
    return pipe == NULL ? -1 : pipe->pid;
}

int write_pipe(int pid, music_protocol* mp, int mp_size) {
    pthread_rwlock_rdlock(&pipes_rwlock);
    
    ListCell *cell = list_find(pid);
    if (cell == NULL) {
        pthread_rwlock_unlock(&pipes_rwlock);
        return -1;
    }
    pipe_t *pipe = (pipe_t *) lfirst(cell);

    pthread_rwlock_rdlock(&pipe->rwlock);
    pthread_rwlock_unlock(&pipes_rwlock);
    
    music_protocol* _mp = (music_protocol *) malloc(mp_size);
    memcpy(_mp, mp, mp_size);
    int result = pArray_set(pipe, _mp, _mp->head.offset);

    pthread_rwlock_unlock(&pipe->rwlock);
    return result;
}

int write_control_pipe(music_protocol* mp, int mp_size) {
    /* ListCell *cell = list_find(CONTROL_PID);
    pipe_t* pipe = NULL;
    if (cell == NULL) {
        pipe = create_pipe_internal(CONTROL_PID, 1);
    } else {
        pipe = (pipe_t *) lfirst(cell);
    }
    music_protocol *_mp = (music_protocol *) malloc(mp_size);
    memcpy(_mp, mp, mp_size);
    if (pipe->pArray[0] == NULL) {
        pipe->pArray[0] = (void *) list_make1((void *) _mp);
        return 0;
    }
    List *l = (List *) pipe->pArray[0];
    ListCell *prev = NULL;
    ListCell *cell = list_head(l);
    int loc = 0;
    while (cell != NULL) {
        if (((music_protocol *) cell->data)->head.id > _mp->head.id) {
            break;
        }
        prev = cell;
        cell = lnext(cell);
        loc++;
    }
    lappend_cell(l, prev, _mp);
    return loc; */

    music_protocol *_mp = (music_protocol *) malloc(mp_size);
    memcpy(_mp, mp, mp_size);
    pthread_rwlock_wrlock(&control_rwlock);
    if (control_list == NIL) {
        control_list = list_make1((void *) _mp);
    }
    ListCell *prev = NULL;
    ListCell *cell = list_head(control_list);
    int loc = 0;
    while (cell != NULL) {
        if (((music_protocol *) cell->data)->head.id > _mp->head.id) {
            break;
        }
        prev = cell;
        cell = lnext(cell);
        loc++;
    }
    lappend_cell(control_list, prev, _mp);
    pthread_rwlock_unlock(&control_rwlock);
    return loc;
}

int read_pipe(int pid, char* buf, int nbytes) {
    pthread_rwlock_rdlock(&pipes_rwlock);

    ListCell *cell = list_find(pid);
    if (cell == NULL) {
        pthread_rwlock_unlock(&pipes_rwlock);
        return -1;
    }
    pipe_t *pipe = (pipe_t *) lfirst(cell);

    pthread_rwlock_rdlock(&pipe->rwlock);
    pthread_rwlock_unlock(&pipes_rwlock);

    music_protocol *mp;

    int piece_size = 0;
    int buf_loc = 0;
    int end = 0;
    while (!pipe->closed && !end) {
        mp = pArray_get(pipe, pipe->loc);
        if (mp == NULL && buf_loc != 0) {
            break;
        } else if (mp == NULL) {
            continue;
        }
        
        piece_size = mp->head.len - pipe->loc_inside;
        int read_size = nbytes - buf_loc;
        if (piece_size > read_size) {
            memcpy(buf + buf_loc, mp->data + pipe->loc_inside, read_size);
            pipe->loc_inside += read_size;
            break;
        } else {
            if (mp->head.flag == FLAG_END) {
                end = 1;
            }
            memcpy(buf + buf_loc, mp->data + pipe->loc_inside, piece_size);
            free(pipe->pArray[pipe->loc]);
            pipe->pArray[pipe->loc] = NULL;
            pipe->loc++;
            pipe->loc_inside = 0;
            buf_loc += piece_size;
        }
    }
    
    pthread_rwlock_unlock(&pipe->rwlock);
    if (end) {
        close_pipe(pid);
    }
    return buf_loc;
}

int read_control_pipe(music_signal* ms, int *ms_size) {
    /* if (ms = NULL) {
        return -1;
    }
    ListCell *cell = list_find(CONTROL_PID);
    if (cell == NULL) {
        return -1;
    }
    pipe_t *pipe = (pipe_t *) lfirst(cell);
    while (1) {
        List *l = (List *)pipe->pArray[0];
        if (l == NIL || l->length == 0) {
            continue;
        }
        int size = ((music_protocol *) linitial(l))->head.len;
        if (size > *ms_size) {
            return -1;
        }
        memcpy(ms, ((music_protocol *) linitial(l))->data, size);
        *ms_size = size;
        list_delete_cell(l, list_head(l), NULL);
    } 
    return 0; */

    while (1) {
        if (control_list == NIL || control_list->length == 0) {
            continue;
        }
        pthread_rwlock_wrlock(&control_rwlock);
        int size = ((music_protocol *) linitial(control_list))->head.len;
        if (size > *ms_size) {
            return -1;
        }
        memcpy(ms, ((music_protocol *) linitial(control_list))->data, size);
        *ms_size = size;
        list_delete_cell(control_list, list_head(control_list), NULL);
        pthread_rwlock_unlock(&control_rwlock);
    }
    return 0;
}

int close_pipe(int pid) {
    if (pipes == NIL) {
        return -1;
    }
    pthread_rwlock_wrlock(&pipes_rwlock);
    
    ListCell* prev = NULL;
    ListCell* cell = list_head(pipes);
    while (cell != NULL) {
        if (lfirst(cell) != NULL && ((pipe_t *) lfirst(cell))->pid == pid) {
            break;
        }
        prev = cell;
        cell = lnext(cell);
    }
    if (cell == NULL) {
        return -1;
    }

    pipe_t *pipe = (pipe_t *) lfirst(cell);
    pipe->closed = 1;
    
    pthread_rwlock_t lock = pipe->rwlock;
    pthread_rwlock_wrlock(&lock);

    int i = 0;
    for(i; i < pipe->pArray_size; i++) {
        if (pipe->pArray[i] != NULL) {
            free(pipe->pArray[i]);
            pipe->pArray[i] = NULL;
        }
    }
    free(pipe);
    lfirst(cell) = NULL;

    pthread_rwlock_unlock(&lock);
    pthread_rwlock_destroy(&lock);

    list_delete_cell(pipes, cell, prev);

    pthread_rwlock_unlock(&pipes_rwlock);
    return 0;
}

static void close_pipes() {
    if (pipes == NIL) {
        return;
    }
    pthread_rwlock_wrlock(&pipes_rwlock);
    
    ListCell* cell = list_head(pipes);
    while (cell != NULL) {
        pipe_t *pipe = (pipe_t *) lfirst(cell);
        pipe->closed = 1;
        
        pthread_rwlock_t lock = pipe->rwlock;
        pthread_rwlock_wrlock(&lock);

        int i = 0;
        for(i; i < pipe->pArray_size; i++) {
            if (pipe->pArray[i] != NULL) {
                free(pipe->pArray[i]);
                pipe->pArray[i] = NULL;
            }
        }
        free(pipe);
        lfirst(cell) = NULL;

        pthread_rwlock_unlock(&lock);
        pthread_rwlock_destroy(&lock);
        
        cell = lnext(cell);
    }

    list_free(pipes);
    pipes = NIL;

    pthread_rwlock_unlock(&pipes_rwlock);
}

int close_control_pipe() {
    /* if (pipes == NIL) {
        return -1;
    }
    ListCell* prev = NULL;
    ListCell* cell = list_head(pipes);
    while (cell != NULL) {
        if (lfirst(cell) != NULL && ((pipe_t *) lfirst(cell))->pid == CONTROL_PID) {
            break;
        }
        prev = cell;
        cell = lnext(cell);
    }
    if (cell == NULL) {
        return -1;
    }

    pipe_t *pipe = (pipe_t *) lfirst(cell);

    list_free_deep((List *) (pipe->pArray[0]));
    pipe->pArray[0] = NULL;
    free(pipe);
    return 0; */
    pthread_rwlock_wrlock(&control_rwlock);
    list_free_deep(control_list);
    control_list = NIL;
    pthread_rwlock_unlock(&control_rwlock);
    return 0;
}

int find_pipe(int pid) {
    if (list_find(pid) != NULL) {
        return -1;
    }
    return pid;
}