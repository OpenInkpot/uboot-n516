/*
 * Platform independend driver for JZ4730.
 *
 * Copyright (c) 2007 Ingenic Semiconductor Inc.
 * Author: <jlwei@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#include <common.h>

#if defined(CONFIG_CMD_NAND) && defined(CONFIG_JZ4730)

#include <nand.h>

#include <asm/jz4730.h>

static void jz_hwcontrol(struct mtd_info *mtd, int dat, 
			 unsigned int ctrl)
{
	struct nand_chip *this = (struct nand_chip *)(mtd->priv);
	unsigned int nandaddr = (unsigned int)this->IO_ADDR_W;

	if (ctrl & NAND_CTRL_CHANGE) {
		if ( ctrl & NAND_ALE )
			nandaddr = (unsigned int)((unsigned long)(this->IO_ADDR_W) | 0x00080000);
		else
			nandaddr = (unsigned int)((unsigned long)(this->IO_ADDR_W) & ~0x00080000);

		if ( ctrl & NAND_CLE )
			nandaddr = nandaddr | 0x00040000;
		else
			nandaddr = nandaddr & ~0x00040000;
		if ( ctrl & NAND_NCE )
			REG_EMC_NFCSR |= EMC_NFCSR_FCE;
		else
			REG_EMC_NFCSR &= ~EMC_NFCSR_FCE;
	}

	this->IO_ADDR_W = (void __iomem *)nandaddr;
	if (dat != NAND_CMD_NONE)
		writeb(dat , this->IO_ADDR_W);

}

static int jz_device_ready(struct mtd_info *mtd)
{
	int ready;
	ready = (REG_EMC_NFCSR & EMC_NFCSR_RB) ? 1 : 0;
	return ready;
}

/*
 * EMC setup
 */
static void jz_device_setup(void)
{
	/* Set NFE bit */
	REG_EMC_NFCSR |= EMC_NFCSR_NFE;
	REG_EMC_SMCR3 = 0x04444400;
}

void board_nand_select_device(struct nand_chip *nand, int chip)
{
	/*
	 * Don't use "chip" to address the NAND device,
	 * generate the cs from the address where it is encoded.
	 */
}

/*
 * Main initialization routine
 */
void board_nand_init(struct nand_chip *nand)
{
	jz_device_setup();

	nand->ecc.mode = NAND_ECC_NONE;	/* FIXME: should use NAND_ECC_SOFT */
        nand->cmd_ctrl = jz_hwcontrol;
        nand->dev_ready = jz_device_ready;

        /* Set address of NAND IO lines */
        nand->IO_ADDR_R = (void __iomem *) CONFIG_SYS_NAND_BASE;
        nand->IO_ADDR_W = (void __iomem *) CONFIG_SYS_NAND_BASE;

        /* 20 us command delay time */
        nand->chip_delay = 20;
}

#endif /* defined(CONFIG_CMD_NAND) */
