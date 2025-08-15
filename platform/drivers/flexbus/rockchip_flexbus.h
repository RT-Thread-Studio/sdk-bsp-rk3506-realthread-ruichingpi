/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __ROCKCHIP_FLEXBUS_H__
#define __ROCKCHIP_FLEXBUS_H__

#include <rtthread.h>
#include <rtdevice.h>

#define ROCKCHIP_FLEXBUS0_OPMODE_NULL 0
#define ROCKCHIP_FLEXBUS0_OPMODE_DAC  1
#define ROCKCHIP_FLEXBUS0_OPMODE_SPI  2

#define ROCKCHIP_FLEXBUS1_OPMODE_NULL 0
#define ROCKCHIP_FLEXBUS1_OPMODE_ADC  1
#define ROCKCHIP_FLEXBUS1_OPMODE_CIF  2

#define FLEXBUS_ENR                   0x000
#define FLEXBUS_FREE_SCLK             0x004
#define FLEXBUS_CSN_CFG               0x008
#define FLEXBUS_COM_CTL               0x00C
#define FLEXBUS_REMAP                 0x010
#define FLEXBUS_STOP                  0x014
#define FLEXBUS_SLAVE_MODE            0x018
#define FLEXBUS_DVP_POL               0x01C
#define FLEXBUS_DVP_CROP_SIZE         0x020
#define FLEXBUS_DVP_CROP_START        0x024
#define FLEXBUS_DVP_ORDER             0x028
#define FLEXBUS_DVP_YUV2RGB           0x02C
#define FLEXBUS_TX_CTL                0x040
#define FLEXBUS_TX_NUM                0x044
#define FLEXBUS_TXWAT_START           0x048
#define FLEXBUS_TXFIFO_DNUM           0x04C
#define FLEXBUS_TX_WIDTH              0x050
#define FLEXBUS_TX_CSN_DUMMY          0x054
#define FLEXBUS_TX_CMD_LEN            0x058
#define FLEXBUS_TX_CMD0               0x05C
#define FLEXBUS_TX_CMD1               0x060
#define FLEXBUS_RX_CTL                0x080
#define FLEXBUS_RX_NUM                0x084
#define FLEXBUS_RXFIFO_DNUM           0x088
#define FLEXBUS_DLL_EN                0x08C
#define FLEXBUS_DLL_NUM               0x090
#define FLEXBUS_RXCLK_DUMMY           0x094
#define FLEXBUS_RXCLK_CAP_CNT         0x098
#define FLEXBUS_DMA_RD_OUTSTD         0x100
#define FLEXBUS_DMA_WR_OUTSTD         0x104
#define FLEXBUS_DMA_SRC_ADDR0         0x108
#define FLEXBUS_DMA_DST_ADDR0         0x10C
#define FLEXBUS_DMA_SRC_ADDR1         0x110
#define FLEXBUS_DMA_DST_ADDR1         0x114
#define FLEXBUS_DMA_SRC_LEN0          0x118
#define FLEXBUS_DMA_DST_LEN0          0x11C
#define FLEXBUS_DMA_SRC_LEN1          0x120
#define FLEXBUS_DMA_DST_LEN1          0x124
#define FLEXBUS_DMA_WAT_INT           0x128
#define FLEXBUS_DMA_TIMEOUT           0x12C
#define FLEXBUS_DMA_RD_LEN            0x130
#define FLEXBUS_STATUS                0x160
#define FLEXBUS_IMR                   0x164
#define FLEXBUS_RISR                  0x168
#define FLEXBUS_ISR                   0x16C
#define FLEXBUS_ICR                   0x170
#define FLEXBUS_REVISION              0x1F0

#define FLEXBUS_RX_ENR                (RT_BIT(16) | RT_BIT(17) | RT_BIT(1))
#define FLEXBUS_RX_DIS                RT_BIT(16 + 1)
#define FLEXBUS_TX_ENR                (RT_BIT(16) | RT_BIT(0))
#define FLEXBUS_TX_DIS                RT_BIT(16)

#define FLEXBUS_RX_FREE_MODE          (RT_BIT(16 + 1) | RT_BIT(1))
#define FLEXBUS_TX_FREE_MODE          (RT_BIT(16) | RT_BIT(0))

#define FLEXBUS_TX_AND_RX             0x0
#define FLEXBUS_TX_ONLY               0x1
#define FLEXBUS_RX_ONLY               0x2
#define FLEXBUS_TX_THEN_RX            0x3
#define FLEXBUS_SCLK_SHARE            RT_BIT(2)
#define FLEXBUS_TX_USE_RX             RT_BIT(3)

#define FLEXBUS_DVP_SEL               RT_BIT(1)
#define FLEXBUS_CLK1_IN               RT_BIT(0)

#define FLEXBUS_CONTINUE_MODE         RT_BIT(4)
#define FLEXBUS_CPOL                  RT_BIT(3)
#define FLEXBUS_CPHA                  RT_BIT(2)

#define FLEXBUS_TX_CTL_UNIT_BYTE      RT_BIT(14)
#define FLEXBUS_TX_CTL_MSB            RT_BIT(13)
#define FLEXBUS_TX_CTL_CPHA_SHIFT     2

#define FLEXBUS_RX_CTL_FILL_DUMMY     RT_BIT(17)
#define FLEXBUS_RX_CTL_UNIT_BYTE      RT_BIT(16)
#define FLEXBUS_RX_CTL_MSB            RT_BIT(15)
#define FLEXBUS_AUTOPAD               RT_BIT(14)
#define FLEXBUS_RXD_DY                RT_BIT(5)
#define FLEXBUS_RX_CTL_CPHA_SHIFT     2

