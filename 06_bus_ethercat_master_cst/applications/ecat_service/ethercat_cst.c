/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rtthread.h>
#include "ecat_master.h"
#include <stdlib.h>

#define CST_TIMEOUTRXM 700000

#pragma pack(push, 1)
struct rpdo_cst
{
    uint16_t control_word;    // 0x6040 控制字
    int16_t target_torque;    // 0x6071 目标转矩
    int16_t torque_offset;    // 0x60B2 转矩偏置
    int8_t mode_of_operation; // 0x6060 模式选择 (CST=10)

    uint16_t max_torque;       // 0x6072 最大允许转矩 (百分比/额定)
    uint32_t max_motor_speed;  // 0x607F 最大转速 (rpm)
    uint32_t torque_limit_pos; // 0x60E0 正向转矩限制
    uint32_t torque_limit_neg; // 0x60E1 反向转矩限制
};

struct tpdo_cst
{
    uint16_t status_word;    // 0x6041 状态字
    int32_t actual_position; // 0x6064 实际位置
    int32_t actual_velocity; // 0x606C 实际速度
    int16_t actual_torque;   // 0x6077 实际转矩
    int8_t mode_display;     // 0x6061 运行模式显示
};
#pragma pack(pop)

static uint8_t servo_run = 0;
static uint8_t servo_dir = 0;
static uint8_t process_data[4096];
static ec_master_t cst_master = {
    .name = "cst-master",
    .nic0 = "e1",
    .main_cycletime_us = 1000,   // 1ms
    .sub_cycletime_us = 5000,    // 5ms
    .recovery_timeout_ms = 3000, // 3s
    .process_data = process_data,
    .process_data_size = 4096,
    .dc_active = 1,
    .dc_cycltime0 = 1000000,
    .dc_cyclshift = 500000,
    .dc_index = 1,
    .net_mode = EC_NET_MODE_EXCLUSIVE,
    .priority = 1,
    .pgain = 0.01f,
    .igain = 0.00002f,
};

static ec_pdo_entry_info_t slave1_rxpdo_entries[] = {
    { 0x6040, 0x00, 16 },
    { 0x6071, 0x00, 16 },
    { 0x60B2, 0x00, 16 },
    { 0x6060, 0x00, 8 },
    { 0x6072, 0x00, 16 },
    { 0x607F, 0x00, 32 },
    { 0x60E0, 0x00, 32 },
    { 0x60E1, 0x00, 32 },
};

static ec_pdo_entry_info_t slave1_txpdo_entries[] = {
    { 0x6041, 0x00, 16 },
    { 0x6064, 0x00, 32 },
    { 0x606C, 0x00, 32 },
    { 0x6077, 0x00, 16 },
    { 0x6061, 0x00, 8 },
};

ec_pdo_info_t slave_pdos[] = {
    { 0x1600, 8, slave1_rxpdo_entries },
    { 0x1a00, 5, slave1_txpdo_entries },
    { 0x1600, 8, slave1_rxpdo_entries },
    { 0x1a00, 5, slave1_txpdo_entries },
};

ec_sync_info_t slave_syncs[] = {
    { 1, EC_DIR_OUTPUT, 1, &slave_pdos[0], EC_WD_DISABLE },
    { 1, EC_DIR_INPUT, 1, &slave_pdos[1], EC_WD_DISABLE },
    { 2, EC_DIR_OUTPUT, 1, &slave_pdos[2], EC_WD_DISABLE },
    { 2, EC_DIR_INPUT, 1, &slave_pdos[3], EC_WD_DISABLE },
};

