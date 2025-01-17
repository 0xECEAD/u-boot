// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <init.h>
#include <net.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <linux/sizes.h>
#include <linux/delay.h>
#include <common.h>
#include <fsl_esdhc_imx.h>
#include <i2c.h>
#include <mmc.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include "../common/pfuze.h"
#include <usb.h>
#include <usb/ehci-ci.h>
#if defined(CONFIG_MXC_EPDC)
#include <mxc_epdc_fb.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_22K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |             \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED   |             \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define I2C_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
		      PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |	\
		      PAD_CTL_DSE_40ohm | PAD_CTL_HYS |		\
		      PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define OTGID_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
			PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_LOW |\
			PAD_CTL_DSE_80ohm | PAD_CTL_HYS |	\
			PAD_CTL_SRE_FAST)

#define ELAN_INTR_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE | \
			    PAD_CTL_PUS_47K_UP | PAD_CTL_HYS)

#define EPDC_PAD_CTRL    (PAD_CTL_PKE | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define ETH_PHY_POWER	IMX_GPIO_NR(4, 21)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

phys_size_t get_effective_memsize(void)
{
	return SZ_512M;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_UART1_TXD__UART1_TXD | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART1_RXD__UART1_RXD | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#ifdef CONFIG_SPL_BUILD
static iomux_v3_cfg_t const usdhc1_pads[] = {
	/* 8 bit SD */
	MX6_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT0__USDHC1_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT1__USDHC1_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT2__USDHC1_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT3__USDHC1_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT4__USDHC1_DAT4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT5__USDHC1_DAT5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT6__USDHC1_DAT6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT7__USDHC1_DAT7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	/*CD pin*/
	MX6_PAD_KEY_ROW7__GPIO_4_7 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_SD2_CLK__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_CMD__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT0__USDHC2_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT1__USDHC2_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT2__USDHC2_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT3__USDHC2_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	/*CD pin*/
	MX6_PAD_SD2_DAT7__GPIO_5_0 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__USDHC3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__USDHC3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__USDHC3_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__USDHC3_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__USDHC3_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__USDHC3_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	/*CD pin*/
	MX6_PAD_REF_CLK_32K__GPIO_3_22 | MUX_PAD_CTRL(NO_PAD_CTRL),
};
#endif

static iomux_v3_cfg_t const elan_pads[] = {
	MX6_PAD_EPDC_PWRCTRL2__GPIO_2_9 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EPDC_PWRCTRL3__GPIO_2_10 | MUX_PAD_CTRL(ELAN_INTR_PAD_CTRL),
	MX6_PAD_KEY_COL6__GPIO_4_4 | MUX_PAD_CTRL(EPDC_PAD_CTRL),
};

#ifdef CONFIG_MXC_EPDC
static iomux_v3_cfg_t const epdc_enable_pads[] = {
	MX6_PAD_EPDC_D0__EPDC_SDDO_0	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D1__EPDC_SDDO_1	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D2__EPDC_SDDO_2	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D3__EPDC_SDDO_3	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D4__EPDC_SDDO_4	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D5__EPDC_SDDO_5	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D6__EPDC_SDDO_6	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_D7__EPDC_SDDO_7	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_GDCLK__EPDC_GDCLK	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_GDSP__EPDC_GDSP	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_GDOE__EPDC_GDOE	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_GDRL__EPDC_GDRL	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDCLK__EPDC_SDCLK	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDOE__EPDC_SDOE	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDLE__EPDC_SDLE	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDSHR__EPDC_SDSHR	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_BDR0__EPDC_BDR_0	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDCE0__EPDC_SDCE_0	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDCE1__EPDC_SDCE_1	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EPDC_SDCE2__EPDC_SDCE_2	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
};

static iomux_v3_cfg_t const epdc_disable_pads[] = {
	MX6_PAD_EPDC_D0__GPIO_1_7,
	MX6_PAD_EPDC_D1__GPIO_1_8,
	MX6_PAD_EPDC_D2__GPIO_1_9,
	MX6_PAD_EPDC_D3__GPIO_1_10,
	MX6_PAD_EPDC_D4__GPIO_1_11,
	MX6_PAD_EPDC_D5__GPIO_1_12,
	MX6_PAD_EPDC_D6__GPIO_1_13,
	MX6_PAD_EPDC_D7__GPIO_1_14,
	MX6_PAD_EPDC_GDCLK__GPIO_1_31,
	MX6_PAD_EPDC_GDSP__GPIO_2_2,
	MX6_PAD_EPDC_GDOE__GPIO_2_0,
	MX6_PAD_EPDC_GDRL__GPIO_2_1,
	MX6_PAD_EPDC_SDCLK__GPIO_1_23,
	MX6_PAD_EPDC_SDOE__GPIO_1_25,
	MX6_PAD_EPDC_SDLE__GPIO_1_24,
	MX6_PAD_EPDC_SDSHR__GPIO_1_26,
	MX6_PAD_EPDC_BDR0__GPIO_2_5,
	MX6_PAD_EPDC_SDCE0__GPIO_1_27,
	MX6_PAD_EPDC_SDCE1__GPIO_1_28,
	MX6_PAD_EPDC_SDCE2__GPIO_1_29,
};
#endif

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

#ifdef CONFIG_SYS_I2C_LEGACY
#define PC	MUX_PAD_CTRL(I2C_PAD_CTRL)
/* I2C1 for PMIC */
struct i2c_pads_info i2c_pad_info1 = {
	.sda = {
		.i2c_mode = MX6_PAD_I2C1_SDA__I2C1_SDA | PC,
		.gpio_mode = MX6_PAD_I2C1_SDA__GPIO_3_13 | PC,
		.gp = IMX_GPIO_NR(3, 13),
	},
	.scl = {
		.i2c_mode = MX6_PAD_I2C1_SCL__I2C1_SCL | PC,
		.gpio_mode = MX6_PAD_I2C1_SCL__GPIO_3_12 | PC,
		.gp = IMX_GPIO_NR(3, 12),
	},
};
#endif

#ifdef CONFIG_POWER_LEGACY
int power_init_board(void)
{
	struct pmic *pfuze;
	unsigned int reg;
	int ret;

	pfuze = pfuze_common_init(I2C_PMIC);
	if (!pfuze)
		return -ENODEV;

	ret = pfuze_mode_init(pfuze, APS_PFM);
	if (ret < 0)
		return ret;

	/* set SW1AB staby volatage 0.975V */
	pmic_reg_read(pfuze, PFUZE100_SW1ABSTBY, &reg);
	reg &= ~0x3f;
	reg |= 0x1b;
	pmic_reg_write(pfuze, PFUZE100_SW1ABSTBY, reg);

	/* set SW1AB/VDDARM step ramp up time from 16us to 4us/25mV */
	pmic_reg_read(pfuze, PFUZE100_SW1ABCONF, &reg);
	reg &= ~0xc0;
	reg |= 0x40;
	pmic_reg_write(pfuze, PFUZE100_SW1ABCONF, reg);

	/* set SW1C staby volatage 0.975V */
	pmic_reg_read(pfuze, PFUZE100_SW1CSTBY, &reg);
	reg &= ~0x3f;
	reg |= 0x1b;
	pmic_reg_write(pfuze, PFUZE100_SW1CSTBY, reg);

	/* set SW1C/VDDSOC step ramp up time to from 16us to 4us/25mV */
	pmic_reg_read(pfuze, PFUZE100_SW1CCONF, &reg);
	reg &= ~0xc0;
	reg |= 0x40;
	pmic_reg_write(pfuze, PFUZE100_SW1CCONF, reg);

	return 0;
}
#elif defined(CONFIG_DM_PMIC_PFUZE100)
int power_init_board(void)
{
	struct udevice *dev;
	unsigned int reg;
	int ret;

	dev = pfuze_common_init();
	if (!dev)
		return -ENODEV;

	ret = pfuze_mode_init(dev, APS_PFM);
	if (ret < 0)
		return ret;

	/* set SW1AB staby volatage 0.975V*/
	reg = pmic_reg_read(dev, PFUZE100_SW1ABSTBY);
	reg &= ~0x3f;
	reg |= 0x1b;
	pmic_reg_write(dev, PFUZE100_SW1ABSTBY, reg);

	/* set SW1AB/VDDARM step ramp up time from 16us to 4us/25mV */
	reg = pmic_reg_read(dev, PFUZE100_SW1ABCONF);
	reg &= ~0xc0;
	reg |= 0x40;
	pmic_reg_write(dev, PFUZE100_SW1ABCONF, reg);

	/* set SW1C staby volatage 0.975V*/
	reg = pmic_reg_read(dev, PFUZE100_SW1CSTBY);
	reg &= ~0x3f;
	reg |= 0x1b;
	pmic_reg_write(dev, PFUZE100_SW1CSTBY, reg);

	/* set SW1C/VDDSOC step ramp up time to from 16us to 4us/25mV */
	reg = pmic_reg_read(dev, PFUZE100_SW1CCONF);
	reg &= ~0xc0;
	reg |= 0x40;
	pmic_reg_write(dev, PFUZE100_SW1CCONF, reg);

	return 0;
}
#endif

#ifdef CONFIG_LDO_BYPASS_CHECK
#ifdef CONFIG_POWER_LEGACY
void ldo_mode_set(int ldo_bypass)
{
	u32 value;
	int is_400M;
	struct pmic *p = pmic_get("PFUZE100");

	if (!p) {
		printf("No pmic!\n");
		return;
	}

	/* swith to ldo_bypass mode */
	if (ldo_bypass) {
		prep_anatop_bypass();

		/* decrease VDDARM to 1.1V */
		pmic_reg_read(p, PFUZE100_SW1ABVOL, &value);
		value &= ~0x3f;
		value |= 0x20;
		pmic_reg_write(p, PFUZE100_SW1ABVOL, value);

		/* increase VDDSOC to 1.3V */
		pmic_reg_read(p, PFUZE100_SW1CVOL, &value);
		value &= ~0x3f;
		value |= 0x28;
		pmic_reg_write(p, PFUZE100_SW1CVOL, value);

		is_400M = set_anatop_bypass(0);

		/*
		 * MX6SL: VDDARM:1.175V@800M; VDDSOC:1.175V@800M
		 *        VDDARM:0.975V@400M; VDDSOC:1.175V@400M
		 */
		pmic_reg_read(p, PFUZE100_SW1ABVOL, &value);
		value &= ~0x3f;
		if (is_400M)
			value |= 0x1b;
		else
			value |= 0x23;
		pmic_reg_write(p, PFUZE100_SW1ABVOL, value);

		/* decrease VDDSOC to 1.175V */
		pmic_reg_read(p, PFUZE100_SW1CVOL, &value);
		value &= ~0x3f;
		value |= 0x23;
		pmic_reg_write(p, PFUZE100_SW1CVOL, value);

		finish_anatop_bypass();
		printf("switch to ldo_bypass mode!\n");
	}
}
#elif defined(CONFIG_DM_PMIC_PFUZE100)
void ldo_mode_set(int ldo_bypass)
{
	struct udevice *dev;
	int ret;
	int is_400M;
	u32 vddarm;

	ret = pmic_get("pfuze100@8", &dev);
	if (ret == -ENODEV) {
		printf("No PMIC found!\n");
		return;
	}

	/* switch to ldo_bypass mode , boot on 800Mhz */
	if (ldo_bypass) {
		prep_anatop_bypass();

		/* decrease VDDARM for 400Mhz DQ:1.1V, DL:1.275V */
		pmic_clrsetbits(dev, PFUZE100_SW1ABVOL, 0x3f, 0x20);

		/* increase VDDSOC to 1.3V */
		pmic_clrsetbits(dev, PFUZE100_SW1CVOL, 0x3f, 0x28);

		is_400M = set_anatop_bypass(0);
		if (is_400M)
			vddarm = 0x1b;
		else
			vddarm = 0x23;

		pmic_clrsetbits(dev, PFUZE100_SW1ABVOL, 0x3f, vddarm);

		/* decrease VDDSOC to 1.175V */
		pmic_clrsetbits(dev, PFUZE100_SW1CVOL, 0x3f, 0x23);

		finish_anatop_bypass();
		printf("switch to ldo_bypass mode!\n");
	}
}
#endif

#endif

#ifdef CONFIG_FEC_MXC
static int setup_fec(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* clear gpr1[14], gpr1[18:17] to select anatop clock */
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC_MASK, 0);

	return enable_fec_anatop_clock(0, ENET_50MHZ);
}
#endif

