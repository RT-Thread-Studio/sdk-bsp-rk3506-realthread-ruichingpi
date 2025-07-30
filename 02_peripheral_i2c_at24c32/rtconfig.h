/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __RTCONFIG_APP_H__
#define __RTCONFIG_APP_H__

/* Automatically generated file; DO NOT EDIT. */
#include <kconfig.h>

/* RuiChing Components Configure */

#define BSP_USING_CANFESTIVAL
#define BSP_CANFESTIVAL_CAN_DEVICE_NAME "ff320000.can"
#define BSP_CANFESTIVAL_TIMER_DEVICE_NAME "timer0"
#define BSP_CANFESTIVAL_RECV_THREAD_PRIO 9
#define BSP_CANFESTIVAL_TIMER_THREAD_PRIO 10
#define PKG_USING_AGILE_FTP
#define PKG_USING_AGILE_FTP_LATEST_VERSION
#define PKG_AGILE_FTP_VER_NUM 0x99999
#define COMPONENTS_ETHERCAT_ENABLE
#define COMPONENTS_FINSH_EXPORT_ENABLE
#define COMPONENTS_MODBUS_ENABLE
#define PKG_USING_RT_PERF
#define RT_PERF_TIMER_FREQ 24000000
#define RT_PERF_TIMER_BITS 32
#define RT_PERF_ENABLE_IRQ_LATENCY
#define RT_PERF_USING_TIMER_NAME "timer3"
#define PKG_USING_RT_PERF_V003
#define RT_USING_SERVICE
#define PKG_NETUTILS_TELNET
#define PKG_USING_WEBCLIENT
#define WEBCLIENT_USING_FILE_DOWMLOAD
#define WEBCLIENT_NOT_USE_TLS
#define PKG_USING_WEBCLIENT_V220
#define PKG_WEBCLIENT_VER_NUM 0x20200
/* end of RuiChing Components Configure */

/* RT-Thread online packages */

/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */

/* end of Marvell WiFi */

/* Wiced WiFi */

/* end of Wiced WiFi */

/* CYW43012 WiFi */

/* end of CYW43012 WiFi */

/* BL808 WiFi */

/* end of BL808 WiFi */

/* CYW43439 WiFi */

/* end of CYW43439 WiFi */
/* end of Wi-Fi */
#define PKG_USING_NETUTILS
#define PKG_NETUTILS_IPERF
#define IPERF_THREAD_STACK_SIZE 2048
#define PKG_USING_NETUTILS_LATEST_VERSION
#define PKG_NETUTILS_VER_NUM 0x99999

/* IoT Cloud */

/* end of IoT Cloud */
/* end of IoT - internet of things */

/* security packages */

/* end of security packages */

/* language packages */

/* JSON: JavaScript Object Notation, a lightweight data-interchange format */

#define PKG_USING_CJSON
#define PKG_USING_CJSON_V1717
/* end of JSON: JavaScript Object Notation, a lightweight data-interchange format */

/* XML: Extensible Markup Language */

/* end of XML: Extensible Markup Language */
/* end of language packages */

/* multimedia packages */

/* LVGL: powerful and easy-to-use embedded GUI library */

/* end of LVGL: powerful and easy-to-use embedded GUI library */

/* u8g2: a monochrome graphic library */

/* end of u8g2: a monochrome graphic library */
/* end of multimedia packages */

/* tools packages */

/* end of tools packages */

/* system packages */

/* enhanced kernel services */

/* end of enhanced kernel services */

/* acceleration: Assembly language or algorithmic acceleration packages */

/* end of acceleration: Assembly language or algorithmic acceleration packages */

/* CMSIS: ARM Cortex-M Microcontroller Software Interface Standard */

/* end of CMSIS: ARM Cortex-M Microcontroller Software Interface Standard */

/* Micrium: Micrium software products porting for RT-Thread */

/* end of Micrium: Micrium software products porting for RT-Thread */
/* end of system packages */

/* peripheral libraries and drivers */

/* HAL & SDK Drivers */

/* STM32 HAL & SDK Drivers */

/* end of STM32 HAL & SDK Drivers */

/* Infineon HAL Packages */

/* end of Infineon HAL Packages */

/* Kendryte SDK */

/* end of Kendryte SDK */

/* WCH HAL & SDK Drivers */

/* end of WCH HAL & SDK Drivers */

/* AT32 HAL & SDK Drivers */

/* end of AT32 HAL & SDK Drivers */
/* end of HAL & SDK Drivers */

/* sensors drivers */

/* end of sensors drivers */

/* touch drivers */

/* end of touch drivers */
/* end of peripheral libraries and drivers */

/* AI packages */

/* end of AI packages */

/* Signal Processing and Control Algorithm Packages */

/* end of Signal Processing and Control Algorithm Packages */

/* miscellaneous packages */

/* project laboratory */

/* end of project laboratory */

/* samples: kernel and components samples */

/* end of samples: kernel and components samples */

/* entertainment: terminal games and other interesting software packages */

/* end of entertainment: terminal games and other interesting software packages */
/* end of miscellaneous packages */

/* Arduino libraries */


/* Projects and Demos */

/* end of Projects and Demos */

/* Sensors */

/* end of Sensors */

/* Display */

/* end of Display */

/* Timing */

/* end of Timing */

/* Data Processing */

/* end of Data Processing */

/* Data Storage */

/* Communication */

/* end of Communication */

/* Device Control */

/* end of Device Control */

/* Other */

/* end of Other */

/* Signal IO */

/* end of Signal IO */

/* Uncategorized */

/* end of Arduino libraries */
/* end of RT-Thread online packages */

#endif /* __RTCONFIG_APP_H__ */ 
