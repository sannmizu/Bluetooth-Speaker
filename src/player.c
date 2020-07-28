/*
 * @Descripttion : 从数据堆中得到数据，播放音乐
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-05-21 15:24:18
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-25 14:26:42
 */ 
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "log.h"
#include "player.h"
#include "nodes/music.h"
#include "nodes/memory.h"
#include "pipe.h"

#define LOG_NAME "player"

#define MS_MAX_SIZE 9
typedef enum _music_state {
    STATE_PREPARED = 0,
    STATE_RUNNING,
    STATE_PAUSED
} music_state_t;


typedef struct music_t {
    int music_id;
    music_state_t state;        // 音乐状态
    unsigned int start_time;    // buffer开始对应的时间，单位秒
    unsigned int now_time;      // 当前播放的时间，单位秒
    unsigned int channel;       // 声道数
    snd_pcm_format_t format;    // 采样位数
    unsigned int rate;          // 采样率
    bt_buffer *buffer;
} music_t;

//==================== 全局变量 ==========================
// 所有线程退出标志
static volatile int thread_end = 0;

// 当前播放音乐nowMusic，预备播放音乐nextMusic
static music_t *nowMusic = NULL;
static music_t *nextMusic = NULL;

// 访问nowMusic, nextMusic的锁
static pthread_rwlock_t now_lock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t next_lock = PTHREAD_RWLOCK_INITIALIZER;

// 获取当前/预备播放音乐内容的线程id
static pthread_t nowThread = 0UL;
static pthread_t nextThread = 0UL;
// 音乐缓存播放线程id
static pthread_t playThread = 0UL;
// 控制音乐线程id
static pthread_t controlThread = 0UL;

// 控制音乐线程等待控制信号的信号量
pthread_cond_t control_cond = PTHREAD_COND_INITIALIZER;
// 访问control_cond的互斥锁
pthread_mutex_t control_cond_lock = PTHREAD_MUTEX_INITIALIZER;

static snd_pcm_t *pcm_handle = NULL;            // pcm句柄
static snd_pcm_hw_params_t *pcm_params = NULL;  // pcm参数

// 获取音乐内容线程
static void* music_read_thread(void *arg);
// 音乐缓存播放线程
static void* play_thread();
// 控制音乐线程
static void* control_thread();

//======================= 函数 ==========================
static void free_music(music_t *music);
static void change_music();
static int set_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, music_t *music);

static void new_music(int music_id, music_trans_data *mtd);
static void start_music(int music_id, music_control_data *mcd);
static void pause_music(int music_id, music_control_data *mcd);
static void forward_music(int music_id, music_control_data *mcd);
static void backword_music(int music_id, music_control_data *mcd);
static void next_music(int music_id, music_control_data *mcd);
static void prev_music(int music_id, music_control_data *mcd);

//======================= 主线程 =========================
void* playerMainThread() {
    // 初始化
    pthread_rwlock_init(&now_lock, NULL);
    pthread_rwlock_init(&next_lock, NULL);
    pthread_create(&playThread, NULL, play_thread, NULL);

    pthread_mutex_init(&control_cond_lock, NULL);
    pthread_cond_init(&control_cond, NULL);
    pthread_create(&controlThread, NULL, control_thread, NULL);

    music_signal *ms = (music_signal *) malloc(MS_MAX_SIZE);
    int ms_size = MS_MAX_SIZE;

    while (!thread_end) {
        // 读控制信号
        if (read_control_pipe(ms, &ms_size) != -1) {
            LOG(LOG_NAME, "recieve a signal");
            if (MUSIC_SIGNAL_TRANSPORT == ms->signal_type) {
                // Assert(ms_size == sizeof(music_trans_data) + 2)
                if (MUSIC_TRANS_NEWDATA == ms->control_type) {
                    new_music((int) ms->music_id, (music_trans_data *) ms->data);
                } else {
                    // 错误
                }
            } else if (MUSIC_SIGNAL_CONTROL == ms->signal_type) {
                // Assert(ms_size == sizeof(music_control_data) + 2)
                switch (ms->control_type)
                {
                case MUSIC_CONTROL_START:
                    start_music((int) ms->music_id, (music_control_data *) ms->data);
                    break;
                case MUSIC_CONTROL_PAUSE:
                    pause_music((int) ms->music_id, (music_control_data *) ms->data);
                    break;
                case MUSIC_CONTROL_FORWARD:
                    forward_music((int) ms->music_id, (music_control_data *) ms->data);
                    break;
                case MUSIC_CONTROL_BACKWARD:
                    backword_music((int) ms->music_id, (music_control_data *) ms->data);
                    break;
                case MUSIC_CONTROL_NEXT:
                    next_music((int) ms->music_id, (music_control_data *) ms->data);
                    break;
                case MUSIC_CONTROL_PREV:
                    prev_music((int) ms->music_id, (music_control_data *) ms->data);
                    break;
                default:
                    break;
                }
                pthread_mutex_lock(&control_cond_lock);
                pthread_cond_signal(&control_cond);
                pthread_mutex_unlock(&control_cond_lock);
            }
        } else {
            // 协议问题
        }
    }
    
    pthread_join(playThread, NULL);
    pthread_join(controlThread, NULL);
    pthread_cond_destroy(&control_cond);
    pthread_mutex_destroy(&control_cond_lock);
    pthread_rwlock_destroy(&next_lock);
    pthread_rwlock_destroy(&now_lock);
    LOG(LOG_NAME, "player thread end");
    return (void *) 0;
}

