/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __CAN_V2_H__
#define __CAN_V2_H__

#include <rtthread.h>

#define CAN_V2_FRAME_VERSION_STRING "CANv2 frame version: 1.0.0"

/* CAN framework sends thread priority asynchronously */
#ifndef CAN_FRAMEWORK_THREAD_PRIORITY
#define CAN_FRAMEWORK_THREAD_PRIORITY 2
#endif /* CAN_FRAMEWORK_THREAD_PRIORITY */

/* CAN framework asynchronously sends threads stack size */
#ifndef CAN_FRAMEWORK_THREAD_SIZE
#define CAN_FRAMEWORK_THREAD_SIZE 2048
#endif /* CAN_FRAMEWORK_THREAD_SIZE */

#ifndef CAN_FRAMEWORK_TX_FIFO_SIZE
#define CAN_FRAMEWORK_TX_FIFO_SIZE 40
#endif

#ifndef CAN_FRAMEWORK_RX_FIFO_SIZE
#define CAN_FRAMEWORK_RX_FIFO_SIZE 40
#endif

/* Control parameters need to be used in the peripheral driver */
/* STOP_SEND 和 ABORT_SEND 的差异, STOP_SEND 需要关掉控制器或者发送缓冲器,
   ABORT_SEND只中止当前发送帧  */
#define RT_CAN_CTL_INIT                           (0x20)
#define RT_CAN_CTL_STOP_SEND                      (0x21)
#define RT_CAN_CTL_START_SEND                     (0x22)
#define RT_CAN_CTL_ABORT_SEND                     (0x23)
#define RT_CAN_CTL_PARSE_ERR_CODE                 (0x24)
#define RT_CAN_CTL_SET_BAUD                       (0x25)
#define RT_CAN_CTL_SET_FD_DATA_BAUD               (0x26)
#define RT_CAN_CTL_SET_FILTER_LIST                (0x27)
#define RT_CAN_CTL_GET_FILTER_LIST                (0x28)
#define RT_CAN_CTL_GET_FIFO_STATUS                (0x29)
#define RT_CAN_CTL_CLR_FIFO_STATUS                (0x2A)
#define RT_CAN_CTL_GET_TX_STATUS                  (0x2B)
#define RT_CAN_CTL_CLR_TX_STATUS                  (0x2C)
#define RT_CAN_CTL_GET_VERSION                    (0x2D)
#define RT_CAN_CTL_SET_FD_ISO_MODE                (0x2E)
#define RT_CAN_SET_AUTO_RETX                      (0x2F)
#define RT_CAN_CTL_GET_STATUS                     (0x30)

/* Only parameters that need to be used in the framework */
#define RT_CAN_CTL_SET_EVENT_INT_CALLBACK         (0x40)
#define RT_CAN_CTL_SET_EVENT_THREAD_CALLBACK      (0x41)
#define RT_CAN_CTL_SET_EVENT_CALLBACK_MASK        (0x42)
#define RT_CAN_CTL_GET_EVENT_CALLBACK_MASK        (0x43)

/* The driving layer uses low 16bit, The framework uses 0xFF000000 start */
#define RT_CAN_ERR_CODE_RX_FIFO_FULL              (0xFF000000L)
#define RT_CAN_ERR_CODE_HARDWARE_RX_FIFO_OVERFLOW (0xFF000001L)
#define RT_CAN_ERR_CODE_BUS_OFF                   (0xFF000002L)

/* Drive the input parameters of the incoming framework ISR */
#define RT_CAN_EVENT_SEND_SUCCESS                 (1 << 0)
#define RT_CAN_EVENT_SEND_FAILURE                 (1 << 1)
#define RT_CAN_EVENT_SEND_TIMEOUT                 (1 << 2)
#define RT_CAN_EVENT_HIGH_PRIORITY_MSG            (1 << 31)
#define RT_CAN_EVENT_RECEIVE_SUCCESS              (1 << 3)
#define RT_CAN_EVENT_ERROR                        (1 << 4)
#define RT_CAN_EVENT_ALL_MASK                     (0x1F)
#define RT_CAN_EVENT_SEND_MASK                    (0x07)

