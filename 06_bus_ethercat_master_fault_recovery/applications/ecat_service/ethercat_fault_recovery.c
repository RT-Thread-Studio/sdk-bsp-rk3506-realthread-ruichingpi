/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include "rtthread.h"
#include <stdint.h>
#include <stdlib.h>
#include "ecat_master.h"

#define ECAT_PROCESS_DATA_SIZE 4096
#define ECAT_OP_TIMEOUT_MS 60000
#define ECAT_FAULT_WAIT_MS 10
#define ECAT_FAULT_HISTORY_BATCH 4
#define ECAT_RECOVERY_LINK_WAIT_RETRY 100
#define ECAT_RECOVERY_STABLE_WAIT_RETRY 50
#define ECAT_RECOVERY_POLL_MS 100
#define ECAT_RECOVERY_SETTLE_MS 500
#define ECAT_MONITOR_PERIOD_MS 100
#define CIA402_TARGET_STEP 1000
#define CIA402_MODE_CSP 0x08

#pragma pack(push, 1)
struct rpdo_csp
{
    uint8_t mode_byte;
    uint16_t control_word;
    int32_t dest_pos;
    int32_t dest_speed;
    int16_t dest_torque;
};

struct tpdo_csp
{
    uint16_t error_word;
    uint16_t status_word;
    int32_t cur_pos;
    int32_t cur_speed;
    int16_t curr_torque;
};
#pragma pack(pop)

static uint8_t servo_run = 0;
static uint8_t servo_dir = 1;
static uint8_t ecat_process_data[ECAT_PROCESS_DATA_SIZE];

static volatile uint8_t link_down_detected = 0;
static volatile uint8_t link_up_detected = 0;
static volatile uint8_t ecat_master_stop_requested = 0;
static volatile uint8_t ecat_master_recovery_requested = 0;
static volatile uint8_t ecat_master_ready = 0;
static volatile uint8_t ecat_master_running = 0;
static volatile uint8_t ecat_recovery_running = 0;
static uint32_t ecat_fault_sequence = 0;
static uint32_t ecat_last_fault_id = 0;
static int ecat_expected_slave_count = -1;

static ec_slave_config_t cia402_slave_config;

static ec_master_t demo_master = {
    .name = "fault-demo-master",
    .nic0 = "e1",
    .main_cycletime_us = 1000,
    .sub_cycletime_us = 5000,
    .recovery_timeout_ms = 3000,
    .process_data = ecat_process_data,
    .process_data_size = sizeof(ecat_process_data),
    .dc_active = 1,
    .dc_cycltime0 = 1000000,
    .dc_cyclshift = 500000,
    .dc_index = 1,
    .net_mode = EC_NET_MODE_EXCLUSIVE,
    .priority = 1,
    .pgain = 0.01f,
    .igain = 0.00002f,
};

static ec_pdo_entry_info_t cia402_output_pdo_entries[] = {
    { 0x6060, 0x00, 8 },
    { 0x6040, 0x00, 16 },
    { 0x607A, 0x00, 32 },
    { 0x60FF, 0x00, 32 },
    { 0x6071, 0x00, 16 },
};

static ec_pdo_entry_info_t cia402_input_pdo_entries[] = {
    { 0x603F, 0x00, 16 },
    { 0x6041, 0x00, 16 },
    { 0x6064, 0x00, 32 },
    { 0x606C, 0x00, 32 },
    { 0x6077, 0x00, 16 },
};

static ec_pdo_info_t cia402_pdos[] = {
    { 0x1600, 5, cia402_output_pdo_entries },
    { 0x1a00, 5, cia402_input_pdo_entries },
};

static ec_sync_info_t cia402_syncs[] = {
    { 2, EC_DIR_OUTPUT, 1, &cia402_pdos[0], EC_WD_DISABLE },
    { 3, EC_DIR_INPUT, 1, &cia402_pdos[1], EC_WD_DISABLE },
};