#ifdef CONFIG_USB_EHCI_MX6
#ifndef CONFIG_DM_USB

#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		(1 << 9)

static iomux_v3_cfg_t const usb_otg_pads[] = {
	/* OTG1 */
	MX6_PAD_KEY_COL4__USB_USBOTG1_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EPDC_PWRCOM__ANATOP_USBOTG1_ID | MUX_PAD_CTRL(OTGID_PAD_CTRL),
	/* OTG2 */
	MX6_PAD_KEY_COL5__USB_USBOTG2_PWR | MUX_PAD_CTRL(NO_PAD_CTRL)
};

static void setup_usb(void)
{
	imx_iomux_v3_setup_multiple_pads(usb_otg_pads,
					 ARRAY_SIZE(usb_otg_pads));
}

int board_usb_phy_mode(int port)
{
	if (port == 1)
		return USB_INIT_HOST;
	else
		return usb_phy_mode(port);
}

int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	if (port > 1)
		return -EINVAL;

	usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
				 port * 4);

	/* Set Power polarity */
	setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

	return 0;
}
#endif
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

#ifdef CONFIG_MXC_EPDC
vidinfo_t panel_info = {
	.vl_refresh = 85,
	.vl_col = 800,
	.vl_row = 600,
	.vl_rot = 0,
	.vl_pixclock = 26666667,
	.vl_left_margin = 8,
	.vl_right_margin = 100,
	.vl_upper_margin = 4,
	.vl_lower_margin = 8,
	.vl_hsync = 4,
	.vl_vsync = 1,
	.vl_sync = 0,
	.vl_mode = 0,
	.vl_flag = 0,
	.vl_bpix = 3,
	.cmap = 0,
};