#define RT_CAN_FIFO_STATUS_TX_EMPTY               (0 << 0)
#define RT_CAN_FIFO_STATUS_TX_NO_EMPTY            (1 << 0)
#define RT_CAN_FIFO_STATUS_RX_EMPTY               (0 << 1)
#define RT_CAN_FIFO_STATUS_RX_NO_EMPTY            (1 << 1)
#define RT_CAN_TX_STATUS_FAIL                     (0)
#define RT_CAN_TX_STATUS_SUCCESS                  (1)

#define RT_CAN_STDID                              (0)
#define RT_CAN_EXTID                              (1)
#define RT_CAN_DTR                                (0)
#define RT_CAN_RTR                                (1)

#define RT_CAN_VER_20A                            (1 << 0)
#define RT_CAN_VER_20B                            (1 << 1)
#define RT_CAN_VER_FD_ISO                         (1 << 2)
#define RT_CAN_VER_FD_NONISO                      (1 << 3)
#define RT_CAN_VER_20_MASK                        (0x3)
#define RT_CAN_VER_FD_MASK                        (0xC)

#define RT_CAN_FILTER_CARE                        (0)
#define RT_CAN_FILTER_UNCARE                      (1)
#define RT_CAN_FILTER_ENABLE                      (1)
#define RT_CAN_FILTER_DISABLE                     (0)

/*  +---------------------------------------------+
    |                 example:                    |
    |   ~                        ~                |
    |   | 7  6  5  4  3  2  1  0 | bit            |
    |   +------------------------+                |
    |   | 1  1  1  1  1  1  1  0 | data_mask[0]   |
    |   +------------------------+                |
    |   | 1  1  0  0  1  0  1  1 | data[0]        |
    |   +------------------------+                |
    |   | x  x  x  x  x  x  x  1 | mask           |
    |   +------------------------+                |
    | x: no care,  only rx rxdata[0].bit0 = 1 msg |
    +---------------------------------------------+  */

#define RT_CAN_CHECK_FILTER_DATA_CAPACITY                                      \
    ((1L << FILTER_DATA_0) | (1L << FILTER_DATA_1) | (1L << FILTER_DATA_2) |   \
        (1L << FILTER_DATA_3) | (1L << FILTER_DATA_4) |                        \
        (1L << FILTER_DATA_5) | (1L << FILTER_DATA_6) | (1L << FILTER_DATA_7))

#define RT_CAN_FILTER_CAPACITY(x) ((1L << (x)) & 0xFFFF)
#define RT_CAN_SET_FILTER_ITEM(                                                \
    list, no, idx, idmask, idex, idemask, rtrx, rtrmask, en)                   \
    {                                                                          \
        (list)->item[no].id = idx;                                             \
        (list)->item[no].id_mask = idmask;                                     \
        (list)->item[no].ide = idex;                                           \
        (list)->item[no].ide_mask = idemask;                                   \
        (list)->item[no].rtr = rtrx;                                           \
        (list)->item[no].rtr_mask = rtrmask;                                   \
        (list)->item[no].enable = en;                                          \
    }

#define RT_CAN_SET_FILTER_ITEM_DATA(list, no, data_no, datax, datamask)        \
    {                                                                          \
        (list)->item[no].data[data_no] = datax;                                \
        (list)->item[no].data_mask[data_no] = datamask;                        \
    }

#define RT_CAN_DLC_TO_LEN(x) __rt_can_dlc_list[(x) & 0xF]
#define RT_CAN_LEN_TO_DLC(x) rt_can_len_to_dlc_func(x)

static const rt_uint8_t __rt_can_dlc_list[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12,
    16, 20, 24, 32, 48, 64 };
rt_inline rt_uint8_t rt_can_len_to_dlc_func(rt_uint8_t len)
{
    rt_int32_t i = 15;
    len = (len > 64) ? 64 : len;
    do
    {
        if (__rt_can_dlc_list[i] < len)
        {
            return i + 1;
        }
    } while (i--);

    return 0;
}

