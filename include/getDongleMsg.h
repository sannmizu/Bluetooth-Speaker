/*
 * @Descripttion : 获取本主机或者远程的所有的蓝牙适配器信息
 * @version      : 1
 * @Author       : sannmizu
 * @Date         : 2020-05-11 20:14:30
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:58:09
 */
#include "types.h"

struct hci_dev_info_list* getDongleLocal();
struct bd_addr_list* getDongleRemote(int dev_id);