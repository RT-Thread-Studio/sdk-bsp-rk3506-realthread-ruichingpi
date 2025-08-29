/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <app_upgrade.h>
#include <service.h>

static void app_upgrade(int argc, char **argv)
{
    if (argc != 3)
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("  app_upgrade app <app.img>\n");
        rt_kprintf("  app_upgrade dtb <dtb.dtb>\n");
        return;
    }

    struct service_core *svc = service_find("upgrade");
    if (!svc)
    {
        rt_kprintf("upgrade upgrade service not found!\n");
        return;
    }

    upgrade_request_t req;
    rt_memset(&req, 0, sizeof(req));

    if (!rt_strcmp(argv[1], "app"))
    {
        req.target = UPGRADE_TGT_APP;
        req.app_path = argv[2];
    }
    else if (!rt_strcmp(argv[1], "dtb"))
    {
        req.target = UPGRADE_TGT_DTB;
        req.dtb_path = argv[2];
    }
    else
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("  app_upgrade app <app.img>\n");
        rt_kprintf("  app_upgrade dtb <dtb.dtb>\n");
        return;
    }

    if (service_control(svc, UPGRADE_CTRL_EXECUTE, &req) != RT_EOK)
    {
        rt_kprintf("upgrade %s upgrade failed\n", argv[1]);
        return;
    }

    rt_kprintf("upgrade %s upgrade was successful. The system will reboot...\n",
        argv[1]);
}
MSH_CMD_EXPORT(app_upgrade, upgrade app / dtb);
