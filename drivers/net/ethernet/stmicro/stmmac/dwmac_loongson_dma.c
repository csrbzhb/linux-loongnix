/*******************************************************************************
  This is the driver for the GMAC on-chip Ethernet controller for Loongson SoCs.

  Copyright 2023 Loongson Technology, Inc.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".
*******************************************************************************/

#include <asm/io.h>
#include "dwmac_loongson.h"
#include "dwmac_loongson_dma.h"

static void dwmac_loongson_dma_axi(void __iomem *ioaddr, struct stmmac_axi *axi)
{
	u32 value = readl(ioaddr + DMA_AXI_BUS_MODE);
	int i;

	pr_info("dwmac_loongson: Master AXI performs %s burst length\n",
		!(value & DMA_AXI_UNDEF) ? "fixed" : "any");

	if (axi->axi_lpi_en)
		value |= DMA_AXI_EN_LPI;
	if (axi->axi_xit_frm)
		value |= DMA_AXI_LPI_XIT_FRM;

	value &= ~DMA_AXI_WR_OSR_LMT;
	value |= (axi->axi_wr_osr_lmt & DMA_AXI_WR_OSR_LMT_MASK) <<
		 DMA_AXI_WR_OSR_LMT_SHIFT;

	value &= ~DMA_AXI_RD_OSR_LMT;
	value |= (axi->axi_rd_osr_lmt & DMA_AXI_RD_OSR_LMT_MASK) <<
		 DMA_AXI_RD_OSR_LMT_SHIFT;

	/* Depending on the UNDEF bit the Master AXI will perform any burst
	 * length according to the BLEN programmed (by default all BLEN are
	 * set).
	 */
	for (i = 0; i < AXI_BLEN; i++) {
		switch (axi->axi_blen[i]) {
		case 256:
			value |= DMA_AXI_BLEN256;
			break;
		case 128:
			value |= DMA_AXI_BLEN128;
			break;
		case 64:
			value |= DMA_AXI_BLEN64;
			break;
		case 32:
			value |= DMA_AXI_BLEN32;
			break;
		case 16:
			value |= DMA_AXI_BLEN16;
			break;
		case 8:
			value |= DMA_AXI_BLEN8;
			break;
		case 4:
			value |= DMA_AXI_BLEN4;
			break;
		}
	}

	writel(value, ioaddr + DMA_AXI_BUS_MODE);
}

static void dwmac_loongson_dma_init(void __iomem *ioaddr,
			       struct stmmac_dma_cfg *dma_cfg, int atds)
{
	u32 value = readl(ioaddr + DMA_BUS_MODE);
	int txpbl = dma_cfg->txpbl ?: dma_cfg->pbl;
	int rxpbl = dma_cfg->rxpbl ?: dma_cfg->pbl;

	/*
	 * Set the DMA PBL (Programmable Burst Length) mode.
	 *
	 * Note: before stmmac core 3.50 this mode bit was 4xPBL, and
	 * post 3.5 mode bit acts as 8*PBL.
	 */
	if (dma_cfg->pblx8)
		value |= DMA_BUS_MODE_MAXPBL;
	value |= DMA_BUS_MODE_USP;
	value &= ~(DMA_BUS_MODE_PBL_MASK | DMA_BUS_MODE_RPBL_MASK);
	value |= (txpbl << DMA_BUS_MODE_PBL_SHIFT);
	value |= (rxpbl << DMA_BUS_MODE_RPBL_SHIFT);

	/* Set the Fixed burst mode */
	if (dma_cfg->fixed_burst)
		value |= DMA_BUS_MODE_FB;

	/* Mixed Burst has no effect when fb is set */
	if (dma_cfg->mixed_burst)
		value |= DMA_BUS_MODE_MB;

	if (atds)
		value |= DMA_BUS_MODE_ATDS;

	if (dma_cfg->aal)
		value |= DMA_BUS_MODE_AAL;