static void servo_switch_op(struct rpdo_csp *rmap, struct tpdo_csp *tmap)
{
    int sta;

    sta = tmap->status_word & 0x3ff;

    if (tmap->status_word & 0x8)
    {
        rmap->control_word = 0x0;
        servo_run = 0;
        return;
    }

    switch (sta)
    {
    case 0x250:
    case 0x270: rmap->control_word = 0x6; break;
    case 0x231: rmap->control_word = 0x7; break;
    case 0x233: rmap->control_word = 0xf; break;
    default: break;
    }
}

static void pdo_callback(uint16_t slave_index, uint8_t *output, uint8_t *input)
{
    struct rpdo_csp *rmap = (struct rpdo_csp *)output;
    struct tpdo_csp *tmap = (struct tpdo_csp *)input;

    (void)slave_index;

    if (servo_run == 0)
    {
        do {
            servo_switch_op(rmap, tmap);
            rmap->control_word = 0x2;
            rmap++; tmap++;
        } while ((uint8_t *)rmap < input);
        return;
    }

    do {
        servo_switch_op(rmap, tmap);
        if (rmap->control_word == 0x7) {
            rmap->mode_byte = CIA402_MODE_CSP;
            rmap->dest_pos = tmap->cur_pos;
        }
        if (rmap->control_word == 0xf) {
            rmap->dest_pos = tmap->cur_pos;
            if (servo_dir == 0) {
                rmap->dest_pos -= CIA402_TARGET_STEP;
            } else {
                rmap->dest_pos += CIA402_TARGET_STEP;
            }
        }
        rmap++; tmap++;
    } while ((uint8_t *)rmap < input);
}

static const char *fault_level_name(uint16_t level)
{
    switch (level)
    {
    case EC_FAULT_LEVEL_INFO: return "INFO";
    case EC_FAULT_LEVEL_WARNING: return "WARNING";
    case EC_FAULT_LEVEL_ERROR: return "ERROR";
    case EC_FAULT_LEVEL_FATAL: return "FATAL";
    default: return "UNKNOWN";
    }
}

static const char *fault_type_name(uint16_t type)
{
    switch (type)
    {
    case EC_FAULT_TOPOLOGY_MISMATCH: return "TOPOLOGY_MISMATCH";
    case EC_FAULT_PORT_LINK_ABNORMAL: return "PORT_LINK_ABNORMAL";
    case EC_FAULT_SLAVE_LOST: return "SLAVE_LOST";
    case EC_FAULT_SLAVE_STATE_CHANGED: return "SLAVE_STATE_CHANGED";
    case EC_FAULT_SLAVE_STATE_ABNORMAL: return "SLAVE_STATE_ABNORMAL";
    case EC_FAULT_SLAVE_AL_ERROR: return "SLAVE_AL_ERROR";
    case EC_FAULT_SLAVE_WKC_LOW: return "SLAVE_WKC_LOW";
    case EC_FAULT_MASTER_WKC_LOW: return "MASTER_WKC_LOW";
    case EC_FAULT_DATAGRAM_TIMEOUT: return "DATAGRAM_TIMEOUT";
    case EC_FAULT_DATAGRAM_ERROR: return "DATAGRAM_ERROR";
    case EC_FAULT_MASTER_LINK_DOWN: return "MASTER_LINK_DOWN";
    case EC_FAULT_MASTER_LINK_UP: return "MASTER_LINK_UP";
    case EC_FAULT_COMM_QUALITY_WARNING: return "COMM_QUALITY_WARNING";
    case EC_FAULT_COMM_QUALITY_ERROR: return "COMM_QUALITY_ERROR";
    default: return "UNKNOWN";
    }
}

static const char *fault_state_name(uint8_t state)
{
    switch (state & 0x0f)
    {
    case EC_STATE_INIT: return "INIT";
    case EC_STATE_PRE_OP: return "PRE-OP";
    case EC_STATE_BOOT: return "BOOT";
    case EC_STATE_SAFE_OP: return "SAFE-OP";
    case EC_STATE_OPERATIONAL: return "OP";
    default: return "UNKNOWN";
    }
}

