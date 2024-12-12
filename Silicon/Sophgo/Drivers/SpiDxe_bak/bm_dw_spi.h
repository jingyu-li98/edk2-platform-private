#ifndef _TESTCASE_SPI_H_
#define _TESTCASE_SPI_H_

#define SRI_RTC_BASE		0x50B6031C00
#define DW_SPI_INTR_RTC		220
#define DW_SPI_INTR0		280
#define DW_SPI_INTR1		281

#define SG2380_IO_G11		0x5088104C00
#define SG2380_IO_G10		0x5088104A00
#define SG2380_IO_G7		0x50B6027000

#define SYS_CTRL_RSTGEN_BASE	0x5088103000
#define SPI_RST_OFFSET		0x08
#define SPI0_RST_SHIFT		8
#define SPI1_RST_SHIFT		9
#define SYS_CTRL_CLKGEN_DIV	0x5088102000
#define CLKEN_H368_OFFSET	0x368

#define RTC_CTRL_BASE		0x50B6025000
#define RTC_SPI_RST_OFFSET	0x18
#define RTC_SPI_RST_SHIFT	21
#define RTC_LPIP_CLKEN_OFFSET	0x104
#define RTC_SPI_CLKEN_SHIFT	19

/*
 * SPI_TEST_CASE
 * 	0: SPI0
 * 	1: SPI1
 * 	2: RTC_SPI
 */
#define SPI_TEST_CASE		0

#if SPI_TEST_CASE == 0
#define DW_SPI_INTR 		DW_SPI_INTR0
#define DW_SPI0_BASE		SPI0_BASE
#define SPI_BASE		DW_SPI0_BASE

#elif SPI_TEST_CASE == 1
#define DW_SPI_INTR		DW_SPI_INTR1
#define DW_SPI0_BASE		SPI1_BASE
#define SPI_BASE		DW_SPI0_BASE

#elif SPI_TEST_CASE == 2
#define DW_SPI_INTR 		DW_SPI_INTR_RTC
#define DW_SPI0_BASE		SRI_RTC_BASE
#define SPI_BASE		DW_SPI0_BASE
#endif

#define ARRAY_SIZE(a)		(sizeof(a) / sizeof((a)[0]))

#define SPI_CS_TOG_EN		0x030001D0
#define DMA_REMAP_UPDATE	0x1

/* Register offsets */
#define DW_SPI_CTRL0		0x00
#define DW_SPI_CTRL1		0x04
#define DW_SPI_SSIENR		0x08
#define DW_SPI_MWCR		0x0c
#define DW_SPI_SER		0x10
#define DW_SPI_BAUDR		0x14
#define DW_SPI_TXFLTR		0x18
#define DW_SPI_RXFLTR		0x1c
#define DW_SPI_TXFLR		0x20
#define DW_SPI_RXFLR		0x24
#define DW_SPI_SR		0x28
#define DW_SPI_IMR		0x2c
#define DW_SPI_ISR		0x30
#define DW_SPI_RISR		0x34
#define DW_SPI_TXOICR		0x38
#define DW_SPI_RXOICR		0x3c
#define DW_SPI_RXUICR		0x40
#define DW_SPI_MSTICR		0x44
#define DW_SPI_ICR		0x48
#define DW_SPI_DMACR		0x4c
#define DW_SPI_DMATDLR		0x50
#define DW_SPI_DMARDLR		0x54
#define DW_SPI_IDR		0x58
#define DW_SPI_VERSION		0x5c
#define DW_SPI_DR		0x60
#define DW_SPI_CTRL2		0xF4
#define DW_SPI_CS_CTRL2		0xF8

/* CTRL0 filed*/
#define SPI_TMOD_MASK		(0x3 << 8)
#define	SPI_TMOD_TR		0x0000		/* xmit & recv */
#define SPI_TMOD_TO		0x0100		/* xmit only */
#define SPI_TMOD_RO		0x0200		/* recv only */
#define SPI_EERPOMREAD		0x0300		/* EEPROM READ */
#define SPI_FRF_MASK		0x0030
#define SPI_SCPOL_MASK		0x0080
#define SPI_SCPOL_LOW		0x0000
#define SPI_SCPOL_HIGH		0x0080
#define SPI_SCPH_MASK		0x0040
#define SPI_SCPH_MIDDLE		0x0000
#define SPI_SCPH_START		0x0040
#define SPI_FRF_SPI		0x0
#define SPI_DFS_MASK		0x000F
#define SPI_DFS_8BIT		0x0007
#define SPI_DFS_16BIT		0x000F


/* RISR filed */
#define SPI_RISR_RXFIR		0x10
#define SPI_RISR_RXOIR		0x08
#define SPI_RISR_RXUIR		0x04
#define SPI_RISR_TXOIR		0x02
#define SPI_RISR_TXEIR		0x01

/* DMACR field */
#define SPI_DMA_TDMAE_ON	0x01
#define SPI_DMA_TDMAE_OFF	0x00
#define SPI_DMA_RDMAE_ON	0x02
#define SPI_DMA_RDMAE_OFF	0x00

/* CTRL2 filed*/
#define CVI_CS_SS_OW_EN		(0x1 << 0)
#define CVI_SS_INV_EN		(0x1 << 1)
#define CVI_LAB_FIRST		(0x1 << 2)
#define CVI_CS_TOG_OW_EN	(0x1 << 3)
#define CVI_CS_TOG_EN		(0x1 << 4)
#define CVI_SS_MD1_EN		(0x1 << 5)
#define CVI_RX_FULL_STOP_EN	(0x1 << 6)

/* CS_CTRL2 filed*/
#define CVI_SS_N_ACTIVE		(0x1)
#define CVI_SS_N_INACTIVE	(0x0)

#define SPI_BAUDR_DIV		0x32

#endif  /* _TESTCASE_SPI_H_ */
