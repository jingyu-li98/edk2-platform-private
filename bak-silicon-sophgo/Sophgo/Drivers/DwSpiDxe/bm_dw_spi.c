#include <common.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "timer.h"
#include "system_common.h"
#include "mmio.h"
#include "module_testcase.h"
#include "bm_dw_spi.h"
#include "dma.h"
#include "command.h"
#include "cli.h"

uint8_t default_tx[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x10, 0x11, 0x12,
	0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
	0x19, 0x20, 0x21, 0x22, 0x23, 0x24,
	0x25, 0x26, 0x27, 0x28, 0x29, 0x30,
	0xF0, 0x0D,
};

uint8_t default_rx[ARRAY_SIZE(default_tx)] = {0, };

struct dw_spi {
	/* Current message transfer state info */
	u16 len;
	void *tx;
	void *tx_end;
	void *rx;
	void *rx_end;
	u8 n_bytes;
};

#define L1_CACHE_BYTES 64

static inline void CBO_flush(unsigned long start, unsigned long size)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);
	for (; i < (start+size); i += L1_CACHE_BYTES)
		asm volatile (".long 0x025200f");
}

static inline void CBO_clean(unsigned long start, unsigned long size)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);
	for (; i < (start+size); i += L1_CACHE_BYTES)
 		asm volatile (".long 0x015200f");
}

static inline void CBO_inval(unsigned long start, unsigned long size)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);
	for (; i < (start+size); i += L1_CACHE_BYTES)
		asm volatile (".long 0x005200f");
}

static void SPI_WRITE(int offset, u32 value)
{
	writel(SPI_BASE + offset , (u32)value);
	__asm__ volatile("fence iorw, iorw":::);
	CBO_flush(SPI_BASE + offset, 4);
	__asm__ volatile("fence iorw, iorw":::);
}

static u32 SPI_READ(int offset)
{
	CBO_inval(SPI_BASE + offset, 4);
	__asm__ volatile("fence iorw, iorw":::);
	u32 val = readl(SPI_BASE + offset);
	__asm__ volatile("fence iorw, iorw":::);
	return val;
}

int dw_spi_irq_handler(int irqn, void *priv)
{
	uartlog("---In IRQ---\n");
	uartlog("%s  irqn=%d ISR: 0x%x\n", __func__, irqn, SPI_READ(DW_SPI_RISR));

	mmio_clrbits_32(SPI_BASE + DW_SPI_IMR, SPI_READ(DW_SPI_RISR));
	disable_irq(irqn);
	return 0;
}

bool loop_back = true;

static void dw_spi_enable(bool enable)
{
	if (enable == true)
		SPI_WRITE(DW_SPI_SSIENR, 0x1);
	else
		SPI_WRITE(DW_SPI_SSIENR, 0x0);

	udelay(5);

	printf("%sabling SPI\n", enable ? "En" : "Dis");
}

static int spi_rx(void *rx_buf, u16 len, u8 n_bytes)
{
	u32 max = len;
	u16 rxw;

	while (max--) {
		rxw = SPI_READ(DW_SPI_DR);
		/* Care rx only if the transfer's original "rx" is not null */
		if (n_bytes == 1)
			*(u8 *)(rx_buf) = rxw;
		else if (n_bytes == 2)
			*(u16 *)(rx_buf) = rxw;
		else {
			printf("No support of this format\n");
			return 1;
		}
		uartlog("rxw: %x tx len: %d\n", rxw, SPI_READ(DW_SPI_RXFLR));
		rx_buf += n_bytes;
	}
	return 0;
}