static int fault_should_stop_master(const ec_fault_record_t *fault)
{
    return fault->level >= EC_FAULT_LEVEL_ERROR ||
           fault->type == EC_FAULT_SLAVE_LOST ||
           fault->type == EC_FAULT_MASTER_LINK_DOWN ||
           fault->type == EC_FAULT_DATAGRAM_ERROR;
}

static void print_fault_suggestion(const ec_fault_record_t *fault)
{
    switch (fault->type)
    {
    case EC_FAULT_MASTER_LINK_DOWN:
    case EC_FAULT_SLAVE_LOST:
        rt_kprintf("[Fault] Suggestion: check cable, slave power and IN/OUT wiring, then run ecat_fault_clear and ecat_master_recover.\n");
        break;
    case EC_FAULT_SLAVE_AL_ERROR:
    case EC_FAULT_SLAVE_STATE_ABNORMAL:
        rt_kprintf("[Fault] Suggestion: check AL code and slave state, fix the device fault, then clear fault before recovery.\n");
        break;
    case EC_FAULT_SLAVE_WKC_LOW:
    case EC_FAULT_MASTER_WKC_LOW:
    case EC_FAULT_DATAGRAM_TIMEOUT:
    case EC_FAULT_DATAGRAM_ERROR:
        rt_kprintf("[Fault] Suggestion: check communication quality, cable shielding and cycle load.\n");
        break;
    default:
        break;
    }
}

static void print_fault_detail(const ec_fault_record_t *fault)
{
    rt_kprintf("[Fault] id=%u seq=%u level=%s type=%s slave=%u code=0x%x reason=%s\n",
               fault->fault_id,
               fault->sequence,
               fault_level_name(fault->level),
               fault_type_name(fault->type),
               fault->slave_index,
               fault->error_code,
               fault->reason);

    if (fault->slave_index == EC_FAULT_NO_SLAVE)
    {
        if (fault->actual_wkc || fault->expected_wkc)
        {
            rt_kprintf("[Fault] master wkc=%u/%u\n",
                       fault->actual_wkc,
                       fault->expected_wkc);
        }
    }
    else
    {
        rt_kprintf("[Fault] slave alias=%u vendor=0x%x product=0x%x state=%s al=0x%x wkc=%u/%u\n",
                   fault->alias,
                   fault->vendor_id,
                   fault->product_code,
                   fault_state_name(fault->current_state),
                   fault->al_status_code,
                   fault->actual_wkc,
                   fault->expected_wkc);

        for (int i = 0; i < EC_FAULT_PORT_COUNT; i++)
        {
            rt_kprintf("[Fault] port%d desc=0x%x link=%u loop=%u signal=%u next=%u delay=%u\n",
                       i,
                       fault->ports[i].desc,
                       fault->ports[i].link_up,
                       fault->ports[i].loop_closed,
                       fault->ports[i].signal_detected,
                       fault->ports[i].next_slave,
                       fault->ports[i].delay_to_next_dc);
        }
    }

    print_fault_suggestion(fault);
}

static void handle_fault_record(const ec_fault_record_t *fault)
{
    print_fault_detail(fault);

    if (fault->type == EC_FAULT_MASTER_LINK_UP)
    {
        link_up_detected = 1;
        rt_kprintf("[Fault] Link is up. Clear fault after checking wiring, then request recovery.\n");
    }

    if (fault->type == EC_FAULT_MASTER_LINK_DOWN ||
        fault->type == EC_FAULT_SLAVE_LOST)
    {
        link_down_detected = 1;
    }

    if (fault_should_stop_master(fault))
    {
        ecat_master_stop_requested = 1;
        servo_run = 0;
    }
}

