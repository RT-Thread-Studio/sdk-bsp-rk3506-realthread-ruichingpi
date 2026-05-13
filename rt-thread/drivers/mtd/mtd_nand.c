/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-12-05     Bernard      the first version
 */

/*
 * COPYRIGHT (C) 2012, Shanghai Real Thread
 */

#include <rtdevice.h>

rt_uint32_t rt_mtd_nand_read_id(struct rt_mtd_nand_device *device)
{
    RT_ASSERT(device->ops->read_id);
    return device->ops->read_id(device);
}

rt_err_t rt_mtd_nand_read(
    struct rt_mtd_nand_device *device,
    rt_off_t page,
    rt_uint8_t *data, rt_uint32_t data_len,
    rt_uint8_t *spare, rt_uint32_t spare_len)
{
    RT_ASSERT(device->ops->read_page);
    return device->ops->read_page(device, page, data, data_len, spare, spare_len);
}

rt_err_t rt_mtd_nand_write(
    struct rt_mtd_nand_device *device,
    rt_off_t page,
    const rt_uint8_t *data, rt_uint32_t data_len,
    const rt_uint8_t *spare, rt_uint32_t spare_len)
{
    RT_ASSERT(device->ops->write_page);
    return device->ops->write_page(device, page, data, data_len, spare, spare_len);
}

rt_err_t rt_mtd_nand_move_page(struct rt_mtd_nand_device *device,
        rt_off_t src_page, rt_off_t dst_page)
{
    RT_ASSERT(device->ops->move_page);
    return device->ops->move_page(device, src_page, dst_page);
}

rt_err_t rt_mtd_nand_erase_block(struct rt_mtd_nand_device *device, rt_uint32_t block)
{
    RT_ASSERT(device->ops->erase_block);
    return device->ops->erase_block(device, block);
}

rt_err_t rt_mtd_nand_check_block(struct rt_mtd_nand_device *device, rt_uint32_t block)
{
    if (device->ops->check_block)
    {
        return device->ops->check_block(device, block);
    }
    else
    {
        return -RT_ENOSYS;
    }
}

rt_err_t rt_mtd_nand_mark_badblock(struct rt_mtd_nand_device *device, rt_uint32_t block)
{
    if (device->ops->mark_badblock)
    {
        return device->ops->mark_badblock(device, block);
    }
    else
    {
        return -RT_ENOSYS;
    }
}

