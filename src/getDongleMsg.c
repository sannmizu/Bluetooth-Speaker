/*
 * @Descripttion : 获取本主机或者远程的所有的蓝牙适配器信息
 * @version      : 1
 * @Author       : sannmizu
 * @Date         : 2020-04-05 23:02:00
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:57:59
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <stdlib.h>

#include "getDongleMsg.h"

/**
 * @author: sannmizu
 * @name: getDongleLocal
 * @msg: 返回所有蓝牙适配器的信息，以动态数组方式
 * @param {void} 
 * @return: hci_dev_info_list*
 */
struct hci_dev_info_list* getDongleLocal() {
    // hci设备列表结构
    struct hci_dev_list_req *dl = NULL;
    // hci设备数组
    struct hci_dev_req *dr = NULL;

    // hci设备信息，i表示第i个设备
    struct hci_dev_info di;
    int i;
    // 存储所有信息的数组
    struct hci_dev_info_list *di_list = NULL;

    // HCI socket fd
    int hci_sck;

    // 先给存储信息的结构分配空间，分配最大空间
    if(dl = malloc(sizeof(uint16_t) + HCI_MAX_DEV * sizeof(struct hci_dev_req)), NULL == dl) {
        perror("Can't allocate memory");
        return NULL;
    }
    dl->dev_num = HCI_MAX_DEV;
    dr = dl->dev_req;

    // 创建一个HCI socket，和HCI层建立连接
    if(hci_sck = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI), hci_sck < 0) {
        perror("Can't open HCI socket");
        return NULL;
    }

    // 使用HCIGETDEVLIST,得到所有dongle的DeviceID。存放在dl中
    if (ioctl(hci_sck, HCIGETDEVLIST, (void *) dl) < 0) {
        perror("Can't get device list");
        return NULL;
    }

    // 动态创建di数组
    if(di_list = malloc(sizeof(uint16_t) + HCI_MAX_DEV * sizeof(struct hci_dev_info)), NULL == di_list) {
        perror("Can't allocate memory");
        return NULL;
    }
    di_list->dev_num = dl->dev_num;
    // 根据所有dongle的DeviceID获取所有dongle的信息
    for(i = 0; i < dl->dev_num; i++) {
        // 获取第i个dongle信息
        di.dev_id = dr[i].dev_id;
        ioctl(hci_sck, HCIGETDEVINFO, (void *)&di);
        // 存入di中
        memcpy(&di_list->dev_info[i], &di, sizeof(hci_dev_info));
    }
    // 释放资源
    free(dl);
    return di_list;
}

/**
 * @author: sannmizu
 * @name: getDongleRemote
 * @msg: 获取搜索到的远程蓝牙设备
 * @param {int} dev_id 
 * @return: bd_addr_list*
 */
struct bd_addr_list* getDongleRemote(int dev_id) {
    // 存储所有信息的数组
    struct bd_addr_list *bd_list = NULL;
    int i;

    // 搜索到的设备
    inquiry_info* ii;
    int ii_size;

    // HCI socket fd
    int hci_sck;
    // 使用设备号创建一个hcisocket
    hci_sck = hci_open_dev(dev_id);

    // 搜索设备
    ii_size = hci_inquiry(dev_id, 10, 0, NULL, &ii, IREQ_CACHE_FLUSH);
    if (ii_size < 0) {
        return NULL;
    }
    if (bd_list = malloc(sizeof(uint16_t) + ii_size * sizeof(bdaddr_t)), NULL == bd_list) {
        return NULL;
    }
    bd_list->bd_num = ii_size;
    for (i = 0; i < ii_size; i++) {
        bd_list->bd_info[i] = ii[i].bdaddr;
    }

    // 关闭socket
    hci_close_dev(hci_sck);

    return bd_list;
}