static void poll_fault_records(ec_master_t *master)
{
    rt_err_t err;
    uint32_t new_sequence = ecat_fault_sequence;
    ec_fault_record_t records[ECAT_FAULT_HISTORY_BATCH];
    uint32_t actual_count = 0;

    err = ecat_fault_wait(master,
                          ecat_fault_sequence,
                          ECAT_FAULT_WAIT_MS,
                          &new_sequence);
    if (err != RT_EOK)
    {
        return;
    }

    ecat_fault_sequence = new_sequence;

    do
    {
        actual_count = 0;
        err = ecat_fault_get_history(master,
                                     ecat_last_fault_id,
                                     records,
                                     sizeof(records) / sizeof(records[0]),
                                     &actual_count);
        if (err != RT_EOK || actual_count == 0)
        {
            break;
        }

        for (uint32_t i = 0; i < actual_count; i++)
        {
            handle_fault_record(&records[i]);
            ecat_last_fault_id = records[i].fault_id;
        }
    } while (actual_count == (sizeof(records) / sizeof(records[0])));
}

static void cia402_config_init(void)
{
    cia402_slave_config.dc_assign_activate = 0x300;
    cia402_slave_config.dc_sync[0].cycle_time =
        demo_master.main_cycletime_us * 1000;
    cia402_slave_config.dc_sync[0].shift_time = 500000;
    cia402_slave_config.dc_sync[1].cycle_time = 0;
    cia402_slave_config.dc_sync[1].shift_time = 0;
    cia402_slave_config.pdo_callback = pdo_callback;
    cia402_slave_config.sync = cia402_syncs;
    cia402_slave_config.sync_count =
        sizeof(cia402_syncs) / sizeof(ec_sync_info_t);
}

static rt_err_t start_master_operational(ec_master_t *master)
{
    int slave_count;
    uint16_t state;
    rt_err_t err;

    slave_count = ecat_slavecount(master);
    if (slave_count <= 0)
    {
        return -RT_ERROR;
    }

    cia402_config_init();

    for (int i = 0; i < slave_count; i++)
    {
        err = ecat_slave_config(master, i, &cia402_slave_config);
        if (err != RT_EOK)
        {
            rt_kprintf("ethercat slave %d config failed, err:%d\n", i, err);
            return err;
        }
    }

    err = ecat_master_start(master);
    if (err != RT_EOK)
    {
        rt_kprintf("ethercat master start failed, err:%d\n", err);
        return err;
    }

    for (int i = 0; i < slave_count; i++)
    {
        state = EC_STATE_OPERATIONAL;
        err = ecat_check_state(master, i, &state, ECAT_OP_TIMEOUT_MS);
        if (err != RT_EOK)
        {
            rt_kprintf("Slave %d did not reach operational mode, err:%d\n", i, err);
            return err;
        }
    }

    ecat_fault_get_sequence(master, &ecat_fault_sequence);
    ecat_master_ready = 1;
    ecat_master_running = 1;
    if (ecat_expected_slave_count < 0)
    {
        ecat_expected_slave_count = slave_count;
    }

    return RT_EOK;
}

static rt_err_t check_recovery_slave_count(ec_master_t *master)
{
    int slave_count;

    slave_count = ecat_slavecount(master);
    if (slave_count <= 0)
    {
        return -RT_ERROR;
    }

    if (ecat_expected_slave_count >= 0 &&
        slave_count != ecat_expected_slave_count)
    {
        rt_kprintf("[Recovery] slave count mismatch: slaves=%d expected=%d\n",
                   slave_count,
                   ecat_expected_slave_count);
        return -RT_ERROR;
    }

    return RT_EOK;
}

static void handle_stop_request(ec_master_t *master)
{
    rt_err_t err;

    ecat_master_stop_requested = 0;
    servo_run = 0;

    if (!ecat_master_ready || !ecat_master_running)
    {
        ecat_master_running = 0;
        return;
    }

    err = ecat_master_stop(master);
    if (err != RT_EOK)
    {
        rt_kprintf("[Monitor] ethercat master stop failed, err:%d\n", err);
    }

    ecat_master_running = 0;
}