static int spi_tx(void *tx_buf, u16 len, u8 n_bytes)
{
	u32 max = len;
	u16 txw = 0;

	timer_meter_start();

	while (max > 0) {
		/* Set the tx word if the transfer's original "tx" is not null */
		if ((SPI_READ(DW_SPI_RISR) & SPI_RISR_TXOIR) != SPI_RISR_TXOIR) {
			timer_meter_start();

			if (n_bytes == 1)
				txw = *(u8 *)(tx_buf);
			else
				txw = *(u16 *)(tx_buf);
			SPI_WRITE(DW_SPI_DR, txw);
			uartlog("txw: %x tx len: %d\n", txw, SPI_READ(DW_SPI_TXFLR));
			tx_buf += n_bytes;
			max--;
		} else if (timer_meter_get_ms() < 100) {
			udelay(10); /* wait FIFO exit overflow status */
		} else {
			printf("SPI TX timeout\n");
			return 1;
		}
	}
	return 0;
}

static int spi_loopback(struct dw_spi *dws)
{
	u32 max = dws->len;
	u16 txw = 0;
	u16 rxw = 0;

	timer_meter_start();
	while (max > 0) {
		/* Set the tx word if the transfer's original "tx" is not null */
		if ((SPI_READ(DW_SPI_RISR) & SPI_RISR_TXOIR) != SPI_RISR_TXOIR) {
			timer_meter_start();

			if (dws->n_bytes == 1)
				txw = *(u8 *)(dws->tx);
			else
				txw = *(u16 *)(dws->tx);

			SPI_WRITE(DW_SPI_DR, txw);
			dws->tx += dws->n_bytes;
			udelay(10);

			rxw = SPI_READ(DW_SPI_DR);
			if (dws->n_bytes == 1)
				*(u8 *)(dws->rx) = rxw;
			else
				*(u16 *)(dws->rx) = rxw;

			dws->rx += dws->n_bytes;
			max--;
		} else if (timer_meter_get_ms() < 100) {
			udelay(10); /* wait FIFO exit overflow status */
		} else {
			printf("SPI TX timeout\n");
			return 1;
		}
		__asm__ volatile("fence iorw, iorw":::);
	}

	return 0;
}

void spi_init(u8 n_bytes)
{
	u32 ctrl0 = 0;
	int spi_mode = 3;

	dw_spi_enable(false);
	ctrl0 = (SPI_READ(DW_SPI_CTRL0) & ~(SPI_TMOD_MASK
						| SPI_SCPOL_MASK
						| SPI_SCPH_MASK
						| SPI_FRF_MASK
						| SPI_DFS_MASK));
	if (n_bytes == 1) /* 8 bits data */
		SPI_WRITE(DW_SPI_CTRL0, ctrl0 | SPI_TMOD_TR | (spi_mode << 6)
						| SPI_FRF_SPI | SPI_DFS_8BIT); /* Set to SPI frame format */
	else if (n_bytes == 2)
		SPI_WRITE(DW_SPI_CTRL0, ctrl0 | SPI_TMOD_TR | (spi_mode << 6)
						| SPI_FRF_SPI | SPI_DFS_16BIT);
	SPI_WRITE(DW_SPI_BAUDR, SPI_BAUDR_DIV);
	SPI_WRITE(DW_SPI_TXFLTR, 4);
	SPI_WRITE(DW_SPI_RXFLTR, 4);
	SPI_WRITE(DW_SPI_SER, 0x1); /* enable slave 1 device */
	uartlog("CS %x\n", SPI_READ(DW_SPI_SER));
	uartlog("SPI mode: %d\n", ((SPI_READ(DW_SPI_CTRL0) >> 6) & 0x3));

	dw_spi_enable(true);
}

