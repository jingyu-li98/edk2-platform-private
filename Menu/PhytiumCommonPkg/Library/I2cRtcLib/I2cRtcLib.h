/** @file
  RTC operation interfaces.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef I2C_RTC_LIB_H_
#define I2C_RTC_LIB_H_

#include <Uefi.h>
#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/RealTimeClockLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/SerialPortLib.h>
#include <Library/I2cLib.h>

#include <Protocol/RealTimeClock.h>

#include <Guid/GlobalVariable.h>
#include <Guid/EventGroup.h>

//
//RTC related
//
#define CONFIG_RTC_DS1337   0x32   //sunrui change 0x68->0X32 
//#define CONFIG_RTC_DS1339        sunrui -
//#define CONFIG_RTC_SD3068   0x32 sunrui -

//
//RTC register addresses
//
#if defined CONFIG_RTC_DS1337
#define RTC_SEC_REG_ADDR         0x0
#define RTC_MIN_REG_ADDR         0x1
#define RTC_HR_REG_ADDR          0x2
#define RTC_DAY_REG_ADDR         0x3
#define RTC_DATE_REG_ADDR        0x4
#define RTC_MON_REG_ADDR         0x5
#define RTC_YR_REG_ADDR          0x6
#define RTC_A1M1_SECONDS_REG     0x7
#define RTC_A1M2_MINUTES_REG     0x8
#define RTC_A1M3_HOURS_REG       0x9
#define RTC_A1M4_DAYS_REG        0xA
#define RTC_3068_DAYS_REG        0xB
#define RTC_CTL_REG_ADDR         0x0e
#define RTC_STAT_REG_ADDR        0x0f
#define RTC_TC_REG_ADDR          0x10
#elif defined CONFIG_RTC_DS1388
#define RTC_SEC_REG_ADDR         0x1
#define RTC_MIN_REG_ADDR         0x2
#define RTC_HR_REG_ADDR          0x3
#define RTC_DAY_REG_ADDR         0x4
#define RTC_DATE_REG_ADDR        0x5
#define RTC_MON_REG_ADDR         0x6
#define RTC_YR_REG_ADDR          0x7
#define RTC_CTL_REG_ADDR         0x0c
#define RTC_STAT_REG_ADDR        0x0b
#define RTC_TC_REG_ADDR          0x0a
#endif

//
//RTC control register bits
//
#define RTC_CTL_BIT_A1IE         0x1    /* Alarm 1 interrupt enable */
#define RTC_CTL_BIT_A2IE         0x2    /* Alarm 2 interrupt enable */
#define RTC_CTL_BIT_INTCN        0x4    /* Interrupt control    */
#define RTC_CTL_BIT_RS1          0x8    /* Rate select 1    */
#define RTC_CTL_BIT_RS2          0x10   /* Rate select 2    */
#define RTC_CTL_BIT_DOSC         0x80   /* Disable Oscillator   */

//
//RTC status register bits
//
#define RTC_STAT_BIT_A1F         0x1   /* Alarm 1 flag     */
#define RTC_STAT_BIT_A2F         0x2   /* Alarm 2 flag     */
#define RTC_STAT_BIT_OSF         0x80  /* Oscillator stop flag   */


