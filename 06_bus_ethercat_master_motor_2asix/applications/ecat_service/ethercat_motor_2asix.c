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

#define CSP_TIMEOUTRXM 700000

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
static uint8_t process_data[4096];
static ec_master_t csp_master = {
    .name = "csp-master",
    .nic0 = "e1",
    .main_cycletime_us = 1000,   // 1ms
    .sub_cycletime_us = 5000,    // 5ms
    .recovery_timeout_ms = 3000, // 3s
    .process_data = process_data,
    .process_data_size = 4096,
    .net_mode = EC_NET_MODE_EXCLUSIVE,
};

static ec_pdo_entry_info_t asix1_output_pdo_entries[] = {
    { 0x6060, 0x00, 8 },  // 6060h(mode)
    { 0x6040, 0x00, 16 }, // 6040h(control)
    { 0x607A, 0x00, 32 }, // 607Ah(dest position)
    { 0x60FF, 0x00, 32 }, // 60FFh(dest speed)
    { 0x6071, 0x00, 16 }, // 6071h(dest torque)
};

static ec_pdo_entry_info_t asix1_input_pdo_entries[] = {
    { 0x603F, 0x00, 16 }, // 603Fh(error)
    { 0x6041, 0x00, 16 }, // 6041h(status)
    { 0x6064, 0x00, 32 }, // 6064h(current postion)
    { 0x606C, 0x00, 32 }, // 606Ch(current speed)
    { 0x6077, 0x00, 16 }, // 6077h(current torque)
};

static ec_pdo_entry_info_t asix2_output_pdo_entries[] = {
    { 0x6860, 0x00, 8 },  // 6060h(mode)
    { 0x6840, 0x00, 16 }, // 6040h(control)
    { 0x687A, 0x00, 32 }, // 607Ah(dest position)
    { 0x68FF, 0x00, 32 }, // 60FFh(dest speed)
    { 0x6871, 0x00, 16 }, // 6071h(dest torque)
};

static ec_pdo_entry_info_t asix2_input_pdo_entries[] = {
    { 0x683F, 0x00, 16 }, // 603Fh(error)
    { 0x6841, 0x00, 16 }, // 6041h(status)
    { 0x6864, 0x00, 32 }, // 6064h(current postion)
    { 0x686C, 0x00, 32 }, // 606Ch(current speed)
    { 0x6877, 0x00, 16 }, // 6077h(current torque)
};

ec_pdo_info_t slave_pdos[] = {
    { 0x1600, 5, asix1_output_pdo_entries },
    { 0x1610, 5, asix2_output_pdo_entries },
    { 0x1a00, 5, asix1_input_pdo_entries },
    { 0x1a10, 5, asix2_input_pdo_entries },
};

ec_sync_info_t slave_syncs[] = {
    { 2, EC_DIR_OUTPUT, 2, &slave_pdos[0], EC_WD_DISABLE },
    { 3, EC_DIR_INPUT, 2, &slave_pdos[2], EC_WD_DISABLE },
};

static void servo_switch_op(struct rpdo_csp *rmap, struct tpdo_csp *tmap)
{
    int sta;

    sta = tmap->status_word & 0x3ff;

    if (tmap->status_word & 0x8)
    {
        rmap->control_word = 0x80;
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

void pdo_callback(uint16_t slave_index, uint8_t *output, uint8_t *input)
{
    struct rpdo_csp *rmap = (struct rpdo_csp *)output;
    struct tpdo_csp *tmap = (struct tpdo_csp *)input;

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
        if (rmap->control_word == 7) {
            rmap->mode_byte = 0x8;
            rmap->dest_pos = tmap->cur_pos;
        }
        if (rmap->control_word == 0xf) {
            rmap->dest_pos = tmap->cur_pos;
            if (servo_dir == 0) {
                rmap->dest_pos -= 1000;
            } else {
                rmap->dest_pos += 1000;
            }
        }
        rmap++; tmap++;
    } while ((uint8_t *)rmap < input);
}

static int sv630nd_csp_mode(const char* ifname)
{
    int slave_counts;
    rt_err_t err;

    ecat_service_init();

    if (ifname)
    {
        csp_master.nic0 = ifname;
    }

    err = ecat_master_init(&csp_master);
    if (err)
    {
        rt_kprintf("ethercat master init failed, err:%d\n", err);
        return err;
    }

    slave_counts = ecat_slavecount(&csp_master);
    rt_kprintf("Found slaves count:%d\n", slave_counts);

    static ec_slave_config_t slave_cia402_config;

    slave_cia402_config.dc_assign_activate = 0x300;
    slave_cia402_config.dc_sync[0].cycle_time = csp_master.main_cycletime_us * 1000;
    slave_cia402_config.dc_sync[0].shift_time = 500000;
    slave_cia402_config.dc_sync[1].cycle_time = 0;
    slave_cia402_config.dc_sync[1].shift_time = 0;
    slave_cia402_config.pdo_callback = pdo_callback;
    slave_cia402_config.sync = slave_syncs;
    slave_cia402_config.sync_count = sizeof(slave_syncs) / sizeof(ec_sync_info_t);

    for(int i = 0; i < slave_counts; i++)
    {
        ecat_slave_config(&csp_master, i, &slave_cia402_config);
    }

    ecat_master_start(&csp_master);

    rt_kprintf("Motor 2 Asix CSP mode control started...\n");
    
    return 0;
}

static void ethercat_entry(void *pram)
{
    sv630nd_csp_mode("e1");
}

static void ect_csp(void)
{
    rt_thread_t tid = RT_NULL;

    tid = rt_thread_create("Ethercat", ethercat_entry, RT_NULL, 20480, 15, 10);
    if (tid != RT_NULL)
    {
        rt_thread_control(tid, RT_THREAD_CTRL_BIND_CPU, (void *)2);
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("create ethercat thread fail.\n");
    }

    return;
}
MSH_CMD_EXPORT(ect_csp, ect_csp);

static int motor_run(void)
{
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
    if (argc == 2)
    {
        if (atoi(argv[1]) == 0)
        {
            servo_dir = 0;
        }
        else
        {
            servo_dir = 1;
        }
    }
}
MSH_CMD_EXPORT(motor_dir, motor dir);
