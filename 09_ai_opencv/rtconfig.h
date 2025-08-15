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

/* Industrial Bus */

#define COMP_USING_CANFESTIVAL
#define CANFESTIVAL_CAN_DEVICE_NAME "ff320000.can"
#define CANFESTIVAL_TIMER_DEVICE_NAME "timer0"
#define CANFESTIVAL_RECV_THREAD_PRIO 9
#define CANFESTIVAL_TIMER_THREAD_PRIO 10
#define COMP_USING_ETHERCAT
#define COMP_USING_MODBUS
/* end of Industrial Bus */

/* Net Apps */

#define COMP_USING_AGILE_FTP
#define COMP_USING_PAHOMQTT
#define _PAHOMQTT_PIPE_MODE
#define MQTT_THREAD_STACK_SIZE 4096
#define PAHOMQTT_SUBSCRIBE_HANDLERS 1
#define MQTT_DEBUG
#define COMP_USING_TELNET
#define COMP_USING_TFTP
#define NETUTILS_TFTP_PORT 69
#define COMP_USING_WEBCLIENT
#define WEBCLIENT_USING_FILE_DOWMLOAD
#define COMPONENTS_WEBCLIENT_USING_MBED_TLS
#define COMP_USING_WEBNET
#define WEBNET_PORT 80
#define WEBNET_CONN_MAX 16
#define WEBNET_ROOT "/sdmmc/webnet"

/* Select supported modules */

#define WEBNET_USING_AUTH
#define WEBNET_USING_CGI
#define WEBNET_USING_ASP
#define WEBNET_USING_SSI
#define WEBNET_USING_INDEX
#define WEBNET_USING_ALIAS
#define WEBNET_USING_UPLOAD
#define WEBNET_CACHE_LEVEL 0
#define WEBNET_USING_SSI_VIRTUAL_HANDLER
/* end of Select supported modules */
/* end of Net Apps */

/* Graphics */

#define COMP_USING_LVGL
#define RT_LVGL_THREAD_PRIO 20
#define RT_LVGL_THREAD_STACK_SIZE 8192
#define RT_LVGL_DISP_REFR_PERIOD 33
#define RT_LVGL_VER_NUM 0x090100
#define RT_LVGL_VER "v9.1.0"
#define LV_CONF_SKIP

/* Color Settings */

#define COLOR_DEPTH_24
#define LV_COLOR_DEPTH 24
/* end of Color Settings */
/* end of Graphics */

/* AI */

#define COMP_USING_OPENCV
/* end of AI */

/* Data Parsers */

#define COMP_USING_CJSON
/* end of Data Parsers */

/* Debug Tools */

#define COMP_USING_BACKTRACE
#define COMP_USING_COREDUMP
#define COREDUMP_STORAGE_RAM
#define COREDUMP_MAX_SIZE_KB 2048
#define COREDUMP_FILE_SAVE_PATH "/sdmmc/core.dump"
#define COMP_USING_IPERF2
#define IPERF_THREAD_STACK_SIZE 16384
#define COMP_USING_RT_PERF
#define RT_PERF_TIMER_FREQ 24000000
#define RT_PERF_TIMER_BITS 32
#define RT_PERF_ENABLE_IRQ_LATENCY
#define RT_PERF_USING_TIMER_NAME "timer3"
/* end of Debug Tools */

/* Language */

#define COMP_USING_LUA
/* end of Language */

/* Security */

#define COMP_USING_MBEDTLS

/* Select Root Certificate */

#define COMP_USING_MBEDTLS_USE_ALL_CERTS
#define COMP_USING_MBEDTLS_USER_CERTS
#define COMP_USING_MBEDTLS_THAWTE_ROOT_CA
#define COMP_USING_MBEDTLS_VERSIGN_PBULIC_ROOT_CA
#define COMP_USING_MBEDTLS_VERSIGN_UNIVERSAL_ROOT_CA
#define COMP_USING_MBEDTLS_GEOTRUST_ROOT_CA
#define COMP_USING_MBEDTLS_DIGICERT_ROOT_CA
#define COMP_USING_MBEDTLS_GODADDY_ROOT_CA
#define COMP_USING_MBEDTLS_COMODOR_ROOT_CA
#define COMP_USING_MBEDTLS_DST_ROOT_CA
#define COMP_USING_MBEDTLS_CLOBALSIGN_ROOT_CA
#define COMP_USING_MBEDTLS_ENTRUST_ROOT_CA
#define COMP_USING_MBEDTLS_AMAZON_ROOT_CA
#define COMP_USING_MBEDTLS_CERTUM_TRUSTED_NETWORK_ROOT_CA
/* end of Select Root Certificate */
#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_ECP_WINDOW_SIZE 6
#define MBEDTLS_SSL_MAX_CONTENT_LEN 16384
/* end of Security */
/* end of RuiChing Components Configure */
#define RT_USING_CPLUSPLUS

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

/* IoT Cloud */

/* end of IoT Cloud */
/* end of IoT - internet of things */

/* security packages */

/* end of security packages */

/* language packages */

/* JSON: JavaScript Object Notation, a lightweight data-interchange format */

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

/* HC32 DDL Drivers */

/* end of HC32 DDL Drivers */

/* NXP HAL & SDK Drivers */

/* end of NXP HAL & SDK Drivers */

/* NUVOTON Drivers */

/* end of NUVOTON Drivers */

/* GD32 Drivers */

/* end of GD32 Drivers */
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