int testcase_spi(void)
{
	struct dw_spi dws;
	u8 idx = 0;

	dws.tx = (void *)default_tx;
	dws.len = ARRAY_SIZE(default_tx);
	dws.tx_end = dws.tx + dws.len;
	dws.rx = (void *)default_rx;
	dws.rx_end = dws.rx + dws.len;

	printf("\n------------ SPI test start ------------\n\n");

	dws.n_bytes = 1; /* test 8 bits data */
	spi_init(dws.n_bytes);

	printf("TX data:");
	for(idx=0; idx < dws.len; idx++)
		printf("%X ", default_tx[idx]);
	printf("\n\n");

	CBO_flush((unsigned long)dws.tx, dws.len);

	if (loop_back == true) {
		spi_loopback(&dws);
	} else {
		spi_tx(dws.tx, dws.len, dws.n_bytes);
		spi_rx(dws.rx, dws.len, dws.n_bytes);
	}

	printf("RX data:");
	for(idx=0; idx < dws.len; idx++)
		printf("%X ", default_rx[idx]);

	printf("\n------------ SPI test done ------------\n");

	return 0;
}

static int do_trx(int argc, char **argv)
{
	int ret;

	ret = testcase_spi();
	uartlog("testcase spi %s\n", ret ? "failed" : "passed");

	return ret;
}

static int do_inv_cs(int argc, char **argv)
{
	struct dw_spi dws;
	u8 idx = 0;
	uint8_t tmp_tx[] = {0x2};
	uint8_t tmp_rx[] = {0x0};

	dws.tx = (void *)tmp_tx;
	dws.len = ARRAY_SIZE(tmp_tx);
	dws.tx_end = dws.tx + dws.len;
	dws.rx = (void *)tmp_rx;
	dws.rx_end = dws.rx + dws.len;

	printf("\n------------ SPI CS invert test start ------------\n\n");

	dws.n_bytes = 1; /* test 8 bits data */
	spi_init(dws.n_bytes);

	dw_spi_enable(false);
	SPI_WRITE(DW_SPI_CTRL2, CVI_SS_INV_EN);
	dw_spi_enable(true);

	printf("TX data:");
	for(idx=0; idx < dws.len; idx++)
		printf("%X ", tmp_tx[idx]);
	printf("\n\n");
	if (loop_back == true) {
		spi_loopback(&dws);
	} else {
		spi_tx(dws.tx, dws.len, dws.n_bytes);
		spi_rx(dws.rx, dws.len, dws.n_bytes);
	}
	printf("RX data:");
	for(idx=0; idx < dws.len; idx++)
		printf("%X ", tmp_rx[idx]);

	printf("\n\n");
	printf("\n------------ SPI CS invert test done ------------\n");

	return 0;
}

static int do_lsb(int argc, char **argv)
{
	struct dw_spi dws;
	u8 idx = 0;
	uint8_t tmp_tx[] = {0x2};
	uint8_t tmp_rx[] = {0x0};

	dws.tx = (void *)tmp_tx;
	dws.len = ARRAY_SIZE(tmp_tx);
	dws.tx_end = dws.tx + dws.len;
	dws.rx = (void *)tmp_rx;
	dws.rx_end = dws.rx + dws.len;

	printf("\n------------ SPI CS LSB test start ------------\n\n");

	dws.n_bytes = 1; /* test 8 bits data */
	spi_init(dws.n_bytes);

	dw_spi_enable(false);
	SPI_WRITE(DW_SPI_CTRL2, CVI_LAB_FIRST);
	dw_spi_enable(true);

	printf("TX data with len %d: \n", dws.len);
	for(idx=0; idx < dws.len; idx++)
		printf("%X ", tmp_tx[idx]);
	printf("\n\n");
	if (loop_back == true) {
		spi_loopback(&dws);
	} else {
		spi_tx(dws.tx, dws.len, dws.n_bytes);
		spi_rx(dws.rx, dws.len, dws.n_bytes);
	}
	printf("RX data:");
	for(idx=0; idx < dws.len; idx++)
		printf("%X ", tmp_rx[idx]);

	printf("\n\n");
	printf("\n------------ SPI CS LSB test done ------------\n");

	return 0;
}