static int sv660n_pdo_config(ec_master_t *master, int slave_counts)
{
    uint32_t index = 0;
    uint32_t pdo_index;
    uint32_t i = 0;

    for (size_t i = 1; i <= slave_counts; i++)
    {
        if (master->dc_type)
        {
            ecat_dc_config_ex(master, i, master->dc_active,
                master->dc_cycltime0, master->dc_cycltime1,
                master->dc_cyclshift); // SYNC0 on slave 1
        }
        else
        {
            ecat_dc_config(master, i, master->dc_active,
                master->dc_cycltime0, master->dc_cyclshift); // SYNC0 on slave 1
        }
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
                    pdo_index, CST_TIMEOUTRXM);
                while (pdo_index < slave_syncs[i].n_pdos)
                {
                    const ec_pdo_info_t *pdo_info =
                        &slave_syncs[i].pdos[pdo_index];
                    index = 0;
                    // 1c12.0
                    ecat_sdo_write_u16(master, slave_syncs[i].slave_pos, 0x1c12,
                        pdo_index + 1, pdo_info->index, CST_TIMEOUTRXM);
                    ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                        pdo_info->index, index, 0, CST_TIMEOUTRXM);
                    while (index < pdo_info->n_entries)
                    {
                        uint32_t sdo_data =
                            (pdo_info->entries[index].index << 16) |
                            (pdo_info->entries[index].subindex << 8) |
                            pdo_info->entries[index].bit_length;
                        ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                            pdo_info->index, index + 1, sdo_data,
                            CST_TIMEOUTRXM);
                        index++;
                    }
                    ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                        pdo_info->index, 0, index, CST_TIMEOUTRXM);
                    pdo_index++;
                }
                // 1c12.0
                ecat_sdo_write_u32(master, slave_syncs[i].slave_pos, 0x1c12, 0,
                    pdo_index, CST_TIMEOUTRXM);
            }
            else if (slave_syncs[i].dir == EC_DIR_INPUT)
            {
                pdo_index = 0;
                // 1c13.0
                ecat_sdo_write_u32(master, slave_syncs[i].slave_pos, 0x1c13, 0,
                    pdo_index, CST_TIMEOUTRXM);
                while (pdo_index < slave_syncs[i].n_pdos)
                {
                    const ec_pdo_info_t *pdo_info =
                        &slave_syncs[i].pdos[pdo_index];
                    index = 0;
                    // 1c13.0
                    ecat_sdo_write_u16(master, slave_syncs[i].slave_pos, 0x1c13,
                        pdo_index + 1, pdo_info->index, CST_TIMEOUTRXM);
                    ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                        pdo_info->index, index, 0, CST_TIMEOUTRXM);
                    while (index < pdo_info->n_entries)
                    {
                        uint32_t sdo_data =
                            (pdo_info->entries[index].index << 16) |
                            (pdo_info->entries[index].subindex << 8) |
                            pdo_info->entries[index].bit_length;
                        ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                            pdo_info->index, index + 1, sdo_data,
                            CST_TIMEOUTRXM);
                        index++;
                    }
                    ecat_sdo_write_u32(master, slave_syncs[i].slave_pos,
                        pdo_info->index, 0, index, CST_TIMEOUTRXM);
                    pdo_index++;
                }
                // 1c13.0
                ecat_sdo_write_u32(master, slave_syncs[i].slave_pos, 0x1c13, 0,
                    pdo_index, CST_TIMEOUTRXM);
            }
        }
    }

    return 0;
}

static void servo_switch_op(struct rpdo_cst *rmap, struct tpdo_cst *tmap)
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

rt_inline struct rpdo_cst *servo_rpdo_get(ec_master_t *mater, int slave)
{
    return (struct rpdo_cst *)(mater->process_data +
        sizeof(struct rpdo_cst) * (slave - 1));
}

rt_inline struct tpdo_cst *servo_tpdo_get(
    ec_master_t *mater, int slave_count, int slave)
{
    return (struct tpdo_cst *)(mater->process_data +
        sizeof(struct rpdo_cst) * slave_count +
        sizeof(struct tpdo_cst) * (slave - 1));
}

