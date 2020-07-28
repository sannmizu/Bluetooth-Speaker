/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-24 19:46:40
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-25 14:32:57
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <nfc/nfc.h>
#include "log.h"
#include "nfc.h"
#include "nodes/slidewnd.h"
#include "nodes/music.h"
#include "pipe.h"
#include "stream.h"

#define LOG_NAME "nfc"
#define MAX_DEVICE_COUNT 16

// 所有线程退出标志
static volatile int thread_end = 0;

void* nfcMainThread() {
    nfc_context *context;
    nfc_target target;
    nfc_device *pnd = NULL;

    char buffer[1024];
    int buffer_size = 1024;
    int ret = 0;
    slide_wnd *wnd = create_slide_wnd(1048576);
    music_protocol* piece = NULL;

    LOG(LOG_NAME, "initing nfc context...");
    nfc_init(&context);
    if (context == NULL) {
        ERR(LOG_NAME, "Unable to init libnfc (malloc)");
        return (void *) -1;
    }

    // Open using the first available NFC device 
    LOG(LOG_NAME, "open nfc device...");
    pnd = nfc_open(context, NULL);
    if (pnd == NULL) {
        ERR(LOG_NAME, "Unable to open NFC device.");
        nfc_exit(context);
        return (void *) -1;
    }
    LOG(LOG_NAME, "NFC device opened.");
    LOG(LOG_NAME, nfc_device_get_name(pnd));
    

    LOG(LOG_NAME, "init nfc device...");
    if (nfc_initiator_init(pnd) < 0) {
        ERR(LOG_NAME, "Unable to init as reader.");
        nfc_exit(context);
        return (void *) -1;
    }

    // 设置目标
    nfc_dep_mode remote_dep_mode = NDM_UNDEFINED;
    nfc_baud_rate remote_dep_rate = NBR_UNDEFINED;

    LOG(LOG_NAME, "start listennig...");
    while (!thread_end) {
        if (nfc_initiator_poll_dep_target(pnd, remote_dep_mode, remote_dep_rate, NULL, &target, 1000) < 0) {
            sleep(500);
            continue;
        }
        LOG(LOG_NAME, "recevie a connect");
        LOG(LOG_NAME, "reading message...");
        while (!thread_end) {
            ret = nfc_initiator_transceive_bytes(pnd, NULL, 0, buffer, buffer_size, 0);
            if (ret < 0) {
                LOG(LOG_NAME, "out connect");
                break;
            }
            ret = slide_wnd_push(wnd, buffer, ret);
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
    }

    nfc_close(pnd);
    nfc_exit(context);
    LOG(LOG_NAME, "nfc thread end");
    return (void *) 0;
}

void endNfcMainThread() {
    thread_end = 1;
}