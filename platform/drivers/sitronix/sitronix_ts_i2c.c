/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <sitronix_ts_i2c.h>

#define I2C_RETRY_COUNT 1

static int sitronix_ts_i2c_parse_dt(
    struct rt_ofw_node *np, struct sitronix_ts_host_interface *host_if)
{
    struct rt_ofw_prop *prop;
    const fdt32_t *cell;
    rt_uint32_t value;

    prop = rt_ofw_get_prop(np, "sitronix_irq_gpio", RT_NULL);
    if (prop)
    {
        cell = rt_ofw_prop_next_u32(prop, RT_NULL, &value);
        cell =
            rt_ofw_prop_next_u32(prop, cell, (rt_uint32_t *)&host_if->irq_gpio);
    }
    else
    {
        host_if->irq_gpio = -1;
        return (-RT_EINVAL);
    }

    prop = rt_ofw_get_prop(np, "sitronix_rst_gpio", RT_NULL);
    if (prop)
    {
        cell = rt_ofw_prop_next_u32(prop, RT_NULL, &value);
        cell =
            rt_ofw_prop_next_u32(prop, cell, (rt_uint32_t *)&host_if->rst_gpio);
    }
    else
    {
        host_if->rst_gpio = -1;
        return (-RT_EINVAL);
    }

    return 0;
}

static int sitronix_ts_i2c_read(
    uint16_t addr, uint8_t *data, uint16_t length, void *if_data)
{
    struct sitronix_ts_i2c_data *i2c_data =
        (struct sitronix_ts_i2c_data *)if_data;
    uint8_t buf[2];
    uint8_t retry;
    int ret;

    struct rt_i2c_msg msgs[] = {
        {
            .addr = i2c_data->client_addr,
            .flags = RT_I2C_WR,
            .len = 2,
            .buf = buf,
        },
        {
            .addr = i2c_data->client_addr,
            .flags = RT_I2C_RD,
            .len = length,
            .buf = data,
        },
    };

    buf[0] = (addr >> 8) & 0xFF;
    buf[1] = (addr) & 0xFF;

    for (retry = 1; retry <= I2C_RETRY_COUNT; retry++)
    {
        if (rt_i2c_transfer(i2c_data->client, msgs, 2) == 2)
        {
            ret = length;
            break;
        }
        rt_kprintf("I2C retry %d\n", retry);
        rt_thread_mdelay(20);
    }

    if (retry > I2C_RETRY_COUNT)
    {
        rt_kprintf("%s: I2C read over retry limit\n", __func__);
        ret = -RT_EIO;
    }

    return ret;
}

static int sitronix_ts_i2c_write(
    uint16_t addr, uint8_t *data, uint16_t length, void *if_data)
{
    struct sitronix_ts_i2c_data *i2c_data =
        (struct sitronix_ts_i2c_data *)if_data;
    unsigned char *buf = NULL;
    uint8_t retry;
    int ret;

    struct rt_i2c_msg msg[] = { {
        .addr = i2c_data->client_addr,
        .flags = RT_I2C_WR,
        .len = length + 2,
    } };

    buf = rt_calloc(1, (length + 2));
    if (!buf)
    {
        rt_kprintf("%s: Alloc memory for buf failed!\n", __func__);
        return -RT_ENOMEM;
    }

    msg[0].buf = buf;
    buf[0] = (addr >> 8) & 0xFF;
    buf[1] = (addr) & 0xFF;
    rt_memcpy(&buf[2], &data[0], length);

    for (retry = 1; retry <= I2C_RETRY_COUNT; retry++)
    {
        if (rt_i2c_transfer(i2c_data->client, msg, 1) == 1)
        {
            ret = length;
            break;
        }
        rt_kprintf("I2C retry %d\n", retry);
        rt_thread_mdelay(8);
    }

    if (retry > I2C_RETRY_COUNT)
    {
        rt_kprintf("%s: I2C write over retry limit\n", __func__);
        ret = -RT_EIO;
    }

    rt_free(buf);

    return ret;
}

static int sitronix_ts_i2c_aread(uint8_t *tx_buf,
    uint16_t tx_len,
    uint8_t *rx_buf,
    uint16_t rx_len,
    void *if_data)
{
    struct sitronix_ts_i2c_data *i2c_data =
        (struct sitronix_ts_i2c_data *)if_data;
    uint8_t retry;
    int ret;

    struct rt_i2c_msg msgs[] = {
        {
            .addr = 0x28,
            .flags = RT_I2C_WR,
            .len = tx_len + 1,
            .buf = tx_buf,
        },
        {
            .addr = 0x28,
            .flags = RT_I2C_RD,
            .len = rx_len,
            .buf = rx_buf,
        },
    };

    tx_buf[0] = 0xA0 | (tx_len - 1);

    for (retry = 1; retry <= I2C_RETRY_COUNT; retry++)
    {
        if (rt_i2c_transfer(i2c_data->client, msgs, 2) == 2)
        {
            ret = rx_len;
            break;
        }
        rt_kprintf("I2C retry %d\n", retry);
        rt_thread_mdelay(20);
    }

    if (retry > I2C_RETRY_COUNT)
    {
        rt_kprintf("%s: I2C read over retry limit\n", __func__);
        ret = -RT_EIO;
    }

    return ret;
}