void endPlayerMainThread() {
    thread_end = 1;
}

static void free_music(music_t *music) {
    if (music != NULL) {
        if (music->buffer != NULL) {
            freeBuffer(music->buffer);
            music->buffer = NULL;
        }
        free(music);
    }
}

static void change_music() {
    pthread_rwlock_wrlock(&next_lock);
    if (nextMusic != NULL) {
        pthread_rwlock_wrlock(&now_lock);
        if (nowMusic != NULL) {
            close_pipe(nowMusic->music_id);
            pthread_join(nowThread, NULL);
            free_music(nowMusic);
        }
        nowMusic = nextMusic;
        nowThread = nextThread;
        nextMusic = NULL;
        nextThread = 0UL;
        pthread_rwlock_unlock(&now_lock);
    }
    pthread_rwlock_unlock(&next_lock);
}

static void* music_read_thread(void *arg) {
    music_t *music = (music_t *) arg;
    char buffer[1024];
    int size;

    while (1) {
        size = read_pipe(music->music_id, buffer, 1024);
        if (size <= 0) {
            break;
        }
        appendBuffer(buffer, size, music->buffer);
    }
}

//=================== 信号操作 ======================

static void new_music(int music_id, music_trans_data * mtd) {
    // 找到新的音乐，加载进缓存，设置等待控制信号后播放的歌曲
    if (open_pipe(music_id) == -1) {
        // error
        return;
    }
    music_t *m = (music_t *) malloc(sizeof(music_t));
    m->music_id = music_id;
    m->state = STATE_PREPARED;
    m->start_time = (unsigned int) mtd->time;
    m->now_time = m->start_time;
    if (mtd->channel & 0x01) {
        m->channel = 1U;
    } else {
        m->channel = 2U;
    }
    switch (mtd->format &0x03) {
        case 0x00:
            m->format = SND_PCM_FORMAT_U8;
            break;
        case 0x01:
            m->format = SND_PCM_FORMAT_U16_LE;
            break;
        case 0x02:
            m->format = SND_PCM_FORMAT_S8;
            break;
        case 0x03:
            m->format = SND_PCM_FORMAT_S16_LE;
            break;
    }
    m->rate = (unsigned int) mtd->rate;
    m->buffer = newBuffer();

    pthread_rwlock_wrlock(&next_lock);
    if (nextMusic != NULL) {
        close_pipe(nextMusic->music_id);
        pthread_join(nextThread, NULL);
        free_music(nextMusic);
    }
    nextMusic = m;
    pthread_rwlock_unlock(&next_lock);
    pthread_create(&nextThread, NULL, music_read_thread, m);
}

static void play_music(int music_id, music_control_data *mcd, int isRunnig) {
    LOG(LOG_NAME, "play");
    if (nowMusic != NULL && nowMusic->music_id == music_id) {
        nowMusic->state = isRunnig ? STATE_RUNNING : nowMusic->state;
        if (nowMusic->start_time > mcd->time) {
            nowMusic->now_time = nowMusic->start_time;
        } else {
            nowMusic->now_time = mcd->time;
        }
    } else if (nextMusic != NULL && nextMusic->music_id == music_id) {
        change_music();
        play_music(music_id, mcd, isRunnig);
    }
}

static void start_music(int music_id, music_control_data *mcd) {
    // 播放
    LOG(LOG_NAME, "start");
    play_music(music_id, mcd, 1);
}

static void pause_music(int music_id, music_control_data *mcd) {
    // 暂停
    LOG(LOG_NAME, "pause");
    if (nowMusic != NULL && nowMusic->music_id == music_id) {
        nowMusic->state = STATE_PAUSED;
        if (nowMusic->start_time > mcd->time) {
            nowMusic->now_time = nowMusic->start_time;
        } else {
            nowMusic->now_time = mcd->time;
        }
    }
}

static void forward_music(int music_id, music_control_data *mcd) {
    // 快进
    LOG(LOG_NAME, "forward");
    play_music(music_id, mcd, 0);
}

static void backword_music(int music_id, music_control_data *mcd) {
    // 回放
    LOG(LOG_NAME, "backword");
    play_music(music_id, mcd, 0);
}

static void next_music(int music_id, music_control_data *mcd) {
    // 下一首
    LOG(LOG_NAME, "next");
    start_music(music_id, mcd);
}

static void prev_music(int music_id, music_control_data *mcd) {
    // 上一首
    LOG(LOG_NAME, "previous");
    start_music(music_id, mcd);
}

