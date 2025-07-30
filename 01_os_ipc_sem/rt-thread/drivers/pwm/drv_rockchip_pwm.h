/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DRV_ROCKCHIP_PWM_H__
#define __DRV_ROCKCHIP_PWM_H__

#define PWM_VERSION_ID                         0x0000
#define PWM_ENABLE                             0x0004
#define PWM_CLK_CTRL                           0x0008
#define PWM_CTRL                               0x000C
#define PWM_PERIOD                             0x0010
#define PWM_DUTY                               0x0014
#define PWM_OFFSET                             0x0018
#define PWM_RPT                                0x001C
#define PWM_FILTER_CTRL                        0x0020
#define PWM_CNT                                0x0024
#define PWM_ENABLE_DELAY                       0x0028
#define PWM_HPC                                0x002C
#define PWM_LPC                                0x0030
#define PWM_BIPHASIC_COUNTER_CTRL0             0x0040
#define PWM_BIPHASIC_COUNTER_CTRL1             0x0044
#define PWM_BIPHASIC_COUNTER_TIMER_VALUE       0x0048
#define PWM_BIPHASIC_COUNTER_RESULT_VALUE      0x004C
#define PWM_BIPHASIC_COUNTER_RESULT_VALUE_SYNC 0x0050
#define PWM_INTSTS                             0x0070
#define PWM_INT_EN                             0x0074
#define PWM_INT_MASK                           0x0078
#define PWM_WAVE_MEM_ARBITER                   0x0080
#define PWM_WAVE_MEM_STATUS                    0x0084
#define PWM_WAVE_CTRL                          0x0088
#define PWM_WAVE_MAX                           0x008C
#define PWM_WAVE_MIN                           0x0090
#define PWM_WAVE_OFFSET                        0x0094
#define PWM_WAVE_MIDDLE                        0x0098
#define PWM_WAVE_HOLD                          0x009C
#define PWM_GLOBAL_ARBITER                     0x00C0
#define PWM_GLOBAL_CTRL                        0x00C4
#define PWM_PWRMATCH_ARBITER                   0x0100
#define PWM_PWRMATCH_CTRL                      0x010C
#define PWM_PWRMATCH_LPRE                      0x0108
#define PWM_PWRMATCH_HPRE                      0x010C
#define PWM_PWRMATCH_LD                        0x0110
#define PWM_PWRMATCH_HD_ZERO                   0x0114
#define PWM_PWRMATCH_HD_ONE                    0x0118
#define PWM_PWRMATCH_VALUE0                    0x011C
#define PWM_PWRMATCH_VALUE1                    0x0120
#define PWM_PWRMATCH_VALUE2                    0x0124
#define PWM_PWRMATCH_VALUE3                    0x0128
#define PWM_PWRMATCH_VALUE4                    0x012C
#define PWM_PWRMATCH_VALUE5                    0x0130
#define PWM_PWRMATCH_VALUE6                    0x0134
#define PWM_PWRMATCH_VALUE7                    0x0138
#define PWM_PWRMATCH_VALUE8                    0x013C
#define PWM_PWRMATCH_VALUE9                    0x0140
#define PWM_PWRMATCH_VALUE10                   0x0144
#define PWM_PWRMATCH_VALUE11                   0x0148
#define PWM_PWRMATCH_VALUE12                   0x014C
#define PWM_PWRMATCH_VALUE13                   0x0150
#define PWM_PWRMATCH_VALUE14                   0x0154
#define PWM_PWRMATCH_VALUE15                   0x0158
#define PWM_PWRMATCH_VALUE                     0x015C
#define PWM_IR_TRANS_ARBITER                   0x0180
#define PWM_IR_TRANS_CTRL0                     0x0184
#define PWM_IR_TRANS_CTRL1                     0x0188
#define PWM_IR_TRANS_PRE                       0x018C
#define PWM_IR_TRANS_SPRE                      0x0190
#define PWM_IR_TRANS_LD                        0x0194
#define PWM_IR_TRANS_HD                        0x0198
#define PWM_IR_TRANS_BURST_FRAME               0x019C
#define PWM_IR_TRANS_DATA_VALUE                0x01A0
#define PWM_IR_TRANS_STATUS                    0x01A4
#define PWM_FREQ_ARBITER                       0x01C0
#define PWM_FREQ_CTRL                          0x01C4
#define PWM_FREQ_TIMER_VALUE                   0x01C8
#define PWM_FREQ_RESULT_VALUE                  0x01CC
#define PWM_COUNTER_ARBITER                    0x0200
#define PWM_COUNTER_CTRL                       0x0204
#define PWM_COUNTER_LOW                        0x0208
#define PWM_COUNTER_HIGH                       0x020C
#define PWM_WAVE_MEM                           0x0400

#endif /* __DRV_ROCKCHIP_PWM_H__ */
