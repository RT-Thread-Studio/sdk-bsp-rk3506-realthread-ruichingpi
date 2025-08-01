/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#if RT_VER_NUM >= 0x40100
#include <fcntl.h> /* 当需要使用文件操作时，需要包含这个头文件 */
#else
#include <dfs_posix.h>
#endif /*RT_VER_NUM >= 0x40100*/

#include <webnet.h>
#include <wn_module.h>

#ifdef WEBNET_USING_UPLOAD

/**
 * upload file.
 */
static const char * sd_upload = RT_NULL;

static const char * upload_dir = "upload"; /* e.g: "upload" */
static int file_size = 0;

const char *get_file_name(struct webnet_session *session)
{
    const char *path = RT_NULL, *path_last = RT_NULL;

    path_last = webnet_upload_get_filename(session);
    if (path_last == RT_NULL)
    {
        rt_kprintf("file name err!!\n");
        return RT_NULL;
    }

    path = strrchr(path_last, '\\');
    if (path != RT_NULL)
    {
        path++;
        path_last = path;
    }

    path = strrchr(path_last, '/');
    if (path != RT_NULL)
    {
        path++;
        path_last = path;
    }

    return path_last;
}

static int upload_open(struct webnet_session *session)
{
    int fd = -1;
    const char *file_name = RT_NULL;

    sd_upload = webnet_get_root();

    file_name = get_file_name(session);

    if (file_name == RT_NULL)
    {
        rt_kprintf("file name is NULL\n");
        return -1;
    }

    rt_kprintf("Upload FileName: %s\n", file_name);
    rt_kprintf("Content-Type   : %s\n", webnet_upload_get_content_type(session));

    if (webnet_upload_get_filename(session) != RT_NULL)
    {
        int path_size;
        char * file_path;

        path_size = strlen(sd_upload) + strlen(upload_dir)
                    + strlen(file_name);

        path_size += 4;
        file_path = (char *)rt_malloc(path_size);

        if(file_path == RT_NULL)
        {
            fd = -1;
            goto _exit;
        }

        rt_sprintf(file_path, "%s/%s/%s", sd_upload, upload_dir, file_name);

        rt_kprintf("save to: %s\r\n", file_path);

        fd = open(file_path, O_WRONLY | O_CREAT, 0);
        if (fd < 0)
        {
            rt_kprintf("open file fail\n");
            webnet_session_close(session);
            rt_free(file_path);

            fd = -1;
            goto _exit;
        }
    }

    file_size = 0;

_exit:
    return (int)fd;
}

static int upload_close(struct webnet_session* session)
{
    int fd;

    fd = (int)webnet_upload_get_userdata(session);
    if (fd < 0) return 0;

    close(fd);
    rt_kprintf("Upload FileSize: %d\n", file_size);
    return 0;
}

static int upload_write(struct webnet_session* session, const void* data, rt_size_t length)
{
    int fd;

    fd = (int)webnet_upload_get_userdata(session);
    if (fd < 0) return 0;

    rt_kprintf("write: length %d\n", length);

    write(fd, data, length);
    file_size += length;

    return length;
}

static int upload_done (struct webnet_session* session)
{
    const char* mimetype;
    static const char* status = "<html><head><title>Upload OK </title>"
                                "</head><body>Upload OK, file length = %d "
                                "<br/><br/><a href=\"javascript:history.go(-1);\">"
                                "Go back to root</a></body></html>\r\n";

    /* get mimetype */
    mimetype = mime_get_type(".html");

    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", rt_strlen(status));
    webnet_session_printf(session, status, file_size);

    return 0;
}

const struct webnet_module_upload_entry upload_entry_upload =
{
    "/upload",
    upload_open,
    upload_close,
    upload_write,
    upload_done
};

#endif /* WEBNET_USING_UPLOAD */
