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

#ifdef RT_USING_RTC
#include "drivers/dev_rtc.h"
#endif /* RT_USING_RTC */

#ifdef RT_USING_SPI
#include "drivers/dev_spi.h"
#endif /* RT_USING_SPI */

#ifdef RT_USING_SERIAL
#include "drivers/dev_serial.h"
#endif /* RT_USING_SERIAL */

#ifdef RT_USING_I2C
#include "drivers/dev_i2c.h"
#endif /* RT_USING_I2C */

#ifdef RT_USING_WDT
#include "drivers/dev_watchdog.h"
#endif /* RT_USING_WDT */

#include <gpio_dev.h>

#ifdef RT_USING_ADC
#include "drivers/adc.h"
#endif /* RT_USING_ADC */

#ifdef RT_USING_DAC
#include "drivers/dac.h"
#endif /* RT_USING_DAC */

#ifdef RT_USING_PWM
#include "drivers/dev_pwm.h"
#endif /* RT_USING_PWM */

#ifdef RT_USING_CLK
#include "drivers/clk.h"
#endif /* RT_USING_CLK */

#ifdef RT_USING_MTD_NAND
#include "drivers/mtd_nand.h"
#endif /* RT_USING_MTD_NAND */

#ifdef RT_USING_CAN
#include "drivers/can_v2.h"
#endif /* RT_USING_CAN */

#ifdef RT_USING_HWTIMER
#include "drivers/hwtimer.h"
#endif /* RT_USING_HWTIMER */

#ifdef RT_USING_TOUCH
#include "drivers/dev_touch.h"
#endif /* RT_USING_TOUCH */

#ifdef RT_MFD_SYSCON
#include "drivers/syscon.h"
#endif /* RT_MFD_SYSCON */

#endif /* __RT_DEVICE_H__ */
