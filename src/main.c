/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-04-07 23:11:55
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-25 14:41:04
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "log.h"
#include "pipe.h"
#include "bluetooth.h"
#include "nfc.h"
#include "player.h"

#define LOG_NAME "main"

pthread_t bltThreadId;
pthread_t nfcThreadId;
pthread_t playThreadId;

void sig_int_handler(int sign) {
    LOG(LOG_NAME, "exiting");

    endBluetoothMainThread();
    endNfcMainThread();
    endPlayerMainThread();
}

int main() {
    signal(SIGINT, sig_int_handler);
    init_pipe();
    
    pthread_create(&bltThreadId, NULL, bluetoothMainThread, NULL);
    pthread_create(&nfcThreadId, NULL, nfcMainThread, NULL);
    pthread_create(&playThreadId, NULL, playerMainThread, NULL);

    pthread_join(bltThreadId, NULL);
    pthread_join(nfcThreadId, NULL);
    pthread_join(playThreadId, NULL);

    destroy_pipe();
    return 0;
}