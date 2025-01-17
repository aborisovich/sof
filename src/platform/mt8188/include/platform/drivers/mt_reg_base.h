/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright(c) 2022 MediaTek. All rights reserved.
 *
 * Author: Allen-KH Cheng <allen-kh.cheng@mediatek.com>
 *         Tinghan Shen <tinghan.shen@mediatek.com>
 */

#ifndef MT_REG_BASE_H
#define MT_REG_BASE_H

#define MTK_REG_TOPCKGEN_BASE		0x10000000
#define MTK_REG_TOPCKGEN_SIZE		0x1000
#define MTK_REG_APMIXDSYS_BASE		0x1000C000
#define MTK_REG_APMIXDSYS_SIZE		0x1000

#define MTK_DSP_REG_BASE		0x10b80000 /* DSP Register base */
#define MTK_DSP_CFGREG_BASE		0x10b80000
#define MTK_DSP_CFGREG_SIZE		0x1000
#define MTK_DSP_CKCTRL_BASE		0x10b81000
#define MTK_DSP_CKCTRL_SIZE		0x1000
#define MTK_DSP_OS_TIMER_BASE		0x10b83000
#define MTK_DSP_OS_TIMER_SIZE		0x1000
#define MTK_DSP_UART0_BASE		0x10b84000
#define MTK_DSP_UART0_SIZE		0x1000
#define MTK_DSP_TIMER_BASE		0x10b8E000
#define MTK_DSP_TIMER_SIZE		0x1000
#define MTK_DSP_BUS_BASE		0x10b8F000
#define MTK_DSP_BUS_SIZE		0x1000
#define MTK_DSP_AFE_BASE		0x11210000 /*?*/
#define MTK_DSP_AFE_SIZE		0xF000 /* register 8k, compacted sram 52k */

#define MTK_ADSP_CFGREG_SW_RSTN		(MTK_DSP_REG_BASE + 0x0000)
#define MTK_ADSP_HIFI_IO_CONFIG		(MTK_DSP_REG_BASE + 0x000c)
#define MTK_ADSP_IRQ_STATUS		(MTK_DSP_REG_BASE + 0x0010)
#define MTK_ADSP_SW_INT_SET		(MTK_DSP_REG_BASE + 0x0018)
#define MTK_ADSP_SW_INT_CLR		(MTK_DSP_REG_BASE + 0x001c)
#define MTK_ADSP_SW_INT_32A		(MTK_DSP_REG_BASE + 0x0020)
#define MTK_ADSP_IRQ_MASK		(MTK_DSP_REG_BASE + 0x0030)
#define MTK_ADSP_GENERAL_IRQ_SET	(MTK_DSP_REG_BASE + 0x0034)
#define MTK_ADSP_GENERAL_IRQ_CLR	(MTK_DSP_REG_BASE + 0x0038)
#define MTK_ADSP_DVFSRC_STATE		(MTK_DSP_REG_BASE + 0x003c)
#define MTK_ADSP_DVFSRC_REQ		(MTK_DSP_REG_BASE + 0x0040)
#define MTK_ADSP_DDREN_REQ_0		(MTK_DSP_REG_BASE + 0x0044)
#define MTK_ADSP_SPM_ACK		(MTK_DSP_REG_BASE + 0x004c)
#define MTK_ADSP_IRQ_EN			(MTK_DSP_REG_BASE + 0x0050)
#define MTK_ADSP_IRQ_POL_FIX		(MTK_DSP_REG_BASE + 0x0054)
#define MTK_ADSP_SPM_WAKEUPSRC_CORE0	(MTK_DSP_REG_BASE + 0x005c)
#define MTK_ADSP_SEMAPHORE		(MTK_DSP_REG_BASE + 0x0064)
#define MTK_ADSP_DBG_SEL		(MTK_DSP_REG_BASE + 0x0074)
#define MTK_ADSP_DBG_INFO		(MTK_DSP_REG_BASE + 0x0078)
#define MTK_ADSP_WDT_CON_C0		(MTK_DSP_REG_BASE + 0x007c)
#define MTK_ADSP_WDT_INIT_VALUE_C0	(MTK_DSP_REG_BASE + 0x0080)
#define MTK_ADSP_WDT_CNT_C0		(MTK_DSP_REG_BASE + 0x0084)
#define MTK_ADSP_WAKEUPSRC_MASK_C0	(MTK_DSP_REG_BASE + 0x00a0)
#define MTK_ADSP_WAKEUPSRC_IRQ_C0	(MTK_DSP_REG_BASE + 0x00a4)
#define MTK_ADSP2SPM_MBOX		(MTK_DSP_REG_BASE + 0x00bc)
#define MTK_SPM2ADSP_MBOX		(MTK_DSP_REG_BASE + 0x00c0)
#define MTK_GPR_RW_REG0			(MTK_DSP_REG_BASE + 0x0440)
#define MTK_GPR_RW_REG1			(MTK_DSP_REG_BASE + 0x0444)
#define MTK_GPR_RW_REG2			(MTK_DSP_REG_BASE + 0x0448)
#define MTK_GPR_RW_REG3			(MTK_DSP_REG_BASE + 0x044c)
#define MTK_GPR_RW_REG4			(MTK_DSP_REG_BASE + 0x0450)
#define MTK_GPR_RW_REG5			(MTK_DSP_REG_BASE + 0x0454)
#define MTK_GPR_RW_REG6			(MTK_DSP_REG_BASE + 0x0458)
#define MTK_GPR_RW_REG7			(MTK_DSP_REG_BASE + 0x045c)
#define MTK_GPR_RW_REG8			(MTK_DSP_REG_BASE + 0x0460)
#define MTK_GPR_RW_REG9			(MTK_DSP_REG_BASE + 0x0464)
#define MTK_GPR_RW_REG10		(MTK_DSP_REG_BASE + 0x0468)
#define MTK_GPR_RW_REG11		(MTK_DSP_REG_BASE + 0x046c)
#define MTK_GPR_RW_REG12		(MTK_DSP_REG_BASE + 0x0470)
#define MTK_GPR_RW_REG13		(MTK_DSP_REG_BASE + 0x0474)
#define MTK_GPR_RW_REG14		(MTK_DSP_REG_BASE + 0x0478)
#define MTK_GPR_RW_REG15		(MTK_DSP_REG_BASE + 0x047c)
#define MTK_GPR_RW_REG16		(MTK_DSP_REG_BASE + 0x0480)
#define MTK_GPR_RW_REG17		(MTK_DSP_REG_BASE + 0x0484)
#define MTK_GPR_RW_REG18		(MTK_DSP_REG_BASE + 0x0488)
#define MTK_GPR_RW_REG19		(MTK_DSP_REG_BASE + 0x048c)
#define MTK_GPR_RW_REG20		(MTK_DSP_REG_BASE + 0x0490)
#define MTK_GPR_RW_REG21		(MTK_DSP_REG_BASE + 0x0494)
#define MTK_GPR_RW_REG22		(MTK_DSP_REG_BASE + 0x0498)
#define MTK_GPR_RW_REG23		(MTK_DSP_REG_BASE + 0x049c)
#define MTK_GPR_RW_REG24		(MTK_DSP_REG_BASE + 0x04a0)
#define MTK_GPR_RW_REG25		(MTK_DSP_REG_BASE + 0x04a4)
#define MTK_GPR_RW_REG26		(MTK_DSP_REG_BASE + 0x04a8)
#define MTK_GPR_RW_REG27		(MTK_DSP_REG_BASE + 0x04ac)
#define MTK_GPR_RW_REG28		(MTK_DSP_REG_BASE + 0x04b0)  /* use for tickless status */
#define MTK_GPR_RW_REG29		(MTK_DSP_REG_BASE + 0x04b4)
#define MTK_ADSP_IRQ_OUT_MASK		(MTK_DSP_REG_BASE + 0x0500)
#define MTK_MBOX_IRQ_IN			(MTK_DSP_REG_BASE + 0xB070)

