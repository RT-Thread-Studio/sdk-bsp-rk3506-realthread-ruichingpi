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

rt_err_t app_upgrade_boot_ok(void)
{
    struct service_core *svc = service_find("upgrade");

    if (!svc)
    {
        return -RT_ERROR;
    }

    return service_control(svc, UPGRADE_CTRL_CLEAR_APP_BOOT_FLAG, RT_NULL);
}

static int app_upgrade_boot_ok_init(void)
{
    return (int)app_upgrade_boot_ok();
}
INIT_APP_EXPORT(app_upgrade_boot_ok_init);

static void app_upgrade(int argc, char **argv)
{
    struct service_core *svc;
    int cmd = UPGRADE_CTRL_EXECUTE;
    rt_err_t ret;
    upgrade_request_t req;

    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("  app_upgrade app <app.img>\n");
        rt_kprintf("  app_upgrade app_back <app.img>\n");
        rt_kprintf("  app_upgrade dtb <dtb.dtb>\n");
        rt_kprintf("  app_upgrade boot_ok\n");
        return;
    }

    svc = service_find("upgrade");
    if (!svc)
    {
        rt_kprintf("upgrade upgrade service not found!\n");
        return;
    }

    rt_memset(&req, 0, sizeof(req));

    if (!rt_strcmp(argv[1], "boot_ok"))
    {
        if (argc != 2)
        {
            rt_kprintf("Usage:\n");
            rt_kprintf("  app_upgrade boot_ok\n");
            return;
        }

        cmd = UPGRADE_CTRL_CLEAR_APP_BOOT_FLAG;
    }
    else if (argc != 3)
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("  app_upgrade app <app.img>\n");
        rt_kprintf("  app_upgrade app_back <app.img>\n");
        rt_kprintf("  app_upgrade dtb <dtb.dtb>\n");
        rt_kprintf("  app_upgrade boot_ok\n");
        return;
    }
    else if (!rt_strcmp(argv[1], "app"))
    {
        req.target = UPGRADE_TGT_APP;
        req.app_path = argv[2];
    }
    else if (!rt_strcmp(argv[1], "app_back"))
    {
        cmd = UPGRADE_CTRL_UPDATE_APP_BACK;
        req.app_back_path = argv[2];
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
        rt_kprintf("  app_upgrade app_back <app.img>\n");
        rt_kprintf("  app_upgrade dtb <dtb.dtb>\n");
        rt_kprintf("  app_upgrade boot_ok\n");
        return;
    }

    if (cmd == UPGRADE_CTRL_CLEAR_APP_BOOT_FLAG)
    {
        ret = app_upgrade_boot_ok();
    }
    else
    {
        ret = service_control(svc, cmd, &req);
    }

    if (ret != RT_EOK)
    {
        rt_kprintf("upgrade %s failed\n", argv[1]);
        return;
    }

}
MSH_CMD_EXPORT(app_upgrade, upgrade app / app_back / dtb);