static int sv660n_cst_mode(const char *ifname)
{
    int slave_counts;
    uint16_t state;
    rt_err_t err;

    ecat_service_init();

    if (ifname)
    {
        cst_master.nic0 = ifname;
    }

    err = ecat_master_init(&cst_master);
    if (err)
    {
        rt_kprintf("ethercat master init failed, err:%d\n", err);
        return err;
    }

    err = ecat_config_init(&cst_master, RT_FALSE);
    if (err)
    {
        rt_kprintf("ethercat master init failed, err:%d\n", err);
        goto cfg_init_err;
    }

    slave_counts = ecat_slavecount(&cst_master);
    rt_kprintf("Found slaves count:%d\n", slave_counts);

    err = ecat_write_state(&cst_master, 0, EC_STATE_PRE_OP);
    if (err)
    {
        rt_kprintf("ecat_write_state PRE_OP failed\n");
        goto write_per_op_err;
    }

    state = EC_STATE_PRE_OP;
    err = ecat_check_state(&cst_master, 0, &state, 2000000 * 3);
    if (err != RT_EOK)
    {
        rt_kprintf("Not all slaves reached PRE_OP state.\n");
        goto check_per_op_err;
    }

    ecat_config_dc(&cst_master);

    sv660n_pdo_config(&cst_master, slave_counts);

    err = ecat_config_map_group(
        &cst_master, (void *)(cst_master.process_data), 0);
    if (err != RT_EOK)
    {
        rt_kprintf("ecat_config_map_group failed\n");
        goto map_err;
    }

    err = ecat_write_state(&cst_master, 0, EC_STATE_SAFE_OP);
    if (err != RT_EOK)
    {
        rt_kprintf("ecat_write_state PRE_OP failed\n");
        goto write_safe_op_err;
    }

    state = EC_STATE_SAFE_OP;
    err = ecat_check_state(&cst_master, 0, &state, 20000000 * 3);
    if (err != RT_EOK)
    {
        rt_kprintf("Not all slaves reached SAFE_OP state.%d\n", err);
        goto check_safe_op_err;
    }

    err = ecat_write_state(&cst_master, 0, EC_STATE_OPERATIONAL);
    if (err != RT_EOK)
    {
        rt_kprintf("ecat_write_state PRE_OP failed\n");
        goto write_op_err;
    }

    struct rpdo_cst *rmap;
    struct tpdo_cst *tmap;

    /* swith mode */
    int _chk = 400;
    do
    {
        for (size_t slave = 1; slave <= slave_counts; slave++)
        {
            rmap = servo_rpdo_get(&cst_master, slave);
            tmap = servo_tpdo_get(&cst_master, slave_counts, slave);
            rmap->mode_of_operation = 0xA;
            servo_switch_op(rmap, tmap);
        }

        ecat_send_processdata_group(&cst_master, 0);
        ecat_receive_processdata_group(&cst_master, 0, 2000000 * 3);
        state = EC_STATE_OPERATIONAL;
    } while (
        _chk-- && (ecat_check_state(&cst_master, 0, &state, 2000) != RT_EOK));
    if (_chk < 0)
    {
        rt_kprintf("Not all slaves reached operational mode.\n");
        return _chk;
    }

    ecat_hwtimer_start(&cst_master);

    while (1)
    {
        if (servo_run == 0)
        {
            for (size_t slave = 1; slave <= slave_counts; slave++)
            {
                rmap = servo_rpdo_get(&cst_master, slave);
                tmap = servo_tpdo_get(&cst_master, slave_counts, slave);
                servo_switch_op(rmap, tmap);
                rmap->control_word = 0x2;
            }
            goto stop;
        }

        for (size_t slave = 1; slave <= slave_counts; slave++)
        {
            rmap = servo_rpdo_get(&cst_master, slave);
            tmap = servo_tpdo_get(&cst_master, slave_counts, slave);

            if (rmap->control_word == 7)
            {
                rmap->mode_of_operation = 0xA;
            }
            if (rmap->control_word == 0xf)
            {
                rmap->torque_limit_pos = -3500;
                rmap->torque_limit_neg = 3500;
                rmap->max_motor_speed = (1 << 17);
                rmap->max_torque = 3500;
                if (servo_dir == 0)
                {
                    rmap->target_torque = 200;
                }
                else
                {
                    rmap->target_torque = -200;
                }
            }
            servo_switch_op(rmap, tmap);
        }
stop:
        ecat_send_processdata_group(&cst_master, 0);
        ecat_receive_processdata_group(&cst_master, 0, 2000 * 10);
        ecat_sync_dc(&cst_master);
    }

    return 0;
write_op_err:
check_safe_op_err:
write_safe_op_err:
map_err:
check_per_op_err:
write_per_op_err:
cfg_init_err:
    return -1;
}

static void ethercat_entry(void *pram)
{
    sv660n_cst_mode("e1");
}

static void ect_cst(void)
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
MSH_CMD_EXPORT(ect_cst, ect_cst);

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
