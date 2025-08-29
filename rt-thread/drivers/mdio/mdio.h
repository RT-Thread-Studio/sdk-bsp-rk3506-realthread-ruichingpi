/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __MDIO_H__
#define __MDIO_H__

#include <rtthread.h>

struct mii_bus
{
    struct rt_list_node node;
    char name[RT_NAME_MAX];
    void *priv;

    int (*read)(struct mii_bus *bus, int addr, int reg);
    int (*write)(struct mii_bus *bus, int addr, int reg, rt_uint16_t val);
    int (*reset)(struct mii_bus *bus);

    int reset_delay_us;
    int reset_post_delay_us;
};

rt_err_t rt_mdio_register(struct mii_bus *bus);
rt_err_t rt_mdio_unregister(struct mii_bus *bus);

struct mii_bus *rt_mdio_get_bus_by_name(const char *busname);
struct mii_bus *rt_mdio_alloc(void);

int rt_mdio_read(struct mii_bus *mii, rt_uint8_t addr, rt_uint8_t reg);
rt_err_t rt_mdio_write(
    struct mii_bus *mii, rt_uint8_t addr, rt_uint8_t reg, rt_uint16_t val);
rt_err_t rt_mdio_reset(struct mii_bus *mii);    

#endif /* __MDIO_H__ */
