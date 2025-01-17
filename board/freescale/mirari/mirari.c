// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 * Copyright 2023 NXP
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <fdt_support.h>
#include <hwconfig.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_fdt.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_liodn.h>
#include <clock_legacy.h>
#include <fm_eth.h>
#include "../common/sleep.h"
#include "mirari.h"
#include "cpld.h"

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(DM_SERIAL)
int get_serial_clock(void)
{
	return get_bus_freq(0) / 2;
}
#endif

int checkboard(void)
{
	struct cpu_type *cpu = gd->arch.cpu;
	u8 sw;

	printf("Board: %s Mirari\n", cpu->name);
	printf("Board rev: 0x%02x CPLD ver: 0x%02x, ",
	       CPLD_READ(hw_ver), CPLD_READ(sw_ver));

	sw = CPLD_READ(flash_ctl_status);
	sw = ((sw & CPLD_LBMAP_MASK) >> CPLD_LBMAP_SHIFT);

	printf("vBank: %d\n", sw);

	return 0;
}

int board_early_init_f(void)
{
#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot())
		fsl_dp_disable_console();
#endif

	return 0;
}

int board_early_init_r(void)
{
#ifdef CFG_SYS_FLASH_BASE
	const unsigned int flashbase = CFG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 2;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CFG_SYS_FLASH_BASE_PHYS,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);
#endif

	pci_init();

	return 0;
}

int misc_init_r(void)
{
	ccsr_gur_t __iomem *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_s1;

	srds_s1 = in_be32(&gur->rcwsr[4]) >> 24;

	printf("SERDES Reference : 0x%X\n", srds_s1);

	/* select SGMII*/
	if (srds_s1 == 0x86)
		CPLD_WRITE(misc_ctl_status, CPLD_READ(misc_ctl_status) |
					 MISC_CTL_SG_SEL);

	/* select SGMII and Aurora*/
	if (srds_s1 == 0x8E)
		CPLD_WRITE(misc_ctl_status, CPLD_READ(misc_ctl_status) |
					 MISC_CTL_SG_SEL | MISC_CTL_AURORA_SEL);

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);

#ifdef CONFIG_HAS_FSL_DR_USB
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
#ifndef CONFIG_DM_ETH
	fdt_fixup_fman_ethernet(blob);
#endif
#endif

	if (hwconfig("qe-tdm"))
		fdt_del_diu(blob);
	return 0;
}