static int sitronix_ts_i2c_awrite(
    uint8_t *tx_buf, uint16_t tx_len, void *if_data)
{
    struct sitronix_ts_i2c_data *i2c_data =
        (struct sitronix_ts_i2c_data *)if_data;
    uint8_t retry;
    int ret;

    struct rt_i2c_msg msg[] = { {
        .addr = 0x28,
        .flags = RT_I2C_WR,
        .len = tx_len + 1,
        .buf = tx_buf,
    } };

    tx_buf[0] = 0x00;

    for (retry = 1; retry <= I2C_RETRY_COUNT; retry++)
    {
        if (rt_i2c_transfer(i2c_data->client, msg, 1) == 1)
        {
            ret = tx_len;
            break;
        }
        rt_kprintf("I2C retry %d\n", retry);
        rt_thread_mdelay(20);
    }

    if (retry > I2C_RETRY_COUNT)
    {
        rt_kprintf("%s: I2C write over retry limit\n", __func__);
        ret = -RT_EIO;
    }

    return ret;
}

static struct sitronix_ts_i2c_data g_i2c_data;

static struct sitronix_ts_host_interface i2c_host_if = { .if_data = &g_i2c_data,
    .read = sitronix_ts_i2c_read,
    .write = sitronix_ts_i2c_write,
    .aread = sitronix_ts_i2c_aread,
    .awrite = sitronix_ts_i2c_awrite };

static struct rt_platform_device sitronix_ts_i2c_device = {
    .name = "sitronix_ts",
    .id = RT_NULL,
};

static rt_err_t sitronix_ts_i2c_probe(struct rt_platform_device *pdev)
{
    struct rt_ofw_node *np = pdev->parent.ofw_node;
    char bus_name[RT_NAME_MAX];
    struct rt_ofw_node *port;
    rt_err_t ret;
    int id;

    /* find i2c bus */
    ret = rt_ofw_prop_read_u32(np, "reg", &(g_i2c_data.client_addr));
    if (ret != RT_EOK)
    {
        rt_kprintf("tca9534a addr not found\n");
        ret = -RT_ERROR;
        goto reg_err;
    }

    port = rt_ofw_get_parent(np);
    if (!port)
    {
        ret = -RT_ERROR;
        goto port_err;
    }

    id = rt_ofw_get_alias_id(port, "i2c");
    rt_snprintf(bus_name, RT_NAME_MAX, "i2c%d", id);

    g_i2c_data.client = rt_i2c_bus_device_find(bus_name);
    if (!g_i2c_data.client)
    {
        ret = -RT_ERROR;
        goto i2c_bus_err;
    }

    if (rt_device_open(&(g_i2c_data.client->parent), RT_DEVICE_FLAG_RDWR) !=
        RT_EOK)
    {
        rt_kprintf("open %s device failed", bus_name);
        goto i2c_open_err;
    }

    if (np)
    {
        ret = sitronix_ts_i2c_parse_dt(np, &i2c_host_if);
        if (ret != RT_EOK)
        {
            goto dt_parse_err;
        }
    }

    sitronix_ts_i2c_device.priv = &i2c_host_if;

    ret = rt_platform_device_register(&sitronix_ts_i2c_device);
    if (ret != RT_EOK)
    {
        goto dev_reg_err;
    }

    return RT_EOK;

dev_reg_err:
dt_parse_err:
    rt_device_close(&(g_i2c_data.client->parent));
i2c_open_err:
i2c_bus_err:
port_err:
reg_err:
    return ret;
}

static const struct rt_ofw_node_id i2c_match_table[] = {
    { .compatible = "sitronix_ts_i2c" }, { /* sentinel */ }
};

static struct rt_platform_driver sitronix_ts_i2c_driver = {
    .name = "sitronix_ts",
    .ids = i2c_match_table,
    .probe = sitronix_ts_i2c_probe,
};

static int sitronix_ts_i2c_register(void)
{
    return rt_platform_driver_register(&sitronix_ts_i2c_driver);
}
INIT_DEVICE_EXPORT(sitronix_ts_i2c_register);