//======================= 音乐线程 =======================

static void* play_thread() {
    int size, ret;
    char* buffer = NULL;

    // 打开设备
    LOG(LOG_NAME, "openning pcm...");
    ret = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (ret < 0) {
        // error
        ERR(LOG_NAME, "snd pcm open fail");
        return (void *) -1;
    }
    LOG(LOG_NAME, "success");
    // 设置阻塞
    snd_pcm_nonblock(pcm_handle, 0);
    // 分配params空间
    snd_pcm_hw_params_alloca(&pcm_params);

    while(!thread_end) {
        if (nowMusic == NULL) {
            change_music();
            pthread_mutex_lock(&control_cond_lock);
            pthread_cond_signal(&control_cond);
            pthread_mutex_unlock(&control_cond_lock);
            continue;
        }
        LOG(LOG_NAME, "setting up parameters...");
        size = set_params(pcm_handle, pcm_params, nowMusic);
        if (size < 0) {
            // error
            ERR(LOG_NAME, "snd pcm open fail");
            return (void *) -1;
        }
        LOG(LOG_NAME, "success");
        buffer = (char *) malloc(size);
        while(1) {
            ret = read_pipe(nowMusic->music_id, buffer, size);
            if (ret < 0) {
                break;
            }
            ret = snd_pcm_writei(pcm_handle, buffer, ret);
            if (ret == -EPIPE) {
                // 饥饿，数据传输过慢，TODO:通知App加快传输速度
            } else if (ret < 0) {
                break;
            }
        }
        free(buffer);
        buffer = NULL;
        snd_pcm_drain(pcm_handle);
        snd_pcm_hw_free(pcm_handle);
    }
    
    snd_pcm_close(pcm_handle);
    return (void *) 0;
}

// 设置参数
static int set_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, music_t *music) {
    int ret;
    // 初始化pcm参数，设置默认值
    snd_pcm_hw_params_any(pcm, params);
    // 交错模式
    snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    // 设置音频格式
    snd_pcm_hw_params_set_format(pcm, params, (snd_pcm_format_t) music->format);
    // 设置通道数
    snd_pcm_hw_params_set_channels(pcm, params, music->channel);
    // 设置采样率
    snd_pcm_hw_params_set_rate_near(pcm, params, &music->rate, 0);
    // 设置采样周期，系统自动设置
    int frames;
    // 设置可以暂停
    snd_pcm_hw_params_can_pause(params);
    // 设置好的参数回写设备
    ret = snd_pcm_hw_params(pcm, params);
        if (ret < 0) {
        // error
        perror("snd pcm param fail");
        return ret;
    }
    // 获取一个周期有多少帧数据，一个周期一个周期方式处理音频数据。
    snd_pcm_hw_params_get_period_size(params, (snd_pcm_uframes_t *) &frames, 0);
    // buffer_size = frames * bit/8 * channel
    int bytes;
    switch (music->format) {
        case SND_PCM_FORMAT_U8:
        case SND_PCM_FORMAT_S8:
            bytes = 1;
            break;
        case SND_PCM_FORMAT_U16_LE:
        case SND_PCM_FORMAT_S16_LE:
            bytes = 2;
            break;
    }
    return bytes * music->channel * frames;
}

static void* control_thread() {
    while (!thread_end) {
        pthread_mutex_lock(&control_cond_lock);
        pthread_cond_wait(&control_cond, &control_cond_lock);
        // 快速操作
        pthread_rwlock_rdlock(&now_lock);
        if (nowMusic != NULL) {
            snd_pcm_state_t state = snd_pcm_state(pcm_handle);
            // 获得当前播放的位置
            snd_pcm_uframes_t avail;
            snd_htimestamp_t timestamp;
            snd_pcm_htimestamp(pcm_handle, &avail, &timestamp);
            // 计算需要移动的帧数，时间差乘以每秒帧数，每秒帧数=采样率
            long loc = timestamp.tv_sec - abs(nowMusic->now_time - nowMusic->start_time);
            snd_pcm_uframes_t uframes = abs(loc) * nowMusic->rate;

            if (nowMusic->state == STATE_PAUSED) {
                snd_pcm_pause(pcm_handle, 1);
            } else if (nowMusic->state == STATE_RUNNING) {
                snd_pcm_pause(pcm_handle, 0);
            } else {
                goto control_thread_end;
            }

            snd_pcm_uframes_t widthable;
            if (loc >= 0) {
                widthable = snd_pcm_forwardable(pcm_handle);
                if (widthable < uframes) {
                    // error
                } else {
                    snd_pcm_forward(pcm_handle, uframes);
                }
            } else if (loc < 0) {
                widthable = snd_pcm_rewindable(pcm_handle);
                if (widthable < uframes) {
                    // error
                } else {
                    snd_pcm_rewind(pcm_handle, uframes);
                }
            }
        }
    control_thread_end:
        pthread_rwlock_unlock(&now_lock);
        pthread_mutex_unlock(&control_cond_lock);
    }
}