struct epdc_timing_params panel_timings = {
	.vscan_holdoff = 4,
	.sdoed_width = 10,
	.sdoed_delay = 20,
	.sdoez_width = 10,
	.sdoez_delay = 20,
	.gdclk_hp_offs = 419,
	.gdsp_offs = 20,
	.gdoe_offs = 0,
	.gdclk_offs = 5,
	.num_ce = 1,
};

static void setup_epdc_power(void)
{
	/* Setup epdc voltage */

	/* EPDC_PWRSTAT - GPIO2[13] for PWR_GOOD status */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRSTAT__GPIO_2_13 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	gpio_request(IMX_GPIO_NR(2, 13), "EPDC PWRSTAT");
	gpio_direction_input(IMX_GPIO_NR(2, 13));

	/* EPDC_VCOM0 - GPIO2[3] for VCOM control */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_VCOM0__GPIO_2_3 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* Set as output */
	gpio_request(IMX_GPIO_NR(2, 3), "EPDC VCOM0");
	gpio_direction_output(IMX_GPIO_NR(2, 3), 1);

	/* EPDC_PWRWAKEUP - GPIO2[14] for EPD PMIC WAKEUP */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRWAKEUP__GPIO_2_14 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	/* Set as output */
	gpio_request(IMX_GPIO_NR(2, 14), "EPDC PWR WAKEUP");
	gpio_direction_output(IMX_GPIO_NR(2, 14), 1);

	/* EPDC_PWRCTRL0 - GPIO2[7] for EPD PWR CTL0 */
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRCTRL0__GPIO_2_7 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	/* Set as output */
	gpio_request(IMX_GPIO_NR(2, 7), "EPDC PWRCTRL0");
	gpio_direction_output(IMX_GPIO_NR(2, 7), 1);
}