static int do_spi_cs_test(int argc, char **argv)
{
	int spi_mode = 3;
	struct dw_spi dws;
	int idx;
	int cs = 0;
	u32 ctrl0 = 0;

	if (argc < 2) {
		uartlog("param error!\n");
		uartlog("cs_test <cs>\n");
		return -1;
	}
	cs = strtoul(argv[1], NULL, 10);
	dw_spi_enable(false);
	ctrl0 = (SPI_READ(DW_SPI_CTRL0) & ~(SPI_TMOD_MASK
						| SPI_SCPOL_MASK
						| SPI_SCPH_MASK
						| SPI_FRF_MASK
						| SPI_DFS_MASK));
	SPI_WRITE(DW_SPI_CTRL0, ctrl0 | SPI_TMOD_TR | (spi_mode << 6)
					| SPI_FRF_SPI | SPI_DFS_8BIT);  // Set to SPI frame format
	SPI_WRITE(DW_SPI_BAUDR, SPI_BAUDR_DIV);
	SPI_WRITE(DW_SPI_TXFLTR, 4);
	SPI_WRITE(DW_SPI_RXFLTR, 4);
	SPI_WRITE(DW_SPI_SER, 1 << cs);
	uartlog("CS %x\n", SPI_READ(DW_SPI_SER));
	uartlog("SPI mode: %d\n", ((SPI_READ(DW_SPI_CTRL0) >> 6) & 0x3));

	dw_spi_enable(true);
	dws.tx = (void *)default_tx;
	dws.len = ARRAY_SIZE(default_tx);
	dws.tx_end = dws.tx + dws.len;
	dws.rx = (void *)default_rx;
	dws.rx_end = dws.rx + dws.len;
	printf("TX data:");
	for (idx = 0; idx < dws.len; idx++)
		printf("%X ", default_tx[idx]);
	CBO_flush((unsigned long)dws.tx, dws.len);

	spi_loopback(&dws);
	return 0;
}

static int do_interupt(int argc, char **argv)
{
	int i = 0;
	struct dw_spi dws;
	uint8_t tmp_tx[] = {0x0, 0x1, 0x2, 0x3, 0x4};
	uint8_t tmp_rx[] = {0x0, 0x0, 0x0, 0x0, 0x0};

	request_irq(DW_SPI_INTR, dw_spi_irq_handler, 0, "spi int", NULL);
	uartlog("---Interrupt Test---\n");
	dw_spi_enable(true);
	uartlog("SSER: 0x%x\n", SPI_READ(DW_SPI_SSIENR));
	uartlog("Tragger:\nTransmit FIFO Overflow Interruptt\n");

	/*
	 * Transmit FIFO Overflow Interrupt:
	 * 	access attempts to write into the transmit FIFO
	 * 	after it has been completely filled
	 * Receive FIFO Overflow Interrupt:
	 * 	receive logic attempts to place data into the
	 * 	receive FIFO after it has been completely filled
	 */
	for (i = 0; i < 257; i++) {
		SPI_WRITE(DW_SPI_DR, i);
	}

	uartlog("Tragger:\nTransmit FIFO Empty Interrupt\nReceive FIFO Full Interrupt\n");

	/*
	 * Transmit FIFO Empty Interrupt:
	 * 	transmit FIFO is equal to or below its threshold value
	 * Receive FIFO Full Interrupt:
	 *	receive FIFO is equal to or above its threshold value plus 1
	 */
	dws.tx = (void *)tmp_tx;
	dws.len = ARRAY_SIZE(tmp_tx);
	dws.tx_end = dws.tx + dws.len;
	dws.rx = (void *)tmp_rx;
	dws.rx_end = dws.rx + dws.len;
	dws.n_bytes = 1;
	spi_init(1);
	spi_tx(dws.tx, dws.len, dws.n_bytes);
	spi_rx(dws.rx, dws.len, dws.n_bytes);

	/*
	 * Receive FIFO Underflow Interrupt:
	 * 	access attempts to read from the receive FIFO when it is empty
	 */
	uartlog("valid data entries in receive FIFO: %d\n", SPI_READ(DW_SPI_RXFLR));
	uartlog("Tragger:\nReceive FIFO Underflow Interrupt\n");
	SPI_READ(DW_SPI_DR);

	dw_spi_enable(true);
	uartlog("SSER: 0x%x\n", SPI_READ(DW_SPI_SSIENR));
	uartlog("Tragger:Receive FIFO Overflow Interrupt\n");

	/*
	 * Transmit FIFO Overflow Interrupt
	 * 	access attempts to write into the transmit FIFO
	 * 	after it has been completely filled
	 * Receive FIFO Overflow Interrupt
	 * 	receive logic attempts to place data into the receive
	 * 	FIFO after it has been completely filled
	 */
	for (i = 0; i < 257; i++) {
		SPI_WRITE(DW_SPI_DR, i);
	}

	return 0;
}

