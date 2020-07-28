/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-22 20:43:26
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:59:57
 */ 
#ifndef SANN_PIPE_H
#define SANN_PIPE_H
#include "nodes/music.h"

#define CONTROL_PID 65536

extern int init_pipe();
extern int destroy_pipe();
extern int open_pipe(int pid);
extern int write_pipe(int pid, music_protocol* mp, int mp_size);
extern int write_control_pipe(music_protocol* mp, int mp_size);
extern int read_pipe(int pid, char* buf, int nbytes);
extern int read_control_pipe(music_signal* ms, int *ms_size);
extern int close_pipe(int pid);
extern int close_control_pipe();
extern int find_pipe(int pid);

#endif