static rt_err_t recover_master(ec_master_t *master)
{
    rt_err_t err;

    {
        int retry = 0;
        ec_master_state_t state_info;
        while (retry < ECAT_RECOVERY_LINK_WAIT_RETRY)
        {
            ecat_master_state(master, &state_info);
            if (state_info.link_up && state_info.slaves_responding > 0)
            {
                break;
            }
            rt_thread_mdelay(ECAT_RECOVERY_POLL_MS);
            retry++;
        }
        if (retry >= ECAT_RECOVERY_LINK_WAIT_RETRY)
        {
            rt_kprintf("[Recovery] Timeout - no slaves found\n");
            return -RT_ETIMEOUT;
        }
    }

    {
        int prev = -1;
        int cur, retry = 0;
        while (retry < ECAT_RECOVERY_STABLE_WAIT_RETRY)
        {
            cur = ecat_slavecount(master);
            if (cur > 0 && cur == prev)
            {
                break;
            }
            prev = cur;
            rt_thread_mdelay(ECAT_RECOVERY_POLL_MS);
            retry++;
        }
        rt_thread_mdelay(ECAT_RECOVERY_SETTLE_MS);
    }

    err = check_recovery_slave_count(master);
    if (err != RT_EOK)
    {
        return err;
    }

    err = start_master_operational(master);
    if (err != RT_EOK)
    {
        rt_kprintf("[Recovery] Failed to recover master: %d\n", err);
        return err;
    }

    link_down_detected = 0;
    link_up_detected = 0;

    return RT_EOK;
}

static int ethercat_fault_demo_start(const char* ifname)
{
    int slave_count;
    rt_err_t err;

    ecat_service_init();

    if (ifname)
    {
        demo_master.nic0 = ifname;
    }

    err = ecat_master_init(&demo_master);
    if (err)
    {
        rt_kprintf("ethercat master init failed, err:%d\n", err);
        return err;
    }

    err = start_master_operational(&demo_master);
    if (err != RT_EOK)
    {
        return err;
    }

    rt_kprintf("EtherCAT fault recovery demo started.\n");

    while (1)
    {
        poll_fault_records(&demo_master);

        if (!ecat_recovery_running && ecat_master_stop_requested)
        {
            ecat_recovery_running = 1;
            handle_stop_request(&demo_master);
            ecat_recovery_running = 0;
        }
        else if (!ecat_recovery_running && ecat_master_recovery_requested)
        {
            ecat_recovery_running = 1;
            ecat_master_recovery_requested = 0;
            err = recover_master(&demo_master);
            if (err == RT_EOK)
            {
                slave_count = ecat_slavecount(&demo_master);
                rt_kprintf("[Monitor] Recovery OK, %d slaves\n", slave_count);
            }
            else
            {
                rt_kprintf("[Monitor] Recovery failed: %d\n", err);
            }
            ecat_recovery_running = 0;
        }
        rt_thread_mdelay(ECAT_MONITOR_PERIOD_MS);
    }

    return 0;
}

static void ethercat_entry(void *pram)
{
    (void)pram;
    ethercat_fault_demo_start("e1");
}

static void ecat_fault_demo_cmd(void)
{
    rt_thread_t tid = RT_NULL;

    tid = rt_thread_create("ecat_demo", ethercat_entry, RT_NULL, 20480, 15, 10);
    if (tid != RT_NULL)
    {
        rt_thread_control(tid, RT_THREAD_CTRL_BIND_CPU, (void *)2);
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("create ethercat thread fail.\n");
    }
}
MSH_CMD_EXPORT_ALIAS(ecat_fault_demo_cmd, ecat_fault_demo, start EtherCAT fault recovery demo);

static int ecat_master_stop_cmd(void)
{
    if (!ecat_master_ready || !ecat_master_running)
    {
        rt_kprintf("ethercat master is not running.\n");
        return -RT_ERROR;
    }

    servo_run = 0;
    ecat_master_stop_requested = 1;
    rt_kprintf("ethercat master stop requested.\n");

    return RT_EOK;
}
MSH_CMD_EXPORT_ALIAS(ecat_master_stop_cmd, ecat_master_stop, request EtherCAT master stop);

