/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <perfetto-server.h>
#include <service.h>
#include <rtthread.h>
#include <webnet.h>
#include <wn_module.h>

static void cgi_perfetto(struct webnet_session* session)
{
    if (!session || !session->request)
        goto __exit;
    const char *action = NULL;

    for (int i = 0; i < session->request->query_counter; i++)
    {
        if (strcmp(session->request->query_items[i].name, "action") == 0)
        {
            action = session->request->query_items[i].value;
            break;
        }
    }

    if (!action)
    {
        webnet_session_printf(session,
                            "{\"status\":\"ok\",\"data\":[\"perfetto/perfetto.json\"] }");
        goto __exit;
    }

    if (action)
    {

        if (strcmp(action, "start") == 0)
        {
            service_control(service_find("perfetto_service"), APP_PERFETTO_START, RT_NULL);

            webnet_session_printf(session, "{\"status\":\"ok\",\"msg\":\"trace started\"}");
        }
        else if (strcmp(action, "stop") == 0)
        {
            service_control(service_find("perfetto_service"), APP_PERFETTO_STOP, RT_NULL);

            webnet_session_printf(session, "{\"status\":\"ok\",\"msg\":\"trace stop\"}");
        }
        else
        {
            webnet_session_printf(session, "{\"status\":\"ok\",\"msg\":\"unknown action\"}");
        }
    }

__exit:

    return;
}

static void start_perfetto_server(void)
{
    struct service_core *svc = service_find("perfetto_service");
    if (!svc)
    {
        rt_kprintf("perfetto_service not found!\n");
        return;
    }

    webnet_cgi_register("trace", cgi_perfetto);

    if (service_control(svc, APP_PERFETTO_SET_PATH, "/sdmmc/webnet_trace/perfetto") != RT_EOK)
    {
        rt_kprintf("perfetto_service failed\n");
        return;
    }

    webnet_start(9001, "/sdmmc/webnet_trace");

}
MSH_CMD_EXPORT(start_perfetto_server, start perfetto server);
