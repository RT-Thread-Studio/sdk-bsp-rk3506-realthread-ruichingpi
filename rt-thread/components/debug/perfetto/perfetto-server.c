/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webnet.h>
#include <wn_module.h>
#include <fcntl.h>
#include <unistd.h>
#include <rtthread.h>
#include <rtdef.h>

static void cgi_perfetto(struct webnet_session* session)
{

    if (!session || !session->request)
        goto __exit;
    const char *action = NULL;

    /* 遍历 query_items */
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
            perfetto_start();
            webnet_session_printf(session, "{\"status\":\"ok\",\"msg\":\"trace started\"}");
        }
        else if (strcmp(action, "stop") == 0)
        {
            perfetto_stop();
            webnet_session_printf(session, "{\"status\":\"ok\",\"msg\":\"trace stop\"}");
        }
        else
        {
            webnet_session_printf(session, "{\"status\":\"error\",\"msg\":\"unknown action\"}");
        }
    }

__exit:

    return;
}

static int  start_perfetto_server(void)
{
#ifdef WEBNET_USING_CGI
    webnet_cgi_register("trace", cgi_perfetto);
#endif

    perfetto_set_path("/sdmmc/webnet/perfetto");
    webnet_init();

    return 0;
}
MSH_CMD_EXPORT(start_perfetto_server, start perfetto server);