static void epdc_enable_pins(void)
{
	/* epdc iomux settings */
	imx_iomux_v3_setup_multiple_pads(epdc_enable_pads,
				ARRAY_SIZE(epdc_enable_pads));
}

static void epdc_disable_pins(void)
{
	/* Configure MUX settings for EPDC pins to GPIO  and drive to 0 */
	imx_iomux_v3_setup_multiple_pads(epdc_disable_pads,
				ARRAY_SIZE(epdc_disable_pads));
}

static void setup_epdc(void)
{
	unsigned int reg;
	struct mxc_ccm_reg *ccm_regs = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/*** Set pixel clock rates for EPDC ***/

	/* EPDC AXI clk from PFD_400M, set to 396/2 = 198MHz */
	reg = readl(&ccm_regs->chsccdr);
	reg &= ~0x3F000;
	reg |= (0x4 << 15) | (1 << 12);
	writel(reg, &ccm_regs->chsccdr);

	/* EPDC AXI clk enable */
	reg = readl(&ccm_regs->CCGR3);
	reg |= 0x0030;
	writel(reg, &ccm_regs->CCGR3);

	/* EPDC PIX clk from PFD_540M, set to 540/4/5 = 27MHz */
	reg = readl(&ccm_regs->cscdr2);
	reg &= ~0x03F000;
	reg |= (0x5 << 15) | (4 << 12);
	writel(reg, &ccm_regs->cscdr2);

	reg = readl(&ccm_regs->cbcmr);
	reg &= ~0x03800000;
	reg |= (0x3 << 23);
	writel(reg, &ccm_regs->cbcmr);

	/* EPDC PIX clk enable */
	reg = readl(&ccm_regs->CCGR3);
	reg |= 0x0C00;
	writel(reg, &ccm_regs->CCGR3);

	panel_info.epdc_data.wv_modes.mode_init = 0;
	panel_info.epdc_data.wv_modes.mode_du = 1;
	panel_info.epdc_data.wv_modes.mode_gc4 = 3;
	panel_info.epdc_data.wv_modes.mode_gc8 = 2;
	panel_info.epdc_data.wv_modes.mode_gc16 = 2;
	panel_info.epdc_data.wv_modes.mode_gc32 = 2;

	panel_info.epdc_data.epdc_timings = panel_timings;

	setup_epdc_power();
}