static int do_reset(int argc, char **argv)
{
	u64 sys_ctrl_rstgen = SYS_CTRL_RSTGEN_BASE;
	int sw_reg2_offset = SPI_RST_OFFSET;
	int sw_rst_reset_x_spi = SPI0_RST_SHIFT;  // spi0

#if SPI_TEST_CASE == 2
	u32 ctrl0 = 0;
	int spi_mode = 3;

	sys_ctrl_rstgen = RTC_CTRL_BASE;
	sw_reg2_offset = RTC_SPI_RST_OFFSET;
	sw_rst_reset_x_spi = RTC_SPI_RST_SHIFT;  // rtc spi
	printf("rtc spi reset test\n");

	dw_spi_enable(false);
	ctrl0 = (SPI_READ(DW_SPI_CTRL0) & ~(SPI_TMOD_MASK
					| SPI_SCPOL_MASK
					| SPI_SCPH_MASK
					| SPI_FRF_MASK
					| SPI_DFS_MASK));
	SPI_WRITE(DW_SPI_CTRL0, ctrl0 | (0x1f << 16) | (0xf << 12)
					| (0x1 << 8) | (spi_mode << 6 ));
	printf("DW_SPI_CTRL0: 0x%x\n", SPI_READ(DW_SPI_CTRL0));

	printf("start reset\n");
	mmio_clrbits_32(sys_ctrl_rstgen + sw_reg2_offset, 0x1 << sw_rst_reset_x_spi);
	mdelay(1);
	mmio_setbits_32(sys_ctrl_rstgen + sw_reg2_offset, 0x1 << sw_rst_reset_x_spi);
	printf("reset done\n");

	printf("DW_SPI_CTRL0: 0x%x\n", SPI_READ(DW_SPI_CTRL0));
	dw_spi_enable(true);

	return 0;

#elif SPI_TEST_CASE == 0
	printf("spi0 reset test\n");
#elif SPI_TEST_CASE == 1
	sw_rst_reset_x_spi = SPI1_RST_SHIFT;  // spi1
	printf("spi1 reset test\n");
#endif

	dw_spi_enable(false);
	SPI_WRITE(DW_SPI_CTRL2, 0xFB3F);
	printf("SPI_CTRLR0: 0x%x\n", SPI_READ(DW_SPI_CTRL2));

	printf("start reset\n");
	mmio_clrbits_32(sys_ctrl_rstgen + sw_reg2_offset, 0x1 << sw_rst_reset_x_spi);
	mdelay(1);
	mmio_setbits_32(sys_ctrl_rstgen + sw_reg2_offset, 0x1 << sw_rst_reset_x_spi);
	printf("reset done\n");

	printf("SPI_CTRLR0(after reset): 0x%x\n", SPI_READ(DW_SPI_CTRL2));
	dw_spi_enable(true);

	return 0;
}