	writel(value, ioaddr + DMA_BUS_MODE);

	/* Mask interrupts by writing to CSR7 */
	writel(DMA_INTR_DEFAULT_MASK, ioaddr + DMA_INTR_ENA);
}

static void dwmac_loongson_dma_init_channel(void __iomem *ioaddr,
				    struct stmmac_dma_cfg *dma_cfg, u32 chan)
{
	u32 value;
	int txpbl = dma_cfg->txpbl ?: dma_cfg->pbl;
	int rxpbl = dma_cfg->rxpbl ?: dma_cfg->pbl;

	/* common channel control register config */
	value = readl(ioaddr + DMA_BUS_MODE + chan * DMA_CHAN_OFFSET);

	/*
	 * Set the DMA PBL (Programmable Burst Length) mode.
	 *
	 * Note: before stmmac core 3.50 this mode bit was 4xPBL, and
	 * post 3.5 mode bit acts as 8*PBL.
	 */
	if (dma_cfg->pblx8)
		value |= DMA_BUS_MODE_MAXPBL;
	value |= DMA_BUS_MODE_USP;
	value &= ~(DMA_BUS_MODE_PBL_MASK | DMA_BUS_MODE_RPBL_MASK);
	value |= (txpbl << DMA_BUS_MODE_PBL_SHIFT);
	value |= (rxpbl << DMA_BUS_MODE_RPBL_SHIFT);

	/* Set the Fixed burst mode */
	if (dma_cfg->fixed_burst)
		value |= DMA_BUS_MODE_FB;

	/* Mixed Burst has no effect when fb is set */
	if (dma_cfg->mixed_burst)
		value |= DMA_BUS_MODE_MB;

	value |= DMA_BUS_MODE_ATDS;

	if (dma_cfg->aal)
		value |= DMA_BUS_MODE_AAL;

	writel(value, ioaddr + DMA_BUS_MODE + chan * DMA_CHAN_OFFSET);

	/* Mask interrupts by writing to CSR7 */
	writel(DMA_INTR_DEFAULT_MASK,
	       ioaddr + DMA_INTR_ENA + chan * DMA_CHAN_OFFSET);
}

static void dwmac_loongson_dma_init_rx(void __iomem *ioaddr,
				  struct stmmac_dma_cfg *dma_cfg,
				  u32 dma_rx_phy, u32 chan)
{
	/* RX descriptor base address list must be written into DMA CSR3 */
	writel(dma_rx_phy, ioaddr + DMA_RCV_BASE_ADDR + chan * DMA_CHAN_OFFSET);
}

static void dwmac_loongson_dma_init_tx(void __iomem *ioaddr,
				  struct stmmac_dma_cfg *dma_cfg,
				  u32 dma_tx_phy, u32 chan)
{
	/* TX descriptor base address list must be written into DMA CSR4 */
	writel(dma_tx_phy, ioaddr + DMA_TX_BASE_ADDR + chan * DMA_CHAN_OFFSET);
}

static u32 dwmac_loongson_configure_fc(u32 csr6, int rxfifosz)
{
	csr6 &= ~DMA_CONTROL_RFA_MASK;
	csr6 &= ~DMA_CONTROL_RFD_MASK;

	/* Leave flow control disabled if receive fifo size is less than
	 * 4K or 0. Otherwise, send XOFF when fifo is 1K less than full,
	 * and send XON when 2K less than full.
	 */
	if (rxfifosz < 4096) {
		csr6 &= ~DMA_CONTROL_EFC;
		pr_debug("GMAC: disabling flow control, rxfifo too small(%d)\n",
			 rxfifosz);
	} else {
		csr6 |= DMA_CONTROL_EFC;
		csr6 |= RFA_FULL_MINUS_1K;
		csr6 |= RFD_FULL_MINUS_2K;
	}
	return csr6;
}