void epdc_power_on(void)
{
	unsigned int reg;
	struct gpio_regs *gpio_regs = (struct gpio_regs *)GPIO2_BASE_ADDR;

	/* Set EPD_PWR_CTL0 to high - enable EINK_VDD (3.15) */
	gpio_set_value(IMX_GPIO_NR(2, 7), 1);
	udelay(1000);

	/* Enable epdc signal pin */
	epdc_enable_pins();

	/* Set PMIC Wakeup to high - enable Display power */
	gpio_set_value(IMX_GPIO_NR(2, 14), 1);

	/* Wait for PWRGOOD == 1 */
	while (1) {
		reg = readl(&gpio_regs->gpio_psr);
		if (!(reg & (1 << 13)))
			break;

		udelay(100);
	}

	/* Enable VCOM */
	gpio_set_value(IMX_GPIO_NR(2, 3), 1);

	udelay(500);
}

void epdc_power_off(void)
{
	/* Set PMIC Wakeup to low - disable Display power */
	gpio_set_value(IMX_GPIO_NR(2, 14), 0);

	/* Disable VCOM */
	gpio_set_value(IMX_GPIO_NR(2, 3), 0);

	epdc_disable_pins();

	/* Set EPD_PWR_CTL0 to low - disable EINK_VDD (3.15) */
	gpio_set_value(IMX_GPIO_NR(2, 7), 0);
}
#endif