static int clk_gating(int argc, char **argv)
{
#if SPI_TEST_CASE == 2
	uint32_t clk_spi = 0;

	clk_spi = (mmio_read_32(RTC_CTRL_BASE + RTC_LPIP_CLKEN_OFFSET) >> RTC_SPI_CLKEN_SHIFT) & 0x1;
	printf("origin rtc spi clk: %X\n", clk_spi);

	SPI_WRITE(DW_SPI_TXFLTR, 11);
	printf("TXFLTR: %d\n", SPI_READ(DW_SPI_TXFLTR));
	printf("----\n");

	printf("disable rtc spi apb clk\n");
	mmio_clrbits_32(RTC_CTRL_BASE + RTC_LPIP_CLKEN_OFFSET, 0x1 << RTC_SPI_CLKEN_SHIFT);
	clk_spi = (mmio_read_32(RTC_CTRL_BASE + RTC_LPIP_CLKEN_OFFSET) >> RTC_SPI_CLKEN_SHIFT) & 0x1;
	printf("rtc spi clk(after clk gating): %X\n", clk_spi);
	SPI_WRITE(DW_SPI_TXFLTR, 4);
	printf("write TXFLTR to 4 in clk disable state.\n");
	printf("TXFLTR when in clk disable: %d\n", SPI_READ(DW_SPI_TXFLTR));

	printf("enable clk.\n");
	mmio_setbits_32(RTC_CTRL_BASE + RTC_LPIP_CLKEN_OFFSET, 0x1 << RTC_SPI_CLKEN_SHIFT);
	clk_spi = (mmio_read_32(RTC_CTRL_BASE + RTC_LPIP_CLKEN_OFFSET) >> RTC_SPI_CLKEN_SHIFT) & 0x1;
	printf("Current rtc spi clk: %X\n", clk_spi);

	return 0;
#else
	int spi_num = 0;
	uint32_t clk_spi = 0;

#if SPI_TEST_CASE == 1
	spi_num = 1;
#endif

	clk_spi = (mmio_read_32(SYS_CTRL_CLKGEN_DIV + CLKEN_H368_OFFSET)) & 0xF;
	printf("origin spi clk: 0x%X\n", clk_spi);
	printf("spi%d CLK state: %d\n", spi_num, (clk_spi >> spi_num) & 0b1);

	SPI_WRITE(DW_SPI_TXFLTR, 11);
	printf("TXFLTR: %d\n", SPI_READ(DW_SPI_TXFLTR));
	printf("----\n");

	printf("disable spi%d apb clk\n", spi_num);
	mmio_clrbits_32(SYS_CTRL_CLKGEN_DIV + CLKEN_H368_OFFSET, 0x1 << spi_num);
	clk_spi = (mmio_read_32(SYS_CTRL_CLKGEN_DIV + CLKEN_H368_OFFSET)) & 0xF;
	printf("spi clk(after clk gating): %X\n", clk_spi);
	printf("spi%d CLK state: %d\n", spi_num, (clk_spi >> spi_num) & 0x1);
	SPI_WRITE(DW_SPI_TXFLTR, 4);
	printf("write TXFLTR to 4 in clk disable state.\n");
	printf("TXFLTR when in clk disable: %d\n", SPI_READ(DW_SPI_TXFLTR));

	printf("enable clk.\n");
	mmio_setbits_32(SYS_CTRL_CLKGEN_DIV + CLKEN_H368_OFFSET, 0x1 << spi_num);

	clk_spi = (mmio_read_32(SYS_CTRL_CLKGEN_DIV + CLKEN_H368_OFFSET)) & 0xF;
	printf("Current spi clk: %X\n", clk_spi);

	return 0;
#endif
}

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
	{"trx", do_trx, 0, "do spi loop test"},
	{"invert_cs", do_inv_cs, 0, "invert cs test (active high)"},
	{"lsb", do_lsb, 0, "TRX with LSB"},
	{"cs_test", do_spi_cs_test, 0, "CS test"},
	{"intr_test", do_interupt, 0, "test 5 interupts"},
	{"reset_test", do_reset, 0, "reset TXFLTR"},
	{"clk_gatting", clk_gating, 0, "clock gatting test"},
	{NULL, NULL, 0, NULL}
};