static void dwmac_loongson_dma_operation_mode_rx(void __iomem *ioaddr, int mode,
					    u32 channel, int fifosz, u8 qmode)
{
	u32 csr6 = readl(ioaddr + DMA_CONTROL + channel * DMA_CHAN_OFFSET);

	if (mode == SF_DMA_MODE) {
		pr_debug("GMAC: enable RX store and forward mode\n");
		csr6 |= DMA_CONTROL_RSF;
	} else {
		pr_debug("GMAC: disable RX SF mode (threshold %d)\n", mode);
		csr6 &= ~DMA_CONTROL_RSF;
		csr6 &= DMA_CONTROL_TC_RX_MASK;
		if (mode <= 32)
			csr6 |= DMA_CONTROL_RTC_32;
		else if (mode <= 64)
			csr6 |= DMA_CONTROL_RTC_64;
		else if (mode <= 96)
			csr6 |= DMA_CONTROL_RTC_96;
		else
			csr6 |= DMA_CONTROL_RTC_128;
	}

	/* Configure flow control based on rx fifo size */
	csr6 = dwmac_loongson_configure_fc(csr6, fifosz);

	writel(csr6, ioaddr + DMA_CONTROL + channel * DMA_CHAN_OFFSET);
}

static void dwmac_loongson_dma_operation_mode_tx(void __iomem *ioaddr, int mode,
					    u32 channel, int fifosz, u8 qmode)
{
	u32 csr6 = readl(ioaddr + DMA_CONTROL + channel * DMA_CHAN_OFFSET);

	if (mode == SF_DMA_MODE) {
		pr_debug("GMAC: enable TX store and forward mode\n");
		/* Transmit COE type 2 cannot be done in cut-through mode. */
		csr6 |= DMA_CONTROL_TSF;
		/* Operating on second frame increase the performance
		 * especially when transmit store-and-forward is used.
		 */
		csr6 |= DMA_CONTROL_OSF;
	} else {
		pr_debug("GMAC: disabling TX SF (threshold %d)\n", mode);
		csr6 &= ~DMA_CONTROL_TSF;
		csr6 &= DMA_CONTROL_TC_TX_MASK;
		/* Set the transmit threshold */
		if (mode <= 32)
			csr6 |= DMA_CONTROL_TTC_32;
		else if (mode <= 64)
			csr6 |= DMA_CONTROL_TTC_64;
		else if (mode <= 128)
			csr6 |= DMA_CONTROL_TTC_128;
		else if (mode <= 192)
			csr6 |= DMA_CONTROL_TTC_192;
		else
			csr6 |= DMA_CONTROL_TTC_256;
	}

	writel(csr6, ioaddr + DMA_CONTROL + channel * DMA_CHAN_OFFSET);
}

static void dwmac_loongson_dump_dma_regs(void __iomem *ioaddr, u32 *reg_space)
{
	int i;

	for (i = 0; i < NUM_DWMAC_LOONGSON_DMA_REGS; i++)
		if ((i < 12) || (i > 17))
			reg_space[DMA_BUS_MODE / 4 + i] =
				readl(ioaddr + DMA_BUS_MODE + i * 4);
}

static void dwmac_loongson_get_hw_feature(void __iomem *ioaddr,
				     struct dma_features *dma_cap)
{
	u32 hw_cap = readl(ioaddr + DMA_HW_FEATURE);

