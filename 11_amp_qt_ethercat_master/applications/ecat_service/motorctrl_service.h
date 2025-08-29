/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __MOTORCTRL_SERVICE_H
#define __MOTORCTRL_SERVICE_H

typedef struct
{
    int      slave;
    uint8_t  run;
    uint8_t  dir;
    uint8_t  mode;
    uint8_t  rev;
	uint32_t ctrl_value;
	int32_t  dest_pos;
	int32_t  cur_pos;
    int32_t  rpm;
} motorctrl_data_t;


#endif /* __MOTORCTRL_SERVICE_H */