void pinmux_spi()
{
	u64 pinlist_gx = 0;
	u32 start_addr = 0;

#if SPI_TEST_CASE == 0
	pinlist_gx = SG2380_IO_G10;
	start_addr = 0x4;
	printf("Pinmux for spi0\n");
	mmio_clrsetbits_32(pinlist_gx + start_addr, 0xF << 4, 0x1 << 4);	// spi0 cs1
	mmio_clrsetbits_32(pinlist_gx + start_addr + 4, 0xF << 4, 0x1 << 4);	// spi0 SDI
	mmio_clrsetbits_32(pinlist_gx + start_addr + 8, 0xF << 4, 0x1 << 4);	// spi0 SDO
	mmio_clrsetbits_32(pinlist_gx + start_addr + 12, 0xF << 4, 0x1 << 4);	// spi0 wp_x
	mmio_clrsetbits_32(pinlist_gx + start_addr + 16, 0xF << 4, 0x1 << 4);	// spi0 hold_x
	mmio_clrsetbits_32(pinlist_gx + start_addr + 20, 0xF << 4, 0x1 << 4);	// spi0 sck

#elif SPI_TEST_CASE == 1
	pinlist_gx = SG2380_IO_G10;
	start_addr = 0x154;
	printf("Pinmux for spi1\n");
	mmio_clrsetbits_32(pinlist_gx + start_addr, 0xF << 4, 0x1 << 4);	// cs0
	mmio_clrsetbits_32(pinlist_gx + start_addr + 4, 0xF << 4, 0x1 << 4);	// cs1
	mmio_clrsetbits_32(pinlist_gx + start_addr + 8, 0xF << 4, 0x1 << 4);	// sdi
	mmio_clrsetbits_32(pinlist_gx + start_addr + 12, 0xF << 4, 0x1 << 4);	// sdo
	mmio_clrsetbits_32(pinlist_gx + start_addr + 16, 0xF << 4, 0x1 << 4);	// wp

	mmio_clrsetbits_32(SG2380_IO_G11, 0xF << 4, 0x2 << 4);			// spi1_sck G11
	mmio_clrsetbits_32(SG2380_IO_G11 + 4, 0xF << 4, 0x1 << 4);		// spi1_hold G11

#elif SPI_TEST_CASE == 2
	pinlist_gx = SG2380_IO_G7;
	start_addr = 0xc;
	printf("Pinmux for rtc spi\n");
	mmio_clrsetbits_32(pinlist_gx + start_addr, 0xF << 4, 0x1 << 4);	// sck
	mmio_clrsetbits_32(pinlist_gx + start_addr + 4, 0xF << 4, 0x1 << 4);	// cs0
	mmio_clrsetbits_32(pinlist_gx + start_addr + 8, 0xF << 4, 0x1 << 4);	// cs1
	mmio_clrsetbits_32(pinlist_gx + start_addr + 12, 0xF << 4, 0x1 << 4);	// cs2
	mmio_clrsetbits_32(pinlist_gx + start_addr + 16, 0xF << 4, 0x1 << 4);	// sdi
	mmio_clrsetbits_32(pinlist_gx + start_addr + 20, 0xF << 4, 0x1 << 4);	// sdo
#endif
}

int testcase_main(void)
{
	int i;

	pinmux_spi();
	for(i = 0;i < ARRAY_SIZE(test_cmd_list) - 1;i++) {
		command_add(&test_cmd_list[i]);
		printf(i ? "\t%s\n" : "Commond List:\n\t%s\n", test_cmd_list[i].name);
	}
	cli_simple_loop();

	return 0;
}

module_testcase("1", testcase_spi);