#define MTK_ADSPPLL_CON0		(MTK_REG_APMIXDSYS_BASE + 0x42C)
#define MTK_ADSPPLL_CON1		(MTK_REG_APMIXDSYS_BASE + 0x430)
#define MTK_ADSPPLL_CON2		(MTK_REG_APMIXDSYS_BASE + 0x434)
#define MTK_ADSPPLL_CON3		(MTK_REG_APMIXDSYS_BASE + 0x438)

#define MTK_CLK_CFG_UPDATE2		(MTK_REG_TOPCKGEN_BASE + 0xC)
#define MTK_CLK_CFG_17			(MTK_REG_TOPCKGEN_BASE + 0x0EC)
#define MTK_CLK_CFG_17_SET		(MTK_REG_TOPCKGEN_BASE + 0x0F0)
#define MTK_CLK_CFG_17_CLR		(MTK_REG_TOPCKGEN_BASE + 0x0F4)

/* MBOX registers */
#define MTK_ADSP_MBOX_REG_BASE(x)	(0x10b86000 + (0x1000 * (x)))
#define MTK_ADSP_MBOX_REG_SIZE		(0x5000)
#define MTK_ADSP_MBOX_IN_CMD(x)		(MTK_ADSP_MBOX_REG_BASE(x) + 0x100)
#define MTK_ADSP_MBOX_IN_CMD_CLR(x)	(MTK_ADSP_MBOX_REG_BASE(x) + 0x108)
#define MTK_ADSP_MBOX_OUT_CMD(x)	(MTK_ADSP_MBOX_REG_BASE(x) + 0x104)
#define MTK_ADSP_MBOX_OUT_CMD_CLR(x)	(MTK_ADSP_MBOX_REG_BASE(x) + 0x10c)

#endif /* MT_REG_BASE_H */
