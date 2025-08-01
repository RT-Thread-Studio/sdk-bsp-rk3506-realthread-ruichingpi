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
typedef struct
{
    uint8_t mode_byte;
    uint16_t control_word;
    int32_t dest_pos;
    int32_t dest_speed;
    int16_t dest_torque;
} servo_data_t;

typedef struct
{
    uint16_t error_word;
    uint16_t status_word;
    int32_t cur_pos;
    int32_t cur_speed;
    int16_t curr_torque;
} servo_status_t;
#pragma pack()

static uint8_t servo_run = 0;
static uint8_t servo_dir = 1;
static uint8_t process_data[4096];
static uint32_t last_ms = 0;
static ec_master_t master1 = {
    .name = "master1",
    .nic0 = "e1",
    .main_cycletime_us = 1000,   // 1ms
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

static void servo_switch_op(uint8_t *input, uint8_t *output)
{
    int sta;
    servo_data_t *data = (servo_data_t *)(output);
    servo_status_t *status = (servo_status_t *)(input);
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

static void process_data_config_handler(struct ec_master *master)
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

static void ecat_process_data_begin_handler(
    struct ec_master *master, uint16_t slave, uint8_t *input, uint8_t *output)
{
    switch (slave)
    {
    case 1: // slave 1
    {
        if (master->state.al_states == EC_AL_STATE_OP)
        {
            servo_switch_op(input, output);
            servo_data_t *data = (servo_data_t *)(output);
            servo_status_t *status = (servo_status_t *)(input);
            if (servo_run)
            {
                if (data->control_word == 7)
                {
                    data->dest_pos = status->cur_pos;
                }
                else if (data->control_word == 0xf)
                {
                    if (servo_dir)
                    {
                        data->dest_pos -= 300;
                    }
                    else
                    {
                        data->dest_pos += 300;
                    }
                }
            }
            else
            {
                data->dest_pos = status->cur_pos;
                data->control_word = 0x0;
            }
        }
        else if (master->state.al_states == EC_AL_STATE_SAFEOP)
        {
            servo_data_t *data = (servo_data_t *)(output);
            if (data->mode_byte != 0x08) /* if not in CSP mode */
            {
                data->mode_byte = 0x08; /* set mode to CSP */
            }
            servo_switch_op(input, output);
        }
    }
    break;
    case 2: // slave 2
    {
        if (rt_tick_get() - last_ms > RT_TICK_PER_SECOND / 4)
        {
            static uint8_t index = 0;
            uint16_t *out = (uint16_t *)output;
            last_ms = rt_tick_get();
            *out = (0x01 << index);
            index++;
            if (index >= 16)
                index = 0;
        }
    }
    break;
    default: break;
    }
}

static void ecat_process_data_end_handler(
    struct ec_master *master, uint16_t slave, uint8_t *input, uint8_t *output)
{
}

static void ecat_error_handler(struct ec_master *master,
    uint32_t error_code,
    const unsigned char *error_str)
{
    rt_kprintf("error: master state=%d,error[%d]:%s\n", master->state.al_states,
        error_code, error_str);
}

extern void ecat_config_service_init(ec_master_t *m);
int ethercat_domain_init(void)
{
    rt_err_t ret = RT_EOK;
    static uint8_t inited = 0;
    
    rt_thread_mdelay(1000);
    if (inited)
    {
        rt_kprintf("ethercat always running!\n");
        return ret;
    }
    /* ethercat service init */
    ecat_service_init();

    ret = ecat_master_init(&master1);

    if (ret == RT_EOK)
    {
        rt_kprintf("ethercat master init successed!\n");

        ecat_config_service_init(&master1);
        ecat_set_config_handler(&master1, process_data_config_handler);
        ecat_set_process_data_begin_handler(
            &master1, ecat_process_data_begin_handler);
        ecat_set_process_data_end_handler(
            &master1, ecat_process_data_end_handler);
        ecat_set_error_handler(&master1, ecat_error_handler);
        rt_kprintf("ethercat master simple start!\n");
        ecat_simple_start(&master1);
        inited = 1;
    }
    else
    {
        rt_kprintf("error: ethercat master init failed,ret:%d\n", ret);
    }

    return 0;
}
MSH_CMD_EXPORT(ethercat_domain_init, ethercat domain init);

int motor_run(void)
{
    servo_run = 1;
    return 0;
}
MSH_CMD_EXPORT(motor_run, motor run);

int motor_stop(void)
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

#endif /* COMP_USING_ETHERCAT */