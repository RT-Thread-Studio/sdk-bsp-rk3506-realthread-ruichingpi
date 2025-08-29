/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <mdio.h>

static struct rt_list_node mdio_list = RT_LIST_OBJECT_INIT(mdio_list);

rt_err_t rt_mdio_register(struct mii_bus *bus)
{
    if (!bus)
    {
        return (-RT_ERROR);
    }

    if (rt_mdio_get_bus_by_name(bus->name))
    {
        rt_kprintf("mdio bus %s already exist!\n", bus->name);
        return (-RT_ERROR);
    }

    rt_list_insert_before(&mdio_list, &bus->node);
    return RT_EOK;
}

rt_err_t rt_mdio_unregister(struct mii_bus *bus)
{
    if (!bus)
    {
        return (-RT_ERROR);
    }

    rt_list_remove(&bus->node);

    rt_free(bus);
    bus = RT_NULL;

    return RT_EOK;
}

struct mii_bus *rt_mdio_get_bus_by_name(const char *busname)
{
    struct rt_list_node *entry;
    struct mii_bus *bus;

    if (!busname)
    {
        rt_kprintf("NULL bus name!\n");
        return RT_NULL;
    }

    rt_list_for_each(entry, &mdio_list)
    {
        bus = rt_container_of(entry, struct mii_bus, node);
        if (!rt_strcmp(bus->name, busname))
        {
            return bus;
        }
    }

    return RT_NULL;
}

struct mii_bus *rt_mdio_alloc(void)
{
    struct mii_bus *mii;
    mii = rt_calloc(1, sizeof(struct mii_bus));
    if (!mii)
    {
        return RT_NULL;
    }

    rt_list_init(&mii->node);
    return mii;
}

int rt_mdio_read(struct mii_bus *mii, rt_uint8_t addr, rt_uint8_t reg)
{
    int ret;

    if (mii == RT_NULL)
    {
        return -RT_EINVAL;
    }

    if (mii->read != RT_NULL)
    {
        ret = mii->read(mii, addr, reg);
        if (ret >= 0)
        {
            return RT_EOK;
        }
        return ret;
    }

    return (-RT_ENOSYS);
}

rt_err_t rt_mdio_write(
    struct mii_bus *mii, rt_uint8_t addr, rt_uint8_t reg, rt_uint16_t val)
{
    int ret;

    if (mii == RT_NULL)
    {
        return (-RT_EINVAL);
    }

    if (mii->write != RT_NULL)
    {
        ret = mii->write(mii, addr, reg, val);
        if (ret == 0)
        {
            return RT_EOK;
        }
        return ret;
    }

    return (-RT_ENOSYS);
}

rt_err_t rt_mdio_reset(struct mii_bus *mii)
{
    int ret;
    rt_uint32_t delay_ticks;

    if (mii == RT_NULL)
    {
        return -RT_EINVAL;
    }

    if (mii->reset == RT_NULL)
    {
        return -RT_ENOSYS;
    }

    ret = mii->reset(mii);
    if (ret != 0)
    {
        return ret;
    }

    if (mii->reset_delay_us > 0)
    {
        delay_ticks = mii->reset_delay_us * RT_TICK_PER_SECOND / 1000000;
        if (delay_ticks == 0)
        {
            delay_ticks = 1;
        }

        rt_thread_delay(delay_ticks);
    }

    if (mii->reset_post_delay_us > 0)
    {
        delay_ticks = mii->reset_post_delay_us * RT_TICK_PER_SECOND / 1000000;
        if (delay_ticks == 0)
        {
            delay_ticks = 1;
        }

        rt_thread_delay(delay_ticks);
    }

    return RT_EOK;
}
