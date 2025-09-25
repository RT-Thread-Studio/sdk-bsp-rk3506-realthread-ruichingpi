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
    { 1, EC_DIR_OUTPUT, 1, &slave_pdos[0], EC_WD_DISABLE },
    { 1, EC_DIR_INPUT, 1, &slave_pdos[1], EC_WD_DISABLE },
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

static void lc_pdo_config(struct ec_master *master)
{
    uint32_t index = 0;
    uint32_t pdo_index;
    uint32_t i = 0;

    if (master->dc_type)
    {
        ecat_dc_config_ex(master, master->dc_index, master->dc_active,
            master->dc_cycltime0, master->dc_cycltime1,
            master->dc_cyclshift); // SYNC0 on slave 1
    }
    else
    {
        ecat_dc_config(master, master->dc_index, master->dc_active,
            master->dc_cycltime0, master->dc_cyclshift); // SYNC0 on slave 1
    }

    for (i = 0; i < sizeof(slave_syncs) / sizeof(ec_sync_info_t); i++)
    {
        if (slave_syncs[i].slave_pos != 0xffff)
        {
            if (slave_syncs[i].dir == EC_DIR_OUTPUT)
            {
                pdo_index = 0;
                // 1c12.0
                ecat_sdo_write_u32(master, slave_syncs[i].slave_pos, 0x1c12, 0,
                    pdo_index, EC_TIMEOUTRXM);
                while (pdo_index < slave_syncs[i].n_pdos)
                {
                    const ec_pdo_info_t *pdo_info =
                        &slave_syncs[i].pdos[pdo_index];
                    index = 0;
                    // 1c12.0
                    ecat_sdo_write_u16(master, slave_syncs[i].slave_pos, 0x1c12,
                        pdo_index + 1, pdo_info->index, EC_TIMEOUTRXM);
                    ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                        pdo_info->index, index, 0, EC_TIMEOUTRXM);
                    while (index < pdo_info->n_entries)
                    {
                        uint32_t sdo_data =
                            (pdo_info->entries[index].index << 16) |
                            (pdo_info->entries[index].subindex << 8) |
                            pdo_info->entries[index].bit_length;
                        ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                            pdo_info->index, index + 1, sdo_data,
                            EC_TIMEOUTRXM);
                        index++;
                    }
                    ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                        pdo_info->index, 0, index, EC_TIMEOUTRXM);
                    pdo_index++;
                }
                // 1c12.0
                ecat_sdo_write_u32(master, slave_syncs[i].slave_pos, 0x1c12, 0,
                    pdo_index, EC_TIMEOUTRXM);
            }
            else if (slave_syncs[i].dir == EC_DIR_INPUT)
            {
                pdo_index = 0;
                // 1c13.0
                ecat_sdo_write_u32(master, slave_syncs[i].slave_pos, 0x1c13, 0,
                    pdo_index, EC_TIMEOUTRXM);
                while (pdo_index < slave_syncs[i].n_pdos)
                {
                    const ec_pdo_info_t *pdo_info =
                        &slave_syncs[i].pdos[pdo_index];
                    index = 0;
                    // 1c13.0
                    ecat_sdo_write_u16(master, slave_syncs[i].slave_pos, 0x1c13,
                        pdo_index + 1, pdo_info->index, EC_TIMEOUTRXM);
                    ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                        pdo_info->index, index, 0, EC_TIMEOUTRXM);
                    while (index < pdo_info->n_entries)
                    {
                        uint32_t sdo_data =
                            (pdo_info->entries[index].index << 16) |
                            (pdo_info->entries[index].subindex << 8) |
                            pdo_info->entries[index].bit_length;
                        ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                            pdo_info->index, index + 1, sdo_data,
                            EC_TIMEOUTRXM);
                        index++;
                    }
                    ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                        pdo_info->index, 0, index, EC_TIMEOUTRXM);
                    pdo_index++;
                }
                // 1c13.0
                ecat_sdo_write_u32(master, slave_syncs[i].slave_pos, 0x1c13, 0,
                    pdo_index, EC_TIMEOUTRXM);
            }
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

    err = ecat_config_init(&csp_master, RT_FALSE);
    if (err)
    {
        rt_kprintf("ethercat master init failed, err:%d\n", err);
        return err;
    }

    slave_counts = ecat_slavecount(&csp_master);
    rt_kprintf("Found slaves count:%d\n", slave_counts);

    err = ecat_write_state(&csp_master, 0, EC_STATE_PRE_OP);
    if (err)
    {
        rt_kprintf("ecat_write_state PRE_OP failed\n");
        return err;
    }

    state = EC_STATE_PRE_OP;
    err = ecat_check_state(&csp_master, 0, &state, 2000000 * 3);
    if (err != RT_EOK)
    {
        rt_kprintf("Not all slaves reached PRE_OP state.\n");
        return err;
    }

    ecat_config_dc(&csp_master);

    lc_pdo_config(&csp_master);

    err = ecat_config_map_group(
        &csp_master, (void *)(csp_master.process_data), 0);
    if (err != RT_EOK)
    {
        rt_kprintf("ecat_config_map_group failed\n");
        return err;
    }
    
    err = ecat_write_state(&csp_master, 0, EC_STATE_SAFE_OP);
    if (err != RT_EOK)
    {
        rt_kprintf("ecat_write_state PRE_OP failed\n");
        return err;
    }

    state = EC_STATE_SAFE_OP;
    err = ecat_check_state(&csp_master, 0, &state, 20000000 * 3);
    if (err != RT_EOK)
    {
        rt_kprintf("Not all slaves reached SAFE_OP state.%d\n", err);
        return err;
    }

    err = ecat_write_state(&csp_master, 0, EC_STATE_OPERATIONAL);
    if (err != RT_EOK)
    {
        rt_kprintf("ecat_write_state PRE_OP failed\n");
        return err;
    }

    struct rpdo_csp *rmap = (struct rpdo_csp *)(csp_master.process_data);
    struct tpdo_csp *tmap =
        (struct tpdo_csp *)(csp_master.process_data + sizeof(struct rpdo_csp));
    rmap->control_word = 0x8;

    /* swith mode */
    int _chk = 400;
    do
    {
        servo_switch_op(rmap, tmap);
        ecat_send_processdata_group(&csp_master, 0);
        ecat_receive_processdata_group(&csp_master, 0, 2000000 * 3);
        state = EC_STATE_OPERATIONAL;
    } while (
        _chk-- && (ecat_check_state(&csp_master, 0, &state, 2000) != RT_EOK));

    struct ecat_timer t;
    ecat_timer_start(&t, 1000);

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

        ecat_send_processdata_group(&csp_master, 0);
        ecat_receive_processdata_group(&csp_master, 0, 2000 * 10);
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
        while (ecat_timer_is_expired(&t) == RT_FALSE);
        ecat_timer_start(&t, 1000);
stop:
        ecat_send_processdata_group(&csp_master, 0);
        ecat_receive_processdata_group(&csp_master, 0, 2000 * 10);
        rt_thread_delay(1);
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