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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webnet.h>
#include <wn_module.h>
#include <fcntl.h>
#include <unistd.h>

#define DBG_TAG "webnet.video"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define RT_UVC_CTRL_SET_CALLBACK          0x0             /**< Set the callback control command */
#define RT_UVC_CTRL_START_STREAM          0x1             /**< Start the video stream control command */
#define RT_UVC_CTRL_STOP_STREAM           0x2             /**< Stop the video stream control command */

struct usbh_videoframe
{
    uint8_t *frame_buf;
    uint32_t frame_size;
};

static struct webnet_session* g_session = RT_NULL;
static struct rt_device *uvc_device = RT_NULL;
static rt_bool_t stream_running = RT_FALSE;

static const char* boundary = "frame";
static const char* mimetype = "multipart/x-mixed-replace; boundary=";

static void video_frame_callback(struct usbh_videoframe *frame)
{
    if (stream_running && g_session)
    {
        webnet_session_printf(g_session, "Content-Type: image/jpeg\r\n\r\n");
        webnet_session_write(g_session, frame->frame_buf, frame->frame_size);
        webnet_session_printf(g_session, "\r\n--%s\r\n", boundary);
    }
}

static void cgi_mjpeg_stream_handler(struct webnet_session* session)
{
    uint8_t type = 1;

    g_session = session;
    RT_ASSERT(g_session != NULL);

    /*Initialize the UVC device*/
    if (!uvc_device)
    {
        uvc_device = rt_device_find("uvc");
        if (!uvc_device)
        {
            LOG_E("uvc equipment cannot be found");
            return;
        }
        rt_device_init(uvc_device);
        rt_device_open(uvc_device, RT_DEVICE_FLAG_RDWR);
    }

    /*Set the response header*/
    char content_type[64];
    rt_snprintf(content_type, sizeof(content_type), "%s%s", mimetype, boundary);
    g_session->request->result_code = 200;
    webnet_session_set_header(g_session, content_type, 200, "OK", -1);
    webnet_session_printf(g_session, "--%s\r\n", boundary);

    /* Start the UVC stream */
    rt_device_control(uvc_device, RT_UVC_CTRL_SET_CALLBACK, (void *) video_frame_callback);
    rt_device_control(uvc_device, RT_UVC_CTRL_START_STREAM, &type);

    stream_running = RT_TRUE;
    LOG_I("The video stream has been started");

    while (stream_running)
    {
        char read_buf[32];
        int read_len;
        read_len = webnet_session_read(g_session, read_buf, sizeof(read_buf) - 1);
        if (read_len < 0)
        {
            LOG_I("The client connection has been disconnected（read_len = %d，errno = %d）", read_len, errno);
            stream_running = RT_FALSE;
        }
        rt_thread_mdelay(33);
    }

    g_session = RT_NULL;
    LOG_I("cgi_mjpeg_stream_handler exits");

}

/* WebNet initialization function */
void webnet_video_init(void)
{

#ifdef WEBNET_USING_CGI
    /* Register for video processing CGI */
    webnet_cgi_register("mjpeg_stream", cgi_mjpeg_stream_handler);
#endif

    webnet_init();

}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(webnet_video_init, initialize video stream server);
#endif
