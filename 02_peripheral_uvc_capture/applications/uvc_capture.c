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
#include <rtdevice.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define RT_UVC_CTRL_SET_CALLBACK          0x0             /**< set callback control command */
#define RT_UVC_CTRL_START_STREAM          0x1             /**< set start stream control command */
#define RT_UVC_CTRL_STOP_STREAM           0x2             /**< set stop stream control command */

struct usbh_videoframe {
     uint8_t *frame_buf; 
     uint32_t frame_bufsize; 
     uint32_t frame_format; 
     uint32_t frame_size; 
};

typedef void (*frame_callback_t)(struct usbh_videoframe *frame);
char file_path[50];
static struct rt_semaphore sem_lock;
bool flag = false;
//  定义回调函数
frame_callback_t my_function(struct usbh_videoframe *frame) {
     int fd = -1;
     flag = true;
     if (frame->frame_format == 0 && flag) {
         flag = false;
         //yuv
         char *file = "output.yuv";
         strncat( file_path, file, sizeof(file_path) - strlen(file_path) -1 );
         fd = open(file_path, O_RDWR | O_CREAT);
         rt_kprintf("YUV data saved to %s\r\n",file_path);
    } else if (frame->frame_format == 1 && flag){
        flag = false;
          //mjpeg
         char* file = "output.jpg";
         strncat( file_path, file, sizeof(file_path) - strlen(file_path) -1 );
         fd = open(file_path, O_RDWR | O_CREAT);
         rt_kprintf("MJPEG data saved to %s\r\n",file_path);
    }
    if (!fd){
         rt_kprintf("failed to open file %s\r\n",file_path);
         return RT_NULL;
    }
    rt_sem_release(&sem_lock);
    rt_kprintf("frame buf:%p,frame len:%d\r\n", frame->frame_buf, frame->frame_size);
    write(fd, frame->frame_buf, frame->frame_size);
    close(fd);

}

static int uvc_capture(int argc, char *argv[])
{
    uint8_t type;
    struct rt_device *device = rt_device_find("uvc");
    if (!device)
    {
        rt_kprintf("Can't find device uvc\n");
        return (-RT_ERROR);
    }

    if (argc < 3) {
         rt_kprintf("please input correct command: usbh_uvc_start type to file_path\r\n");
         rt_kprintf("type 0:yuyv, type 1:mjpeg\r\n");
         return (-RT_ERROR);
    }

    type = atoi(argv[1]);
    if (type == 0) {
         rt_kprintf("uvc capture yuyv type picture\r\n");
    } else if(type == 1) {
         rt_kprintf("uvc capture mjpeg type picture\r\n");
    } else {
	 rt_kprintf("uvc capture type is unsupport!\r\n");
	 return (-RT_ERROR);
    }
  
    if (access(argv[2], F_OK) == 0) {  // F_OK检查文件是否存在
         rt_kprintf("file path exist\r\n");
    } else {
         rt_kprintf("file path does not exist\r\n");
         return (-RT_ERROR); // 路径无效
    }

    snprintf(file_path, sizeof(file_path), "%s", argv[2]);

    frame_callback_t my_callback = my_function; // 用户定义的函数
    rt_sem_init(&sem_lock, "lock", 0, RT_IPC_FLAG_PRIO);

    rt_device_init(device);
    rt_device_open(device, RT_DEVICE_FLAG_RDWR);
    
    rt_device_control(device, RT_UVC_CTRL_SET_CALLBACK, (void *)my_callback);
    rt_device_control(device, RT_UVC_CTRL_START_STREAM, &type);

    rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
    rt_device_control(device, RT_UVC_CTRL_STOP_STREAM, NULL);

    rt_sem_detach(&sem_lock);    
    return RT_EOK;
}

MSH_CMD_EXPORT(uvc_capture, uvc capture example);
