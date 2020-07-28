/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-25 13:32:46
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-25 13:47:13
 */ 
#ifndef SANN_LOG_H
#define SANN_LOG_H

#define ERR(x, y) { printf("error: "); printf(x); printf(":"); fflush(stdout); perror(y); }

#ifdef DEBUG
#define LOG(x, y) { printf("log: "); printf(x); printf(":"); printf(y); printf("\n"); }
#else
#define LOG(x, y)
#endif

#endif