enum CANBAUD
{
    CAN8MBaud = 8000UL * 1000,  /* 8  MBit/sec   */
    CAN5MBaud = 5000UL * 1000,  /* 5  MBit/sec   */
    CAN4MBaud = 4000UL * 1000,  /* 4  MBit/sec   */
    CAN2MBaud = 2000UL * 1000,  /* 2  MBit/sec   */
    CAN1MBaud = 1000UL * 1000,  /* 1  MBit/sec   */
    CAN800kBaud = 1000UL * 800, /* 800 kBit/sec  */
    CAN500kBaud = 1000UL * 500, /* 500 kBit/sec  */
    CAN250kBaud = 1000UL * 250, /* 250 kBit/sec  */
    CAN125kBaud = 1000UL * 125, /* 125 kBit/sec  */
    CAN100kBaud = 1000UL * 100, /* 100 kBit/sec  */
    CAN50kBaud = 1000UL * 50,   /* 50  kBit/sec  */
    CAN20kBaud = 1000UL * 20,   /* 20  kBit/sec  */
    CAN10kBaud = 1000UL * 10    /* 10  kBit/sec  */
};

enum AUTO_RETX
{
    RETRANSMISSION_ENABLE = 0,
    RETRANSMISSION_DISABLE
};

typedef enum can_message_priority
{
    LOW_PRIORITY = 0,
    HIGH_PRIORITY,
    PRIORITY_MAX_NUM = 0xFFFF,
} can_message_priority_t;

typedef enum can_message_sync
{
    CAN_ASYNC = 0,
    CAN_SYNC,
    SYNC_MAX_NUM = 0xFFFF,
} can_message_sync_t;

typedef enum can_message_frame
{
    CAN_20 = 0,
    CAN_FD,
    FRAME_MAX_NUM = 0xFFFF,
} can_message_frame_t;

typedef enum can_message_bit_rate
{
    CAN_SLOW = 0,
    CAN_FAST,
    BITRATE_MAX_NUM = 0xFFFF,
} can_message_bit_rate_t;

typedef enum can_fd_iso_mode
{
    CAN_FD_NON_ISO_MODE = 0,
    CAN_FD_ISO_MODE,
    FD_ISO_MAX_NUM = 0xFFFF,
} can_fd_iso_mode_t;

typedef enum can_work_status
{
    CAN_INIT_STATUS = 0,
    CAN_RUN_STATUS,
    CAN_STOP_STATUS,
    STATUS_MAX_NUM = 0xFFFF,
} can_work_status_t;

typedef enum can_filter_type
{
    OFF_FILTER = 0,
    HARDWARE_FILTER,
    SOFTWARE_FILTER,
    FILTER_MAX_NUM = 0xFFFF,
} can_filter_type_t;

typedef enum can_filter_capacity
{
    FILTER_DATA_0 = 0,
    FILTER_DATA_1,
    FILTER_DATA_2,
    FILTER_DATA_3,
    FILTER_DATA_4,
    FILTER_DATA_5,
    FILTER_DATA_6,
    FILTER_DATA_7,
    FILTER_ID,
    FILTER_IDE_INDEPENDENT,
    FILTER_IDE_SHARE,
    FILTER_RTR,
    FILTER_CAP_MAX_NUM = 0xFFFF,
} can_filter_capacity_t;

typedef enum can_filter_mode
{
    FILTER_MODE_SINGLE = 0,
    FILTER_MODE_IDE_RTR_OR,  /* 多个过滤表结果 or */
    FILTER_MODE_IDE_RTR_AND, /* 多个过滤表结果 and */
    FILTER_MODE_ID_RANGE,    /* ID范围过滤 */
} can_filter_mode_t;

