/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-04-06 21:17:43
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:37:43
 */
#ifndef HCI_TYPES_H
#define HCI_TYPES_H
typedef struct hci_dev_info hci_dev_info;
struct hci_dev_info_list {
    uint16_t dev_num;
    struct hci_dev_info dev_info[0];
};
struct bd_addr_list {
    uint16_t bd_num;
    bdaddr_t bd_info[0];
};

#endif