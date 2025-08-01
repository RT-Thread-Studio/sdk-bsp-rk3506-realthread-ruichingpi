/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <sitronix_ts.h>

int sitronix_ts_get_device_status(struct sitronix_ts_data *ts_data)
{
    int ret;
    uint8_t buf[8];

    ret = sitronix_ts_reg_read(ts_data, STATUS_REG, buf, sizeof(buf));
    if (ret < 0)
    {
        rt_kprintf("%s: Read Status register error!(%d)\n", __func__, ret);
        return ret;
    }

    return (int)(buf[0] & 0x0F);
}

int sitronix_ts_get_fw_revision(struct sitronix_ts_data *ts_data)
{
    int ret = 0;
    uint8_t buf[4];

    ret = sitronix_ts_reg_read(ts_data, FIRMWARE_VERSION, buf, 1);
    if (ret < 0)
    {
        rt_kprintf("%s: Read FW Version error!(%d)\n", __func__, ret);
        return ret;
    }

    ts_data->ts_dev_info.fw_version = buf[0];

    ret = sitronix_ts_reg_read(ts_data, FIRMWARE_REVISION_3, buf, sizeof(buf));
    if (ret < 0)
    {
        rt_kprintf("%s: Read FW revision error!(%d)\n", __func__, ret);
        return ret;
    }

    rt_memcpy(&ts_data->ts_dev_info.fw_revision, buf, 4);

    return 0;
}

int sitronix_ts_get_customer_info(struct sitronix_ts_data *ts_data)
{
    int ret = 0;
    uint8_t buf[4];
    uint8_t bank;

    ret = sitronix_ts_reg_read(ts_data, MISC_CONTROL, buf, 1);
    if (ret < 0)
    {
        rt_kprintf("%s: Read MISC_CONTROL error!(%d)\n", __func__, ret);
        return ret;
    }

    bank = buf[0];
    buf[0] = (buf[0] & 0xFC) | 1;
    ret = sitronix_ts_reg_write(ts_data, MISC_CONTROL, buf, 1);

    ret = sitronix_ts_reg_read(ts_data, FIRMWARE_REVISION_3, buf, sizeof(buf));
    rt_memcpy(&ts_data->ts_dev_info.customer_info, buf, 4);

    buf[0] = bank;
    ret = sitronix_ts_reg_write(ts_data, MISC_CONTROL, buf, 1);

    return 0;
}

int sitronix_ts_get_resolution(struct sitronix_ts_data *ts_data)
{
    int ret = 0;
    uint8_t buf[4];

    ret = sitronix_ts_reg_read(ts_data, X_RESOLUTION_HIGH, buf, sizeof(buf));
    if (ret < 0)
    {
        rt_kprintf("%s: Read resolution error!(%d)\n", __func__, ret);
        return ret;
    }

    ts_data->ts_dev_info.x_res = (((uint16_t)buf[0] & 0x3F) << 8) | buf[1];
    ts_data->ts_dev_info.y_res = (((uint16_t)buf[2] & 0x3F) << 8) | buf[3];

    return 0;
}

int sitronix_ts_get_max_touches(struct sitronix_ts_data *ts_data)
{
    int ret = 0;
    uint8_t max_touches;

    ret = sitronix_ts_reg_read(
        ts_data, MAX_NUM_TOUCHES, &max_touches, sizeof(max_touches));
    if (ret < 0)
    {
        rt_kprintf("%s: Read max touches error!(%d)\n", __func__, ret);
        return ret;
    }

    ts_data->ts_dev_info.max_touches = max_touches;

    return 0;
}

int sitronix_ts_get_chip_id(struct sitronix_ts_data *ts_data)
{
    int ret = 0;
    uint8_t chip_id;

    ret = sitronix_ts_reg_read(ts_data, CHIP_ID, &chip_id, sizeof(chip_id));
    if (ret < 0)
    {
        rt_kprintf("%s: Read chip ID error!(%d)\n", __func__, ret);
        return ret;
    }

    ts_data->ts_dev_info.chip_id = chip_id;

    return 0;
}

int sitronix_ts_get_misc_info(struct sitronix_ts_data *ts_data)
{
    int ret = 0;
    uint8_t misc_info;

    ts_data->swu_flag = RT_FALSE;
    ts_data->is_support_proximity = RT_FALSE;
    ret =
        sitronix_ts_reg_read(ts_data, MISC_INFO, &misc_info, sizeof(misc_info));
    if (ret < 0)
    {
        rt_kprintf("%s: Read Misc. Info error!(%d)\n", __func__, ret);
        return ret;
    }

    ts_data->ts_dev_info.misc_info = misc_info;
    ts_data->swu_flag = (misc_info & ST_MISC_INFO_SWU_FLAG);
    ts_data->is_support_proximity = (misc_info & ST_MISC_INFO_PROX_FLAG);
    ts_data->is_support_coord_chksum =
        (misc_info & ST_MISC_INFO_COORD_CHKSUM_FLAG);

    // ts_data->mode_flag[1] = ts_data->swu_flag;

    return 0;
}

int sitronix_ts_get_device_info(struct sitronix_ts_data *ts_data)
{
    int ret;

    ret = sitronix_ts_get_fw_revision(ts_data);
    if (ret)
    {
        return ret;
    }
    ret = sitronix_ts_get_customer_info(ts_data);
    if (ret)
    {
        return ret;
    }
    ret = sitronix_ts_get_resolution(ts_data);
    if (ret)
    {
        return ret;
    }
    ret = sitronix_ts_get_max_touches(ts_data);
    if (ret)
    {
        return ret;
    }
    ret = sitronix_ts_get_chip_id(ts_data);
    if (ret)
    {
        return ret;
    }
    ret = sitronix_ts_get_misc_info(ts_data);
    if (ret)
    {
        return ret;
    }

    return ret;
}

void st_checksum_calculation(
    unsigned short *pChecksum, unsigned char *pInData, unsigned long Len)
{
    unsigned long i;
    unsigned char LowByteChecksum = 0;

    for (i = 0; i < Len; i++)
    {
        *pChecksum += (unsigned short)pInData[i];
        LowByteChecksum = (unsigned char)(*pChecksum & 0xFF);
        LowByteChecksum = (LowByteChecksum) >> 7 | (LowByteChecksum) << 1;
        *pChecksum = (*pChecksum & 0xFF00) | LowByteChecksum;
    }
}
