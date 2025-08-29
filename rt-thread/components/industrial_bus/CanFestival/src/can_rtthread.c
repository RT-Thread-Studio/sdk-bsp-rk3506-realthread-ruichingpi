#include <rtthread.h>
#include <rtdevice.h>
#include "canfestival.h"
#include "timers_driver.h"
#include <stdint.h>

#define DBG_TAG "app.CANopen"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define MAX_MUTEX_WAIT_TIME 5000
#define MAX_SEM_WAIT_TIME   5000

struct can_app_struct
{
    const char *name;
    struct rt_semaphore sem;
};

static rt_device_t candev = RT_NULL;
static CO_Data *OD_Data = RT_NULL;
static rt_mutex_t canfstvl_mutex = RT_NULL;

static struct can_app_struct can_data = { CANFESTIVAL_CAN_DEVICE_NAME };

void EnterMutex(void)
{
    if (rt_mutex_take(canfstvl_mutex, MAX_MUTEX_WAIT_TIME) != RT_EOK)
    {
        LOG_E("canfestival take mutex failed!");
    }
}

void LeaveMutex(void)
{
    if (rt_mutex_release(canfstvl_mutex) != RT_EOK)
    {
        LOG_E("canfestival release mutex failed!");
    }
}

static rt_err_t can1ind(rt_device_t dev, rt_size_t size)
{
    rt_err_t err = rt_sem_release(&can_data.sem);
    if (err != RT_EOK)
    {
        LOG_E("canfestival release receive semaphore failed!");
    }
    return err;
}

unsigned char canSend(CAN_PORT port, Message *m)
{
    static int err_cnt = 0;
    struct rt_can_msg msg;

    msg.id = m->cob_id;
    msg.ide = RT_CAN_STDID;
    msg.rtr = m->rtr;
    msg.len = m->len;
    msg.sync = CAN_ASYNC;
    msg.priv = LOW_PRIORITY;
    msg.fdf = CAN_20;
    memcpy(msg.data, m->data, m->len);

    if (port == RT_NULL)
    {
        return 1;
    }

    rt_ssize_t write_size = rt_device_write(port, 0, &msg, 1);

    if (write_size <= 0)
    {
        LOG_W("canfestival send failed, err = %d msg.rtr:%d write_size:%d",
            rt_get_errno(), msg.rtr, write_size);
        if (++err_cnt >= 100)
        {
            setState(OD_Data, Stopped);
        }
        return 0xFF;
    }

    err_cnt = 0;
    return 0;
}

static rt_err_t can_set_baudrate(rt_device_t dev, rt_uint32_t baud)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t ver = 0;

    rt_device_control(dev, RT_CAN_CTL_GET_VERSION, (void *)&ver);
    ret = rt_device_control(dev, RT_CAN_CTL_SET_BAUD, (void *)baud);
    if (ret != RT_EOK)
    {
        LOG_W("set baud faild");
        return ret;
    }

    return RT_EOK;
}

static rt_device_t can_init(const char *name, enum CANBAUD baud)
{
    rt_device_t can;
    rt_err_t err;

    if (name == RT_NULL)
    {
        LOG_E("canfestival device name is NULL");
        return RT_NULL;
    }

    can = rt_device_find(name);
    if (can == RT_NULL)
    {
        return RT_NULL;
    }

    err = can_set_baudrate(can, baud);
    if (err != RT_EOK)
    {
        LOG_E("canfestival set baudrate failed, err = %d", err);
        return RT_NULL;
    }

    err = rt_device_open(can,
        (RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX));
    if (err != RT_EOK)
    {
        LOG_E("canfestival open device %s failed, err = %d", name, err);
        return RT_NULL;
    }

    err = rt_device_set_rx_indicate(can, can1ind);
    if (err != RT_EOK)
    {
        LOG_E("canfestival set rx indicate failed, err = %d", err);
        return RT_NULL;
    }

    return can;
}

void canopen_recv_thread_entry(void *parameter)
{
    Message co_msg;
    struct rt_can_msg msg[CAN_FRAMEWORK_TX_FIFO_SIZE];
    struct rt_can_msg *curr;
    rt_err_t err;

    rt_size_t read_size = 0;
    while (1)
    {
        err = rt_sem_take(&can_data.sem, MAX_SEM_WAIT_TIME);
        if (err != RT_EOK)
        {
            if (getState(OD_Data) == Operational)
            {
                LOG_W("canfestival wait receive timeout, err = %d", err);
            }
        }
        else
        {
            read_size =
                rt_device_read(candev, 0, msg, CAN_FRAMEWORK_TX_FIFO_SIZE);
            if (read_size <= 0)
            {
                LOG_W("canfestival receive faild, err = %d", rt_get_errno());
            }
            else
            {
                curr = &msg[read_size];
                curr -= read_size;

                while (read_size--)
                {
                    co_msg.cob_id = curr->id;
                    co_msg.len = curr->len;
                    co_msg.rtr = curr->rtr;
                    memcpy(co_msg.data, curr->data, curr->len);
                    EnterMutex();
                    canDispatch(OD_Data, &co_msg);
                    LeaveMutex();
                    curr++;
                }
            }
        }
    }
}

enum CANBAUD can_baud_to_rtt(s_BOARD *board)
{
    enum CANBAUD baud = CAN1MBaud;

    if (board->baudrate == RT_NULL)
    {
        return baud;
    }

    if (strcmp(board->baudrate, "500K") == 0)
    {
        baud = CAN500kBaud;
    }
    else if (strcmp(board->baudrate, "250K") == 0)
    {
        baud = CAN250kBaud;
    }
    else if (strcmp(board->baudrate, "125K") == 0)
    {
        baud = CAN125kBaud;
    }
    else
    {
        baud = CAN1MBaud;
    }

    return baud;
}

CAN_PORT canOpen(s_BOARD *board, CO_Data *d)
{
    rt_thread_t tid;
    rt_err_t ret;

    if ((board == RT_NULL) || (d == RT_NULL))
    {
        return RT_NULL;
    }

    candev = can_init(board->busname, can_baud_to_rtt(board));
    if (candev == RT_NULL)
    {
        rt_kprintf("can_init failed\n");
        return RT_NULL;
    }

    ret = rt_sem_init(&can_data.sem, "co-rx", 0, RT_IPC_FLAG_PRIO);
    if (ret != RT_EOK)
    {
        rt_kprintf("canfestival init semaphore failed, err = %d\n", ret);
        return RT_NULL;
    }

    canfstvl_mutex = rt_mutex_create("canfstvl", RT_IPC_FLAG_PRIO);
    if (canfstvl_mutex == RT_NULL)
    {
        return RT_NULL;
    }

    OD_Data = d;
    tid = rt_thread_create("cf_recv", canopen_recv_thread_entry, &can_data,
        10240, CANFESTIVAL_RECV_THREAD_PRIO, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        rt_mutex_delete(canfstvl_mutex);
        canfstvl_mutex = RT_NULL;
    }

    return candev;
}