typedef struct
{
    rt_uint32_t version;
    rt_uint32_t baud_rate;                   /* can2.0/canfd 仲裁域波特率 */
    rt_uint32_t fd_data_baud_rate;           /* canfd 数据域波特率 */
    rt_bool_t hardware_auto_resend_flag;     /* 硬件自动重发 */
    rt_bool_t software_filter_enabled;       /* 软件过滤使能 */
    rt_uint32_t hardware_resend_count_limit; /* 硬件重发次数上限 */
    rt_uint32_t recv_buf_size;               /* 接收缓冲区大小 */
    rt_uint32_t send_buf_size;               /* 接收缓冲区大小 */
    rt_uint32_t send_timeout_ms;             /* 发送超时时间 */
    rt_uint32_t clog_send_timeout_us;        /* 阻塞发送超时时间 */
    rt_uint32_t canfd_iso_mode;              /* canfd iso模式 */
} rt_can_config_t;

typedef struct
{
    /* software fifo */
    rt_uint8_t *buffer;
    rt_uint16_t put_index;
    rt_uint16_t get_index;
    rt_bool_t is_full;
} rt_can_rx_fifo;

struct rt_can_device
{
    struct rt_device parent;
    struct rt_can_ops *ops;
    rt_can_config_t config;
    can_work_status_t status;
    rt_uint32_t rece_valid_packets; /* 接收过滤后的包数量 */
    rt_can_rx_fifo rx_fifo;
    struct rt_messagequeue send_mq;
    struct rt_messagequeue priv_send_mq;
    struct rt_mutex lock;
    struct rt_semaphore sem;
    rt_uint32_t errcode;
    rt_uint32_t resend_count;
    struct rt_event event;
    struct rt_timer timer;
    rt_thread_t tx_thread;
    rt_uint32_t event_mask;
    rt_uint32_t interrupt_mask;
    void (*event_int_callback)(
        struct rt_can_device *dev, rt_uint32_t code, void *param);
    void (*event_thread_callback)(
        struct rt_can_device *dev, rt_uint32_t code, void *param);
};
typedef struct rt_can_device *rt_can_device_t;

struct rt_can_msg
{
    rt_uint32_t id   : 29;
    rt_uint32_t ide  : 1;
    rt_uint32_t rtr  : 1;
    rt_uint32_t res  : 1;
    rt_uint32_t len  : 8;
    rt_uint32_t fdf  : 1;
    rt_uint32_t brs  : 1;
    rt_uint32_t res1 : 6;
    rt_uint32_t priv : 4;
    rt_uint32_t sync : 4;
    rt_uint32_t res2 : 8;
    rt_uint8_t data[64];
};
typedef struct rt_can_msg *rt_can_msg_t;

struct rt_can_filter_item
{
    rt_uint8_t data[8];
    rt_uint8_t data_mask[8];
    rt_uint32_t id       : 29;
    rt_uint32_t ide      : 1;
    rt_uint32_t rtr      : 1;
    rt_uint32_t enable   : 1;
    rt_uint32_t id_mask  : 29;
    rt_uint32_t ide_mask : 1;
    rt_uint32_t rtr_mask : 1;
    rt_uint32_t res      : 1;
};
typedef struct rt_can_filter_item *rt_can_filter_item_t;

struct rt_can_filter_list
{
    rt_uint8_t number;
    rt_uint16_t capacity;
    rt_uint8_t mode;
    rt_can_filter_item_t item;
};
typedef struct rt_can_filter_list *rt_can_filter_list_t;

#if 0
struct rt_can_ops
{
    rt_err_t(*control)(struct rt_can_device* can, rt_uint32_t cmd, void* arg);
    rt_err_t(*sendmsg)(struct rt_can_device* can, rt_can_msg_t msg, can_message_sync_t sync);
    rt_err_t(*recvmsg)(struct rt_can_device* can, rt_can_msg_t msg);
};

void rt_hw_can_isr(struct rt_can_device* can, rt_uint32_t event);
rt_err_t rt_hw_can_register(struct rt_can_device* can, char* name, struct rt_can_ops* ops, void* data);
#endif

#endif /* __CAN_V2_H__ */
