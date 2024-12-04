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
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/I3cBusLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/RealTimeClockLib.h>
#include <Library/SerialPortLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/RealTimeClock.h>

#include <Guid/GlobalVariable.h>
#include <Guid/EventGroup.h>

//
//RTC related
//
#define CONFIG_RTC_DS1337   0x68
#define CONFIG_RTC_DS1339
#define CONFIG_RTC_SD3068   0x32

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

/*
 * RTC control register bits
 */
#define RTC_CTL_BIT_A1IE   0x1  /* Alarm 1 interrupt enable */
#define RTC_CTL_BIT_A2IE   0x2  /* Alarm 2 interrupt enable */
#define RTC_CTL_BIT_INTCN  0x4  /* Interrupt control */
#define RTC_CTL_BIT_RS1    0x8  /* Rate select 1 */
#define RTC_CTL_BIT_RS2    0x10 /* Rate select 2 */
#define RTC_CTL_BIT_DOSC   0x80 /* Disable Oscillator */

/*
 * RTC status register bits
 */
#define RTC_STAT_BIT_A1F  0x1  /* Alarm 1 flag */
#define RTC_STAT_BIT_A2F  0x2  /* Alarm 2 flag */
#define RTC_STAT_BIT_OSF  0x80 /* Oscillator stop flag */

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