#define FLEXBUS_SRC_WAT_LVL_MASK      0x3
#define FLEXBUS_SRC_WAT_LVL_SHIFT     2
#define FLEXBUS_DST_WAT_LVL_MASK      0x3
#define FLEXBUS_DST_WAT_LVL_SHIFT     0

#define FLEXBUS_DMA_TIMEOUT_ISR       RT_BIT(13)
#define FLEXBUS_DMA_ERR_ISR           RT_BIT(12)
#define FLEXBUS_DMA_DST1_ISR          RT_BIT(11)
#define FLEXBUS_DMA_DST0_ISR          RT_BIT(10)
#define FLEXBUS_DMA_SRC1_ISR          RT_BIT(9)
#define FLEXBUS_DMA_SRC0_ISR          RT_BIT(8)
#define FLEXBUS_DVP_FRAME_START_ISR   RT_BIT(7)
#define FLEXBUS_DVP_FRAME_AB_ISR      RT_BIT(6)
#define FLEXBUS_RX_DONE_ISR           RT_BIT(5)
#define FLEXBUS_RX_UDF_ISR            RT_BIT(4)
#define FLEXBUS_RX_OVF_ISR            RT_BIT(3)
#define FLEXBUS_TX_DONE_ISR           RT_BIT(2)
#define FLEXBUS_TX_UDF_ISR            RT_BIT(1)
#define FLEXBUS_TX_OVF_ISR            RT_BIT(0)

#define FLEXBUS1_DATA0                0xff660020
#define FLEXBUS1_DATA1                0xff660024
#define FLEXBUS1_DATA2                0xff660028
#define FLEXBUS1_DATA3                0xff66002c
#define FLEXBUS1_DATA4                0xff660030

#define RK3506_GRF_SOC_CON1           0x0004
#define FLEXBUS_ADC_REPEAT            0x40
#define FLEXBUS_MAX_RX_RATE           200000000
#define FLEXBUS_ADC_TIMEOUT           rt_tick_from_millisecond(100)
#define FLEXBUS_ADC_REPEAT            0x40
#define FLEXBUS_ADC_ERR_ISR                                                    \
    (FLEXBUS_DMA_TIMEOUT_ISR | FLEXBUS_DMA_ERR_ISR | FLEXBUS_RX_UDF_ISR |      \
        FLEXBUS_RX_OVF_ISR)
#define FLEXBUS_ADC_ISR    (FLEXBUS_ADC_ERR_ISR | FLEXBUS_RX_DONE_ISR)
#define DIV_ROUND_UP(x, y) (((x) + ((y) - 1)) / (y))

struct rockchip_flexbus_dfs_reg
{
    rt_uint32_t dfs_1bit;
    rt_uint32_t dfs_2bit;
    rt_uint32_t dfs_4bit;
    rt_uint32_t dfs_8bit;
    rt_uint32_t dfs_16bit;
    rt_uint32_t dfs_mask;
};

struct rockchip_flexbus
{
    void *base;
    char *name;
    struct rt_syscon *regmap_grf;
    rt_uint32_t irq;
    rt_uint32_t opmode0;
    rt_uint32_t opmode1;
    void *fb0_data;
    void *fb1_data;
    struct rockchip_flexbus_dfs_reg *dfs_reg;
    const struct rockchip_flexbus_config *config;
    struct rt_clk *tx_clk_flexbus;
    struct rt_clk *rx_clk_flexbus;
    struct rt_clk *aclk_flexbus;
    struct rt_clk *hclk_flexbus;
    void (*fb0_isr)(struct rockchip_flexbus *rkfb, rt_uint32_t isr);
    void (*fb1_isr)(struct rockchip_flexbus *rkfb, rt_uint32_t isr);
};
typedef struct rockchip_flexbus *rockchip_flexbus_t;

static inline void rockchip_flexbus_set_fb0(struct rockchip_flexbus *rkfb,
    void *fb0_data,
    void (*fb0_isr)(struct rockchip_flexbus *rkfb, rt_uint32_t isr))
{
    rkfb->fb0_data = fb0_data;
    rkfb->fb0_isr = fb0_isr;
}

static inline void rockchip_flexbus_set_fb1(struct rockchip_flexbus *rkfb,
    void *fb1_data,
    void (*fb1_isr)(struct rockchip_flexbus *rkfb, rt_uint32_t isr))
{
    rkfb->fb1_data = fb1_data;
    rkfb->fb1_isr = fb1_isr;
}

void rk3506_flexbus_grf_config(
    struct rockchip_flexbus *rkfb, bool slave_mode, bool cpol, bool cpha);
rt_uint32_t rockchip_flexbus_readl(
    struct rockchip_flexbus *rkfb, rt_uint32_t reg);
void rockchip_flexbus_writel(
    struct rockchip_flexbus *rkfb, rt_uint32_t reg, rt_uint32_t val);
void rockchip_flexbus_clrbits(
    struct rockchip_flexbus *rkfb, rt_uint32_t reg, rt_uint32_t clr_val);
void rockchip_flexbus_setbits(
    struct rockchip_flexbus *rkfb, rt_uint32_t reg, rt_uint32_t set_val);
void rockchip_flexbus_clrsetbits(struct rockchip_flexbus *rkfb,
    rt_uint32_t reg,
    rt_uint32_t clr_val,
    rt_uint32_t set_val);

#endif /* __ROCKCHIP_FLEXBUS_H__ */
