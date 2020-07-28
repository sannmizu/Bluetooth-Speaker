/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-24 22:14:49
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:40:24
 */ 
#ifndef SANN_MUSIC_TYPES_H
#define SANN_MUSIC_TYPES_H

#include <stdint.h>

#define MUSIC_HEAD_LEN  8

typedef struct music_protocol_head {
    uint8_t    type;         // 报文类型
    uint8_t    music_id;     // 音频表示
    uint16_t   len;          // 数据部分长度
    uint16_t   id;           // 计数器
    uint16_t   flag:3;       // 标记
    uint16_t   offset:13;    // 片偏移
} music_protocol_head;
#define MUSIC_CONTROL   0x01
#define MUSIC_DATA      0x02

#define FLAG_END        0b000
#define FLAG_CONTINUE   0b001

// MP3
#define MUSIC_TYPE_MP3          0x01
// WAV
#define MUSIC_TYPE_WAV          0x02

typedef struct music_protocol {
    music_protocol_head head;       // 头部数据
    char                data[0];    // 数据部分
} music_protocol;

// 控制报文数据部分
typedef struct music_signal {
    uint8_t    control_type;
    uint8_t    signal_type;
    uint8_t    music_id;
    uint8_t    checksum;
    char        data[0];        // 可能使用到的数据
} music_signal;

// 音乐传输信号
#define MUSIC_SIGNAL_TRANSPORT  0x01
typedef struct music_trans_data {
    uint16_t   time;           // 开头定位时间
    uint8_t    channel;        // 声道数
    uint8_t    format;         // 采样位数
    uint32_t   rate;           // 采样率
} music_trans_data;
// 准备传输新数据
#define MUSIC_TRANS_NEWDATA     0x01

// 音乐控制信号
#define MUSIC_SIGNAL_CONTROL    0x02
typedef struct music_control_data {
    uint16_t time;     // 命令定位时间
} music_control_data;
// 开始播放
#define MUSIC_CONTROL_START     0x01
// 暂停播放
#define MUSIC_CONTROL_PAUSE     0x02
// 前进，data为前进时间，单位秒
#define MUSIC_CONTROL_FORWARD   0x03
// 后退，data为后退时间，单位秒
#define MUSIC_CONTROL_BACKWARD  0x04
// 下一首
#define MUSIC_CONTROL_NEXT      0x05
// 上一首
#define MUSIC_CONTROL_PREV      0x06

#endif