static int ecat_master_recover_cmd(void)
{
    ec_fault_record_t fault;

    if (!ecat_master_ready)
    {
        rt_kprintf("ethercat master is not ready.\n");
        return -RT_ERROR;
    }

    if (ecat_master_stop_requested)
    {
        rt_kprintf("ethercat master stop is pending, retry recovery later.\n");
        return -RT_ERROR;
    }

    if (ecat_master_running)
    {
        rt_kprintf("ethercat master is already running.\n");
        return -RT_ERROR;
    }

    if (ecat_recovery_running || ecat_master_recovery_requested)
    {
        rt_kprintf("ethercat master recovery is already pending.\n");
        return -RT_ERROR;
    }

    if (ecat_fault_get_current(&demo_master, &fault) == RT_EOK)
    {
        rt_kprintf("ethercat fault is latched, clear it before recovery.\n");
        print_fault_detail(&fault);
        return -RT_ERROR;
    }

    if (link_down_detected && !link_up_detected)
    {
        rt_kprintf("ethercat link is not confirmed up, retry recovery later.\n");
        return -RT_ERROR;
    }

    servo_run = 0;
    ecat_master_recovery_requested = 1;
    rt_kprintf("ethercat master recovery requested.\n");

    return RT_EOK;
}
MSH_CMD_EXPORT_ALIAS(ecat_master_recover_cmd, ecat_master_recover, request EtherCAT master recovery);

static int ecat_fault_show_cmd(void)
{
    rt_err_t err;
    ec_fault_record_t fault;
    ec_fault_record_t records[8];
    uint32_t actual_count = 0;

    if (!ecat_master_ready)
    {
        rt_kprintf("ethercat master is not ready.\n");
        return -RT_ERROR;
    }

    err = ecat_fault_get_current(&demo_master, &fault);
    if (err == RT_EOK)
    {
        rt_kprintf("[Fault] current latched fault:\n");
        print_fault_detail(&fault);
    }
    else
    {
        rt_kprintf("[Fault] no latched fault.\n");
    }

    err = ecat_fault_get_history(&demo_master,
                                 0,
                                 records,
                                 sizeof(records) / sizeof(records[0]),
                                 &actual_count);
    if (err != RT_EOK)
    {
        rt_kprintf("[Fault] history query failed, err:%d\n", err);
        return err;
    }

    rt_kprintf("[Fault] history count shown=%u\n", actual_count);
    for (uint32_t i = 0; i < actual_count; i++)
    {
        print_fault_detail(&records[i]);
    }

    return RT_EOK;
}
MSH_CMD_EXPORT_ALIAS(ecat_fault_show_cmd, ecat_fault_show, show EtherCAT fault records);

static int ecat_fault_clear_cmd(void)
{
    rt_err_t err;

    if (!ecat_master_ready)
    {
        rt_kprintf("ethercat master is not ready.\n");
        return -RT_ERROR;
    }

    err = ecat_fault_clear_all(&demo_master);
    if (err != RT_EOK)
    {
        rt_kprintf("ethercat fault clear failed, err:%d\n", err);
        return err;
    }

    ecat_fault_get_sequence(&demo_master, &ecat_fault_sequence);
    ecat_last_fault_id = 0;
    link_down_detected = 0;
    link_up_detected = 0;
    rt_kprintf("ethercat fault records cleared.\n");

    return RT_EOK;
}
MSH_CMD_EXPORT_ALIAS(ecat_fault_clear_cmd, ecat_fault_clear, clear EtherCAT fault records);

static int motor_run(void)
{
    ec_fault_record_t fault;

    if (!ecat_master_running)
    {
        rt_kprintf("ethercat master is not running.\n");
        return -RT_ERROR;
    }

    if (ecat_fault_get_current(&demo_master, &fault) == RT_EOK)
    {
        rt_kprintf("ethercat fault is latched, motor run rejected.\n");
        print_fault_detail(&fault);
        return -RT_ERROR;
    }

    servo_run = 1;
    return 0;
}
MSH_CMD_EXPORT(motor_run, motor run);

static int motor_stop(void)
{
    servo_run = 0;
    return 0;
}
MSH_CMD_EXPORT(motor_stop, motor stop);

void motor_dir(int argc, char *argv[])
{
    if (argc != 2)
    {
        return;
    }

    servo_dir = atoi(argv[1]) ? 1 : 0;
}
MSH_CMD_EXPORT(motor_dir, motor dir);