	dma_cap->mbps_10_100 = (hw_cap & DMA_HW_FEAT_MIISEL);
	dma_cap->mbps_1000 = (hw_cap & DMA_HW_FEAT_GMIISEL) >> 1;
	dma_cap->half_duplex = (hw_cap & DMA_HW_FEAT_HDSEL) >> 2;
	dma_cap->hash_filter = (hw_cap & DMA_HW_FEAT_HASHSEL) >> 4;
	dma_cap->multi_addr = (hw_cap & DMA_HW_FEAT_ADDMAC) >> 5;
	dma_cap->pcs = (hw_cap & DMA_HW_FEAT_PCSSEL) >> 6;
	dma_cap->sma_mdio = (hw_cap & DMA_HW_FEAT_SMASEL) >> 8;
	dma_cap->pmt_remote_wake_up = (hw_cap & DMA_HW_FEAT_RWKSEL) >> 9;
	dma_cap->pmt_magic_frame = (hw_cap & DMA_HW_FEAT_MGKSEL) >> 10;
	/* MMC */
	dma_cap->rmon = (hw_cap & DMA_HW_FEAT_MMCSEL) >> 11;
	/* IEEE 1588-2002 */
	dma_cap->time_stamp =
	    (hw_cap & DMA_HW_FEAT_TSVER1SEL) >> 12;
	/* IEEE 1588-2008 */
	dma_cap->atime_stamp = (hw_cap & DMA_HW_FEAT_TSVER2SEL) >> 13;
	/* 802.3az - Energy-Efficient Ethernet (EEE) */
	dma_cap->eee = (hw_cap & DMA_HW_FEAT_EEESEL) >> 14;
	dma_cap->av = (hw_cap & DMA_HW_FEAT_AVSEL) >> 15;
	/* TX and RX csum */
	dma_cap->tx_coe = (hw_cap & DMA_HW_FEAT_TXCOESEL) >> 16;
	dma_cap->rx_coe_type1 = (hw_cap & DMA_HW_FEAT_RXTYP1COE) >> 17;
	dma_cap->rx_coe_type2 = (hw_cap & DMA_HW_FEAT_RXTYP2COE) >> 18;
	dma_cap->rxfifo_over_2048 = (hw_cap & DMA_HW_FEAT_RXFIFOSIZE) >> 19;
	/* TX and RX number of channels */
	if (((hw_cap & LOONGSON_HW_DMA_CHCNT) >> 20) == 0) {
		dma_cap->number_rx_channel = 8;
		dma_cap->number_tx_channel = 8;
	}
	/* Alternate (enhanced) DESC mode */
	dma_cap->enh_desc = (hw_cap & DMA_HW_FEAT_ENHDESSEL) >> 24;
}

static void dwmac_loongson_rx_watchdog(void __iomem *ioaddr, u32 riwt,
				  u32 number_chan)
{
	u32 chan;

	for (chan = 0; chan < number_chan; chan++)
		writel(riwt, ioaddr + DMA_RX_WATCHDOG + chan * DMA_CHAN_OFFSET);
}

const struct stmmac_dma_ops dwmac_loongson_dma_ops = {
	.reset = dwmac_loongson_dma_reset,
	.init = dwmac_loongson_dma_init,
	.init_chan = dwmac_loongson_dma_init_channel,
	.init_rx_chan = dwmac_loongson_dma_init_rx,
	.init_tx_chan = dwmac_loongson_dma_init_tx,
	.axi = dwmac_loongson_dma_axi,
	.dump_regs = dwmac_loongson_dump_dma_regs,
	.dma_rx_mode = dwmac_loongson_dma_operation_mode_rx,
	.dma_tx_mode = dwmac_loongson_dma_operation_mode_tx,
	.enable_dma_transmission_chan = dwmac_loongson_enable_dma_transmission_chan,
	.enable_dma_irq = dwmac_loongson_enable_dma_irq,
	.disable_dma_irq = dwmac_loongson_disable_dma_irq,
	.start_tx = dwmac_loongson_dma_start_tx,
	.stop_tx = dwmac_loongson_dma_stop_tx,
	.start_rx = dwmac_loongson_dma_start_rx,
	.stop_rx = dwmac_loongson_dma_stop_rx,
	.dma_interrupt = dwmac_loongson_dma_interrupt,
	.get_hw_feature = dwmac_loongson_get_hw_feature,
	.rx_watchdog = dwmac_loongson_rx_watchdog,
};