void setup_elan_pads(void)
{
#define TOUCH_CS	IMX_GPIO_NR(2, 9)
#define TOUCH_INT   IMX_GPIO_NR(2, 10)
#define TOUCH_RST	IMX_GPIO_NR(4, 4)
	imx_iomux_v3_setup_multiple_pads(elan_pads, ARRAY_SIZE(elan_pads));
	gpio_request(TOUCH_CS, "TOUCH CS");
	gpio_request(TOUCH_INT, "TOUCH Interrupt");
	gpio_request(TOUCH_RST, "TOUCH Reset");
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_SYS_I2C_LEGACY
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
#endif

	setup_elan_pads();

#ifdef	CONFIG_FEC_MXC
	setup_fec();
#endif

#ifdef	CONFIG_MXC_EPDC
	setup_epdc();
#endif

#ifdef CONFIG_USB_EHCI_MX6
#ifndef CONFIG_DM_USB
	setup_usb();
#endif
#endif

	return 0;
}

void elan_init(void)
{
	gpio_direction_input(TOUCH_INT);
	/*
	 * If epdc panel not plugged in, gpio_get_value(TOUCH_INT) will
	 * return 1. And no need to mdelay, which will make i2c operation
	 * slow.
	 * If epdc panel plugged in, gpio_get_value(TOUCH_INT) will
	 * return 0. And elan init flow will be executed.
	 */
	if (gpio_get_value(TOUCH_INT))
		return;
	gpio_direction_output(TOUCH_CS , 1);
	gpio_set_value(TOUCH_CS, 0);
	gpio_direction_output(TOUCH_RST , 1);
	gpio_set_value(TOUCH_RST, 0);
	mdelay(10);
	gpio_set_value(TOUCH_RST, 1);
	gpio_set_value(TOUCH_CS, 1);
	mdelay(100);
}

/*
 * This function overwrite the function defined in
 * drivers/i2c/mxc_i2c.c, which is a weak symbol
 */
void i2c_force_reset_slave(void)
{
	elan_init();
}

int board_late_init(void)
{
	env_set("tee", "no");
#ifdef CONFIG_IMX_OPTEE
	env_set("tee", "yes");
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif
	return 0;
}

int checkboard(void)
{
	puts("Board: MX6SLEVK\n");

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <spl.h>
#include <linux/libfdt.h>

#define USDHC1_CD_GPIO	IMX_GPIO_NR(4, 7)
#define USDHC2_CD_GPIO	IMX_GPIO_NR(5, 0)
#define USDHC3_CD_GPIO	IMX_GPIO_NR(3, 22)

static struct fsl_esdhc_cfg usdhc_cfg[3] = {
	{USDHC1_BASE_ADDR},
	{USDHC2_BASE_ADDR, 0, 4},
	{USDHC3_BASE_ADDR, 0, 4},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		gpio_request(USDHC1_CD_GPIO, "cd1_gpio");
		ret = !gpio_get_value(USDHC1_CD_GPIO);
		break;
	case USDHC2_BASE_ADDR:
		gpio_request(USDHC2_CD_GPIO, "cd2_gpio");
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		gpio_request(USDHC3_CD_GPIO, "cd3_gpio");
		ret = !gpio_get_value(USDHC3_CD_GPIO);
		break;
	}

	return ret;
}