#define DW_IC_CON                0x0     /* i2c control register */
#define IC_CON_SD                0x0040
#define IC_CON_RE                0x0020
#define IC_CON_10BITADDRMASTER   0x0010
#define IC_CON_10BITADDR_SLAVE   0x0008
#define IC_CON_SPD_MSK           0x0006
#define IC_CON_SPD_SS            0x0002
#define IC_CON_SPD_FS            0x0004
#define IC_CON_SPD_HS            0x0006
#define IC_CON_MM                0x0001
#define DW_IC_TAR                0x4     /* i2c target address register */
#define DW_IC_SAR                0x8     /* i2c slave address register */
#define DW_IC_DATA_CMD           0x10    /* i2c data buffer and command register */
#define IC_CMD                   0x0100
#define IC_STOP                  0x0200
#define DW_IC_SS_SCL_HCNT        0x14
#define DW_IC_SS_SCL_LCNT        0x18
#define DW_IC_FS_SCL_HCNT        0x1c
#define DW_IC_FS_SCL_LCNT        0x20
#define DW_IC_HS_SCL_HCNT        0x24
#define DW_IC_HS_SCL_LCNT        0x28
#define DW_IC_INTR_STAT          0x2c    /* i2c interrupt status register */
#define IC_GEN_CALL              0x0800
#define IC_START_DET             0x0400
#define IC_STOP_DET              0x0200
#define IC_ACTIVITY              0x0100
#define IC_RX_DONE               0x0080
#define IC_TX_ABRT               0x0040
#define IC_RD_REQ                0x0020
#define IC_TX_EMPTY              0x0010
#define IC_TX_OVER               0x0008
#define IC_RX_FULL               0x0004
#define IC_RX_OVER               0x0002
#define IC_RX_UNDER              0x0001
#define DW_IC_INTR_MASK          0x30
#define DW_IC_RAW_INTR_STAT      0x34
#define DW_IC_RX_TL              0x38  /* fifo receive threshold register */
#define DW_IC_TX_TL              0x3c  /* fifo transmit threshold register */
#define IC_TL0                   0x00
#define IC_TL1                   0x01
#define IC_TL2                   0x02
#define IC_TL3                   0x03
#define IC_TL4                   0x04
#define IC_TL5                   0x05
#define IC_TL6                   0x06
#define IC_TL7                   0x07
#define IC_RX_TL                 IC_TL0
#define IC_TX_TL                 IC_TL0
#define DW_IC_CLR_INTR           0x40
#define DW_IC_CLR_RX_UNDER       0x44
#define DW_IC_CLR_RX_OVER        0x48
#define DW_IC_CLR_TX_OVER        0x4c
#define DW_IC_CLR_RD_REQ         0x50
#define DW_IC_CLR_TX_ABRT        0x54
#define DW_IC_CLR_RX_DONE        0x58
#define DW_IC_CLR_ACTIVITY       0x5c
#define DW_IC_CLR_STOP_DET       0x60
#define DW_IC_CLR_START_DET      0x64
#define DW_IC_CLR_GEN_CALL       0x68
#define DW_IC_ENABLE             0x6c  /* i2c enable register */
#define IC_ENABLE_0B             0x0001
#define DW_IC_STATUS             0x70  /* i2c status register */
#define IC_STATUS_SA             0x0040
#define IC_STATUS_MA             0x0020
#define IC_STATUS_RFF            0x0010
#define IC_STATUS_RFNE           0x0008
#define IC_STATUS_TFE            0x0004
#define IC_STATUS_TFNF           0x0002
#define IC_STATUS_ACT            0x0001
#define DW_IC_TXFLR              0x74
#define DW_IC_RXFLR              0x78
#define DW_IC_SDA_HOLD           0x7c
#define DW_IC_TX_ABRT_SOURCE     0x80
#define DW_IC_ENABLE_STATUS      0x9c
#define DW_IC_COMP_PARAM_1       0xf4
#define DW_IC_COMP_VERSION       0xf8
#define DW_IC_COMP_TYPE          0xfc

//
//Worst case timeout for 1 byte is kept as 2ms
//
#define I2C_BYTE_TO            (2)
#define I2C_STOPDET_TO         (2)
#define I2C_BYTE_TO_BB         (I2C_BYTE_TO * 16)

//
//Speed Selection
//
#define I2C_MAX_SPEED          3400000
#define I2C_FAST_SPEED         400000
#define I2C_STANDARD_SPEED     100000
#define IC_CLK_FREQ            (50 * 1000 * 1000)

typedef struct {
  UINT16 TmSec;
  UINT16 TmMin;
  UINT16 TmHour;
  UINT16 TmMday;
  UINT16 TmMon;
  UINT16 TmYear;
  UINT16 TmWday;
  UINT16 TmYday;
  UINT16 TmIsdst;
} RTC_TIME;

/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param[out]  Time              A pointer to storage to receive a snapshot of the current time.
  @param[out]  Capabilities      An optional pointer to a buffer to receive the real time clock
                                 device's capabilities.
  @param[in]   Slaveaddress      Rtc device i2c slave address.

  @retval EFI_SUCCESS            The operation completed successfully.
  @retval EFI_INVALID_PARAMETER  Time is NULL.

**/
EFI_STATUS
EFIAPI
GetRtcTime (
  OUT EFI_TIME                  *Time,
  OUT EFI_TIME_CAPABILITIES     *Capabilities,
  IN  UINT32                    Slaveaddress
  );

/**
  Sets the current local time and date information.

  @param[in]  Time              A pointer to the current time.
  @param[in]  Slaveaddress      Rtc device i2c slave address.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.

**/
EFI_STATUS
EFIAPI
SetRtcTime (
  IN EFI_TIME    *Time,
  IN UINT32       Slaveaddress
  );

/**
  Sets the system wakeup alarm clock time.

  @param[in]   Enabled          Enable or disable the wakeup alarm.
  @param[out]  Time             If Enable is TRUE, the time to set the wakeup alarm for.
  @param[in]   Slaveaddress     Rtc device i2c slave address.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was enabled. If
                                Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.

**/
EFI_STATUS
EFIAPI
SetRtcWakeUpTime (
  IN  BOOLEAN     Enabled,
  OUT EFI_TIME    *Time,
  IN  UINT32      Slaveaddress
  );

/**
  Returns the current wakeup alarm clock setting.

  @param[out]  Enabled          Indicates if the alarm is currently enabled or disabled.
  @param[out]  Pending          Indicates if the alarm signal is pending and requires acknowledgement.
  @param[out]  Time             The current alarm setting.
  @param[in]   Slaveaddress     Rtc device i2c slave address.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.

**/
EFI_STATUS
EFIAPI
GetRtcWakeUpTime (
  OUT BOOLEAN        *Enabled,
  OUT BOOLEAN        *Pending,
  OUT EFI_TIME       *Time,
  IN UINT32           Slaveaddress
  );

#endif /* I2C_RTC_LIB_H_ */
