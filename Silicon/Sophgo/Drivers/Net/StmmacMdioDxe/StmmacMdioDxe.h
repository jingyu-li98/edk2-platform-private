/** @file
 *
 *  STMMAC registers.
 *
 *  Copyright (c) 2024, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef __MDIO_DXE_H__
#define __MDIO_DXE_H__

#include <Uefi.h>

//
// MAC configuration register definitions.
//
#define FRAMEBURSTENABLE        BIT21
#define MII_PORTSELECT          BIT15
#define FES_100                 BIT14
#define DISABLERXOWN            BIT13
#define FULLDPLXMODE            BIT11
#define RXENABLE                BIT2
#define TXENABLE                BIT3

//
// MII address register definitions.
//
#define MII_BUSY                BIT0
#define MII_WRITE               BIT1
#define MII_CLKRANGE_60_100M    (0)
#define MII_CLKRANGE_100_150M   (0x4)
#define MII_CLKRANGE_20_35M     (0x8)
#define MII_CLKRANGE_35_60M     (0xC)
#define MII_CLKRANGE_150_250M   (0x10)
#define MII_CLKRANGE_250_300M   (0x14)

#define MIIADDRSHIFT            (11)
#define MIIREGSHIFT             (6)
#define MII_REGMSK              (0x1F << 6)
#define MII_ADDRMSK             (0x1F << 11)

#define MII_DATA_MASK GENMASK(15, 0)

//
// GMAC4 defines
//
#define MII_GMAC4_GOC_SHIFT             2
#define MII_GMAC4_REG_ADDR_SHIFT        16
#define MII_GMAC4_WRITE                 (1 << MII_GMAC4_GOC_SHIFT)
#define MII_GMAC4_READ                  (3 << MII_GMAC4_GOC_SHIFT)
#define MII_GMAC4_C45E                  BIT(1)

//
// XGMAC defines.
//
#define MII_XGMAC_SADDR                 BIT(18)
#define MII_XGMAC_CMD_SHIFT             16
#define MII_XGMAC_WRITE                 (1 << MII_XGMAC_CMD_SHIFT)
#define MII_XGMAC_READ                  (3 << MII_XGMAC_CMD_SHIFT)
#define MII_XGMAC_BUSY                  BIT(22)
#define MII_XGMAC_MAX_C22ADDR           3
#define MII_XGMAC_C22P_MASK             GENMASK(MII_XGMAC_MAX_C22ADDR, 0)
#define MII_XGMAC_PA_SHIFT              16
#define MII_XGMAC_DA_SHIFT              21

#define MTL_MAX_RX_QUEUES       8
#define MTL_MAX_TX_QUEUES       8
#define STMMAC_CH_MAX           8

#define STMMAC_RX_COE_NONE      0
#define STMMAC_RX_COE_TYPE1     1
#define STMMAC_RX_COE_TYPE2     2

/* Define the macros for CSR clock range parameters to be passed by
 * platform code.
 * This could also be configured at run time using CPU freq framework. */

//
// MDC Clock Selection define.
// MAC_MDIO_ADDRESS.CR[11:8] (CSR Clock Range)
//
#define STMMAC_CSR_60_100M      0x0     /* MDC = clk_scr_i/42 */
#define STMMAC_CSR_100_150M     0x1     /* MDC = clk_scr_i/62 */
#define STMMAC_CSR_20_35M       0x2     /* MDC = clk_scr_i/16 */
#define STMMAC_CSR_35_60M       0x3     /* MDC = clk_scr_i/26 */
#define STMMAC_CSR_150_250M     0x4     /* MDC = clk_scr_i/102 */
#define STMMAC_CSR_250_300M     0x5     /* MDC = clk_scr_i/122 */

//
// MTL algorithms identifiers.
//
#define MTL_TX_ALGORITHM_WRR    0x0
#define MTL_TX_ALGORITHM_WFQ    0x1
#define MTL_TX_ALGORITHM_DWRR   0x2
#define MTL_TX_ALGORITHM_SP     0x3
#define MTL_RX_ALGORITHM_SP     0x4
#define MTL_RX_ALGORITHM_WSP    0x5

//
// RX/TX Queue Mode.
//
#define MTL_QUEUE_AVB           0x0
#define MTL_QUEUE_DCB           0x1

/* The MDC clock could be set higher than the IEEE 802.3
 * specified frequency limit 0f 2.5 MHz, by programming a clock divider
 * of value different than the above defined values. The resultant MDIO
 * clock frequency of 12.5 MHz is applicable for the interfacing chips
 * supporting higher MDC clocks.
 * The MDC clock selection macros need to be defined for MDC clock rate
 * of 12.5 MHz, corresponding to the following selection.
 */
#define STMMAC_CSR_I_4          0x8     /* clk_csr_i/4 */
#define STMMAC_CSR_I_6          0x9     /* clk_csr_i/6 */
#define STMMAC_CSR_I_8          0xA     /* clk_csr_i/8 */
#define STMMAC_CSR_I_10         0xB     /* clk_csr_i/10 */
#define STMMAC_CSR_I_12         0xC     /* clk_csr_i/12 */
#define STMMAC_CSR_I_14         0xD     /* clk_csr_i/14 */
#define STMMAC_CSR_I_16         0xE     /* clk_csr_i/16 */
#define STMMAC_CSR_I_18         0xF     /* clk_csr_i/18 */

//
// AXI DMA Burst length supported.
//
#define DMA_AXI_BLEN_4          (1 << 1)
#define DMA_AXI_BLEN_8          (1 << 2)
#define DMA_AXI_BLEN_16         (1 << 3)
#define DMA_AXI_BLEN_32         (1 << 4)
#define DMA_AXI_BLEN_64         (1 << 5)
#define DMA_AXI_BLEN_128        (1 << 6)
#define DMA_AXI_BLEN_256        (1 << 7)
#define DMA_AXI_BLEN_ALL (DMA_AXI_BLEN_4 | DMA_AXI_BLEN_8 | DMA_AXI_BLEN_16 \
                        | DMA_AXI_BLEN_32 | DMA_AXI_BLEN_64 \
                        | DMA_AXI_BLEN_128 | DMA_AXI_BLEN_256)

#define STMMAC_MDIO_TIMEOUT     10000   // 10000us
					//
#endif // __MDIO_DXE_H__