int board_mmc_init(struct bd_info *bis)
{
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;
	u32 val;
	u32 port;

	val = readl(&src_regs->sbmr1);

	/* Boot from USDHC */
	port = (val >> 11) & 0x3;
	switch (port) {
	case 0:
		imx_iomux_v3_setup_multiple_pads(usdhc1_pads,
						 ARRAY_SIZE(usdhc1_pads));
		gpio_direction_input(USDHC1_CD_GPIO);
		usdhc_cfg[0].esdhc_base = USDHC1_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
		break;
	case 1:
		imx_iomux_v3_setup_multiple_pads(usdhc2_pads,
						 ARRAY_SIZE(usdhc2_pads));
		gpio_direction_input(USDHC2_CD_GPIO);
		usdhc_cfg[0].esdhc_base = USDHC2_BASE_ADDR;
		usdhc_cfg[0].max_bus_width = 4;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
		break;
	case 2:
		imx_iomux_v3_setup_multiple_pads(usdhc3_pads,
						 ARRAY_SIZE(usdhc3_pads));
		gpio_direction_input(USDHC3_CD_GPIO);
		usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
		usdhc_cfg[0].max_bus_width = 4;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
		break;
	}

	gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
}

const struct mx6sl_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_sdqs0 = 0x00003030,
	.dram_sdqs1 = 0x00003030,
	.dram_sdqs2 = 0x00003030,
	.dram_sdqs3 = 0x00003030,
	.dram_dqm0 = 0x00000030,
	.dram_dqm1 = 0x00000030,
	.dram_dqm2 = 0x00000030,
	.dram_dqm3 = 0x00000030,
	.dram_cas  = 0x00000030,
	.dram_ras  = 0x00000030,
	.dram_sdclk_0 = 0x00000028,
	.dram_reset = 0x00000030,
	.dram_sdba2 = 0x00000000,
	.dram_odt0 = 0x00000008,
	.dram_odt1 = 0x00000008,
};

const struct mx6sl_iomux_grp_regs mx6_grp_ioregs = {
	.grp_b0ds = 0x00000030,
	.grp_b1ds = 0x00000030,
	.grp_b2ds = 0x00000030,
	.grp_b3ds = 0x00000030,
	.grp_addds = 0x00000030,
	.grp_ctlds = 0x00000030,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_ddrmode = 0x00020000,
	.grp_ddr_type = 0x00080000,
};

const struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpdgctrl0 =  0x20000000,
	.p0_mpdgctrl1 =  0x00000000,
	.p0_mprddlctl =  0x4241444a,
	.p0_mpwrdlctl =  0x3030312b,
	.mpzqlp2ctl = 0x1b4700c7,
};

static struct mx6_lpddr2_cfg mem_ddr = {
	.mem_speed = 800,
	.density = 4,
	.width = 32,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.trcd_lp = 2000,
	.trppb_lp = 2000,
	.trpab_lp = 2250,
	.trasmin = 4200,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0xFFFFFFFF, &ccm->CCGR0);
	writel(0xFFFFFFFF, &ccm->CCGR1);
	writel(0xFFFFFFFF, &ccm->CCGR2);
	writel(0xFFFFFFFF, &ccm->CCGR3);
	writel(0xFFFFFFFF, &ccm->CCGR4);
	writel(0xFFFFFFFF, &ccm->CCGR5);
	writel(0xFFFFFFFF, &ccm->CCGR6);

	writel(0x00260324, &ccm->cbcmr);
}

static void spl_dram_init(void)
{
	struct mx6_ddr_sysinfo sysinfo = {
		.dsize = mem_ddr.width / 32,
		.cs_density = 20,
		.ncs = 2,
		.cs1_mirror = 0,
		.walat = 0,
		.ralat = 2,
		.mif3_mode = 3,
		.bi_on = 1,
		.rtt_wr = 0,        /* LPDDR2 does not need rtt_wr rtt_nom */
		.rtt_nom = 0,
		.sde_to_rst = 0,    /* LPDDR2 does not need this field */
		.rst_to_cke = 0x10, /* JEDEC value for LPDDR2: 200us */
		.ddr_type = DDR_TYPE_LPDDR2,
		.refsel = 0,	/* Refresh cycles at 64KHz */
		.refr = 3,	/* 4 refresh commands per refresh cycle */
	};
	mx6sl_dram_iocfg(32, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&sysinfo, &mx6_mmcd_calib, &mem_ddr);
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();

	/* iomux and setup of i2c */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif
