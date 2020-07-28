/*
 * @Descripttion : bluetoothMainThread控制蓝牙连接，获取数据的总函数
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-15 13:55:41
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-25 14:55:35
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "log.h"
#include "bluetooth.h"
#include "nodes/music.h"
#include "nodes/slidewnd.h"
#include "stream.h"
#include "pipe.h"

#define LOG_NAME "bluetooth"

// 所有线程退出标志
static volatile int thread_end = 0;

void* bluetoothMainThread() {
    int blue_socket, client_socket;
    struct sockaddr_rc loc_addr = {0}, rem_addr = {0};
    char rem_addr_str[19];
    int ret;
    int rem_addr_len;

    slide_wnd *wnd = create_slide_wnd(1048576); // 1M
    char buffer[1024];
    int size;
    music_protocol* piece = NULL;
    
    memset(rem_addr_str, 0 ,19);

    LOG(LOG_NAME, "creating socket...");
    blue_socket = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (blue_socket < 0) {
        ERR(LOG_NAME, "create socket error");
        return (void *) -1;
    } else {
        LOG(LOG_NAME, "success");
    }
    // 本地地址
    loc_addr.rc_family=AF_BLUETOOTH;
    loc_addr.rc_bdaddr=*BDADDR_ANY;
    loc_addr.rc_channel=(uint8_t)1;
    
    // 绑定本地地址
    LOG(LOG_NAME, "binding socket...");
    ret = bind(blue_socket, (struct sockaddr *) &loc_addr, sizeof(loc_addr));
    if (ret < 0) {
        ERR(LOG_NAME, "bind socket error");
        close(blue_socket);
        return (void *) -1;
    } else {
        LOG(LOG_NAME, "success");
    }


    // 检查是否有历史配对
    // if (hasHistory) {
    // 有历史记录，主动搜索连接
    // } else {
    // 没有历史记录，等待外部主动连接，listen监听
        LOG(LOG_NAME, "start listen...");
        ret = listen(blue_socket, 1);
        if (ret < 0) {
            ERR(LOG_NAME, "listen error");
            close(blue_socket);
            return (void *) -1;
        } else {
            LOG(LOG_NAME, "listening...");
        }
        while (!thread_end) {
            rem_addr_len = sizeof(rem_addr);
            client_socket = accept(blue_socket, (struct sockaddr *) &rem_addr, &rem_addr_len);
            if (client_socket < 0) {
                ERR(LOG_NAME, "accpet error");
                continue;
            } else {
                LOG(LOG_NAME, "connect success");
            }
            ba2str(&rem_addr.rc_bdaddr, rem_addr_str);
            LOG(LOG_NAME, rem_addr_str);


            LOG(LOG_NAME, "reading message...");
            while(!thread_end) {
                size = read(client_socket, buffer, 1024);
                if (size <= 0) {
                    LOG(LOG_NAME, "out connect");
                    break;
                }
                size = slide_wnd_push(wnd, buffer, size);
                // 读出一个协议包
                while(piece = read_a_piece(wnd), piece != NULL) {
                    if (MUSIC_CONTROL == piece->head.type) {
                        // 如果是控制信号
                        ret = write_control_pipe(piece, piece->head.len + MUSIC_HEAD_LEN);
                    } else if (MUSIC_DATA == piece->head.type) {
                        // 如果是音乐数据
                        ret = write_pipe(piece->head.music_id, piece, piece->head.len + MUSIC_HEAD_LEN);
                    }
                    free(piece);
                    piece = NULL;
                }
            }
            close(client_socket);
        }
    // } 

    close(blue_socket);
    LOG(LOG_NAME, "bluetooth thread end");
    return (void *) 0;
}

void endBluetoothMainThread() {
    thread_end = 1;
}