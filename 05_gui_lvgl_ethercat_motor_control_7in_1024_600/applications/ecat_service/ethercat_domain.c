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

#ifdef COMP_USING_ETHERCAT
#include "ecat_master.h"

#define EC_TIMEOUTRXM 700000

#pragma pack(1)
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
} ;
#pragma pack()

static uint8_t servo_run = 0;
static uint8_t servo_dir = 1;
static uint8_t process_data[4096];

static ec_master_t csp_master = {
    .name = "csp-master",
    .nic0 = "e1",
    .main_cycletime_us = 2000,   // 2ms
    .sub_cycletime_us = 5000,    // 5ms
    .recovery_timeout_ms = 3000, // 3s
    .process_data = process_data,
    .process_data_size = 4096,
    .dc_active = 1,
    .dc_cycltime0 = 5000000,
    .dc_cyclshift = 1000,
    .dc_index = 1,
    .net_mode = EC_NET_MODE_EXCLUSIVE,
    .priority = 1,
};

static ec_pdo_entry_info_t slave1_output_pdo_entries[] = {
    { 0x6060, 0x00, 8 },  // 6060h(mode)
    { 0x6040, 0x00, 16 }, // 6040h(control)
    { 0x607A, 0x00, 32 }, // 607Ah(dest position)
    { 0x60FF, 0x00, 32 }, // 60FFh(dest speed)
    { 0x6071, 0x00, 16 }, // 6071h(dest torque)
};

static ec_pdo_entry_info_t slave1_input_pdo_entries[] = {
    { 0x603F, 0x00, 16 }, // 603Fh(error)
    { 0x6041, 0x00, 16 }, // 6041h(status)
    { 0x6064, 0x00, 32 }, // 6064h(current postion)
    { 0x606C, 0x00, 32 }, // 606Ch(current speed)
    { 0x6077, 0x00, 16 }, // 6077h(current torque)
};

ec_pdo_info_t slave_pdos[] = {
    { 0x1600, 5, slave1_output_pdo_entries },
    { 0x1a00, 5, slave1_input_pdo_entries },
};

ec_sync_info_t slave_syncs[] = {
    { 2, EC_DIR_OUTPUT, 1, &slave_pdos[0], EC_WD_DISABLE },
    { 3, EC_DIR_INPUT, 1, &slave_pdos[1], EC_WD_DISABLE },
};

static void servo_switch_op(struct rpdo_csp *output, struct tpdo_csp *input)
{
    int sta;
    struct rpdo_csp *data = (struct rpdo_csp *)(output);
    struct tpdo_csp *status = (struct tpdo_csp *)(input);
    sta = status->status_word & 0x3ff;
    if (status->status_word & 0x8)
    {
        data->control_word = 0x80;
    }
    else
    {
        // swtich servo stattus, ref cia402
        switch (sta)
        {
        case 0x250:
        case 0x270:
            data->control_word = 0x6;
            ;
            break;
        case 0x231: data->control_word = 0x7; break;
        case 0x233: data->control_word = 0xf; break;
        default:
            // data->control_word = 0x6;
            break;
        }
    }
}

#define ANGLE_TO_PULSE(x)       ((((x) * 10) * 131071 / 3600) % 131072)
#define PULSE_TO_ANGLE(x)       (abs(((x) % 131072) / 131072.0f * 3600 / 10))

int32_t motor_target_pos = 0;
int32_t motor_current_pos = 0;
static int32_t motor_target_pos_last = -1;
static int32_t motor_step = 100;

static int lc_csp_mode(const char *ifname)
{
    int slave_counts;
    uint16_t state;
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
    slave_cia402_config.sync = slave_syncs;
    slave_cia402_config.sync_count = sizeof(slave_syncs) / sizeof(ec_sync_info_t);
    ecat_slave_config(&csp_master, 0, &slave_cia402_config);
    ecat_master_start(&csp_master);

    state = EC_STATE_OPERATIONAL;
    err = ecat_check_state(&csp_master, 0, &state, 20000000 * 3);
    if (err != RT_EOK)
    {
        rt_kprintf("Not all slaves reached operational mode.\n");
        return err;
    }

    struct rpdo_csp* rmap = (struct rpdo_csp*)(csp_master.process_data);
    struct tpdo_csp *tmap =
        (struct tpdo_csp *)(csp_master.process_data + sizeof(struct rpdo_csp));
    rmap->control_word = 0x8;

    while (1)
    {
        servo_switch_op(rmap, tmap);

        if (servo_run == 0)
        {
            rmap->control_word = 0x2;
            /* get the current position */
            motor_current_pos = PULSE_TO_ANGLE(tmap->cur_pos);
            rmap->dest_pos = tmap->cur_pos;
            goto stop;
        }

        if (rmap->control_word == 7)
        {
            rmap->mode_byte = 0x8;
            rmap->dest_pos = tmap->cur_pos;
        }
        if (rmap->control_word == 0xf)
        {
            if (motor_target_pos_last != motor_target_pos)
            {
                /* get the current position */
                motor_current_pos = PULSE_TO_ANGLE(tmap->cur_pos);

                if (abs(motor_current_pos - motor_target_pos) <= 0)
                {
                    rmap->dest_pos = tmap->cur_pos;
                    if (abs(tmap->cur_pos - rmap->dest_pos) <= 10)
                    {
                        motor_target_pos_last = motor_target_pos;
                    }
                    goto stop;
                }
                if (servo_dir)
                {
                    rmap->dest_pos -= motor_step;
                }
                else
                {
                    rmap->dest_pos += motor_step;
                }
            }  
        }
stop:
        rt_thread_delay(5);
    }

    return 0;
}

static void ethercat_entry(void *pram)
{
    lc_csp_mode("e1");
}

int ethercat_domain_init(void)
{
    rt_thread_t tid = RT_NULL;
    rt_thread_mdelay(1000);
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

    return 0;
}
MSH_CMD_EXPORT(ethercat_domain_init, ethercat_domain_init);

int motor_run(void)
{
    servo_run = 1;
    rt_kprintf("Motor started to move one revolution.\r\n");

    return 0;
}
MSH_CMD_EXPORT(motor_run, motor run);

int motor_stop(void)
{
    servo_run = 0;
    rt_kprintf("Motor returned to home position.\r\n");

    return 0;
}
MSH_CMD_EXPORT(motor_stop, motor stop);

void motor_dir_set(uint8_t dir)
{
    rt_kprintf("Switch the movement direction of the motor.\r\n");

    if (dir == 0)
    {
        servo_dir = 0;
    }
    else
    {
        servo_dir = 1;
    }
}

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

static void motor_status(void)
{
    rt_kprintf("motor status [%d]\r\n", servo_run);
    rt_kprintf("motor dir    [%d]\r\n", servo_dir);
}
MSH_CMD_EXPORT(motor_status, motor status);

#endif /* COMP_USING_ETHERCAT */