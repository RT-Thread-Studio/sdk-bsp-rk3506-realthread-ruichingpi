/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-01-08     bernard      first version.
 * 2014-07-12     bernard      Add workqueue implementation.
 */

#ifndef __RT_DEVICE_H__
#define __RT_DEVICE_H__

#include <rtthread.h>
#include <drivers/classes/block.h>
#include <drivers/classes/char.h>
#include <drivers/classes/graphic.h>
#include <drivers/classes/mtd.h>
#include <drivers/classes/net.h>
#include <drivers/core/bus.h>
#include <drivers/core/driver.h>
#include <rtdef.h>
#include "ipc/completion.h"
#include "ipc/condvar.h"
#include "ipc/dataqueue.h"
#include "ipc/pipe.h"
#include "ipc/poll.h"
#include "ipc/ringblk_buf.h"
#include "ipc/ringbuffer.h"
#include "ipc/waitqueue.h"
#include "ipc/workqueue.h"

#include "drivers/ioctl.h"

#define RT_DEVICE(device) ((rt_device_t)device)

#ifdef RT_USING_DM
#include "drivers/core/dm.h"
#include "drivers/io.h"
#include "drivers/platform.h"

#ifdef RT_USING_OFW
#include "drivers/ofw.h"
#include "drivers/ofw_io.h"
#include "drivers/ofw_irq.h"
#endif /* RT_USING_OFW */
#endif /* RT_USING_DM */

#ifdef RT_USING_PIC
#include "drivers/pic.h"
#endif /* RT_USING_PIC */

#include "drivers/dev_rtc.h"

#include "drivers/dev_spi.h"

#include "drivers/dev_serial.h"

#include "drivers/dev_i2c.h"

#include "drivers/dev_watchdog.h"

#include <gpio_dev.h>

#include "drivers/adc.h"

#include "drivers/dac.h"

#include "drivers/dev_pwm.h"

#include "drivers/clk.h"

#include "drivers/mtd_nand.h"

#include "drivers/can_v2.h"

#include "drivers/hwtimer.h"

#include "drivers/dev_touch.h"

#include "drivers/syscon.h"

#endif /* __RT_DEVICE_H__ */
