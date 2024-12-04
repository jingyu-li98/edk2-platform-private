/** @file
  RTC operation interfaces.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <I3cRtcLib.h>

STATIC I3C_BUS_INFO  Bus;
STATIC UINT32        SlaveAddress;
/**
  BCD to BIN.

  @param[in]  Val    Bcd data.

  @retval     BIN
**/
UINT32
Bcd2bin (
  IN UINT8 Val
  )
{
  return ((Val) & 0x0f) + ((Val) >> 4) * 10;
}

/**
  BIN to BCD.

  @param[in]  Val    BIN

  @retval     BCD
**/
UINT8
Bin2bcd (
  IN UINT32 Val
  )
{
  return (((Val / 10) << 4) | (Val % 10));
}

/**
  Rtc register read.

  @param[in]  Reg        Register to read.
  @param[in]  SlaveAddr  I2c slave address.

  @retval     Buf        Register value.
**/
UINT8
RtcRead (
  IN UINT16  Reg,
  IN UINT32  SlaveAddr
  )
{
  UINT8 Buf;

  I3cRead(&Bus, SlaveAddr, Reg, &Buf, 1);

  return Buf;
}

/**
  Rtc register write.

  @param[in]  Reg        Register to write.
  @param[in]  Val        Data to write.
  @param[in]  SlaveAddr  I2c slave address.

**/
VOID
RtcWrite (
  IN UINT16  Reg,
  IN UINT8   Val,
  IN UINT32  SlaveAddr
  )
{
  I3cWrite(&Bus, SlaveAddr, Reg, &Val, 1);
}


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
  IN UINT32                      Slaveaddress
  )
{
  EFI_STATUS  Status;
  RTC_TIME    RtcTime;
  UINT8       Sec;
  UINT8       Min;
  UINT8       Hour;
  UINT8       Mday;
  UINT8       MonCent;
  UINT8       Year;
  UINT64      BusAddress;
  UINT32      Speed;

  Status = EFI_SUCCESS;

  if (Time == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto EXIT;
  }

  BusAddress = PcdGet64 (PcdI3cRuntimeBaseAddress);
  Speed = PcdGet32 (PcdRtcI3cControllerSpeed);
  ZeroMem(&Bus, sizeof(Bus));
  I3cInit(&Bus, BusAddress, Speed, I2C_MODE, XmitBurstStaticSubaddr, SubAddress8Bit);

  Sec      = RtcRead (RTC_SEC_REG_ADDR,  Slaveaddress);
  Min      = RtcRead (RTC_MIN_REG_ADDR,  Slaveaddress);
  Hour     = RtcRead (RTC_HR_REG_ADDR,   Slaveaddress);
  Mday     = RtcRead (RTC_DATE_REG_ADDR, Slaveaddress);
  MonCent  = RtcRead (RTC_MON_REG_ADDR,  Slaveaddress);
  Year     = RtcRead (RTC_YR_REG_ADDR,   Slaveaddress);

  RtcTime.TmSec  = Bcd2bin (Sec & 0x7F);
  RtcTime.TmMin  = Bcd2bin (Min & 0x7F);
  RtcTime.TmHour = Bcd2bin (Hour & 0x3F);
  RtcTime.TmMday = Bcd2bin (Mday & 0x3F);
  RtcTime.TmMon  = Bcd2bin (MonCent & 0x1F);
  RtcTime.TmYear = Bcd2bin (Year) + 2000;
  RtcTime.TmYday = 0;
  RtcTime.TmIsdst= 0;

  Time->Year       = (UINT16)RtcTime.TmYear;
  Time->Month      = (UINT8)RtcTime.TmMon;
  Time->Day        = (UINT8)RtcTime.TmMday;
  Time->Hour       = (UINT8)RtcTime.TmHour;
  Time->Minute     = (UINT8)RtcTime.TmMin;
  Time->Second     = (UINT8)RtcTime.TmSec;
  Time->Nanosecond = 0;

EXIT:
  return Status;
}

/**
  Sets the current local time and date information.

  @param[in]  Time              A pointer to the current time.
  @param[in]  Slaveaddress      Rtc device i2c slave address.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The time could not be set due due to hardware error.

**/
EFI_STATUS
EFIAPI
SetRtcTime (
  IN EFI_TIME              *Time,
  IN UINT32                 Slaveaddress
  )
{
  EFI_STATUS  Status;
  UINT8       Sec;
  UINT8       Min;
  UINT8       Hour;
  UINT8       Mday;
  UINT8       Month;
  UINT8       Year;
  UINT64      BusAddress;
  UINT32      Speed;

  Status = EFI_SUCCESS;

  if (!IsTimeValid (Time)) {
    Status = EFI_INVALID_PARAMETER;
    goto EXIT;
  }

  //
  // Because the PL031 is a 32-bit counter counting seconds,
  // the maximum time span is just over 136 years.
  // Time is stored in Unix Epoch format, so it starts in 1970,
  // Therefore it can not exceed the year 2106.
  //
  if ((Time->Year < 1970) || (Time->Year >= 2106)) {
    Status = EFI_UNSUPPORTED;
    goto EXIT;
  }

  if (Slaveaddress == CONFIG_RTC_DS1337) {
    Sec   = Bin2bcd (Time->Second);
    Min   = Bin2bcd (Time->Minute);
    Hour  = Bin2bcd (Time->Hour);
    Mday  = Bin2bcd (Time->Day);
    Month = Bin2bcd (Time->Month) | 0x80;
    Year  = Bin2bcd (Time->Year % 100);
  } else if(Slaveaddress == CONFIG_RTC_SD3068){
    Sec   = Bin2bcd (Time->Second);
    Min   = Bin2bcd (Time->Minute);
    Hour  = Bin2bcd (Time->Hour) | 0x80;
    Mday  = Bin2bcd (Time->Day);
    Month = Bin2bcd (Time->Month);
    Year  = Bin2bcd (Time->Year % 100);
  }

  BusAddress = PcdGet64 (PcdI3cRuntimeBaseAddress);
  Speed = PcdGet32 (PcdRtcI3cControllerSpeed);
  ZeroMem(&Bus, sizeof(Bus));
  I3cInit(&Bus, BusAddress, Speed, I2C_MODE, XmitBurstStaticSubaddr, SubAddress8Bit);


  RtcWrite (RTC_YR_REG_ADDR, Year, Slaveaddress);
  RtcWrite (RTC_MON_REG_ADDR, Month, Slaveaddress);
  RtcWrite (RTC_DATE_REG_ADDR, Mday, Slaveaddress);
  RtcWrite (RTC_HR_REG_ADDR, Hour, Slaveaddress);
  RtcWrite (RTC_MIN_REG_ADDR, Min, Slaveaddress);
  RtcWrite (RTC_SEC_REG_ADDR, Sec, Slaveaddress);

EXIT:
  return Status;
}

/**
  Sets the system wakeup alarm clock time.

  @param[in]   Enabled          Enable or disable the wakeup alarm.
  @param[out]  Time             If Enable is TRUE, the time to set the wakeup alarm for.
  @param[in]   Slaveaddress     Rtc device i2c slave address.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was enabled. If
                                Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be set due to a hardware error.

**/
EFI_STATUS
EFIAPI
SetRtcWakeUpTime (
  IN BOOLEAN     Enabled,
  OUT EFI_TIME   *Time,
  IN UINT32      Slaveaddress
  )
{
  EFI_STATUS         Status;
  UINT8              Temp;
  Temp   = 0;
  Status = EFI_SUCCESS;

  if (Slaveaddress == CONFIG_RTC_DS1337){
    //
    //disable alarm
    //
    if (!Enabled) {
      Temp = RtcRead (RTC_CTL_REG_ADDR, Slaveaddress);
      Temp &= ~(1 << 0);
      RtcWrite (RTC_CTL_REG_ADDR, Temp, Slaveaddress);
      return EFI_SUCCESS;
    }

    if (Time == NULL || !IsTimeValid (Time)) {
      return EFI_INVALID_PARAMETER;
    }
    //
    //enable alarm
    //
    RtcWrite (RTC_CTL_REG_ADDR, RTC_CTL_BIT_A1IE | RTC_CTL_BIT_INTCN, Slaveaddress);
    //
    //set alarm time
    //
    RtcWrite (RTC_A1M1_SECONDS_REG, Bin2bcd (Time->Second), Slaveaddress);
    RtcWrite (RTC_A1M2_MINUTES_REG, Bin2bcd (Time->Minute), Slaveaddress);
    RtcWrite (RTC_A1M3_HOURS_REG,   Bin2bcd (Time->Hour), Slaveaddress);
    RtcWrite (RTC_A1M4_DAYS_REG,    Bin2bcd (Time->Day), Slaveaddress);
  } else if (Slaveaddress == CONFIG_RTC_SD3068){
    //
    //disable alarm
    //
    if (!Enabled){
      RtcWrite (RTC_CTL_REG_ADDR, 0x0, Slaveaddress);
      Temp = RtcRead(RTC_TC_REG_ADDR, Slaveaddress);
      Temp &= ~(1 << 1);
      RtcWrite (RTC_TC_REG_ADDR, Temp, Slaveaddress);
      return EFI_SUCCESS;
    }
    if (Time == NULL || !IsTimeValid (Time)) {
      return EFI_INVALID_PARAMETER;
    }
    RtcWrite (RTC_TC_REG_ADDR, 0x92, Slaveaddress);        //10h    1001 0010 IM=0 INTS1=0 INTS0=1 INTAE=1
    RtcWrite (RTC_STAT_REG_ADDR, 0xFF, Slaveaddress);
    RtcWrite (RTC_CTL_REG_ADDR, 0x17, Slaveaddress);        //enable alarm

    RtcWrite (RTC_A1M1_SECONDS_REG, Bin2bcd (Time->Second), Slaveaddress);
    RtcWrite (RTC_A1M2_MINUTES_REG, Bin2bcd (Time->Minute), Slaveaddress);
    RtcWrite (RTC_A1M3_HOURS_REG, Bin2bcd (Time->Hour), Slaveaddress);
    RtcWrite (0x0B, Bin2bcd (Time->Day), Slaveaddress);
    RtcWrite (0x0C, Bin2bcd (Time->Month), Slaveaddress);
    RtcWrite (0x0D, Bin2bcd (Time->Year % 100), Slaveaddress);
  }

  return Status;
}

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
  OUT BOOLEAN    *Enabled,
  OUT BOOLEAN    *Pending,
  OUT EFI_TIME   *Time,
  IN  UINT32     Slaveaddress
  )
{
  EFI_STATUS  Status;
  UINT16      Year;
  UINT8       Mounth;
  UINT8       Second;
  UINT8       Minute;
  UINT8       Hour;
  UINT8       Day;

  Status = EFI_SUCCESS;

  if (Time == NULL || Enabled == NULL || Pending == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Slaveaddress == CONFIG_RTC_DS1337){
    *Enabled = RtcRead (RTC_CTL_REG_ADDR, Slaveaddress) & RTC_CTL_BIT_A1IE;
    *Pending = RtcRead (RTC_STAT_REG_ADDR, Slaveaddress) & RTC_STAT_BIT_A1F;
    if (*Pending) {
      RtcWrite (RTC_STAT_REG_ADDR, RTC_STAT_BIT_A1F,Slaveaddress);
    }
    Day = RtcRead (RTC_A1M4_DAYS_REG, Slaveaddress);
  } else if (Slaveaddress == CONFIG_RTC_SD3068) {
    if (RtcRead (RTC_TC_REG_ADDR, Slaveaddress) & 0x2) {
      *Enabled = 1;
    } else {
      *Enabled = 0;
    }
    if (RtcRead (RTC_STAT_REG_ADDR, Slaveaddress) & 0x20) {
      *Pending = 1;
    } else {
      *Pending = 0;
    }
    if (*Pending) {
      RtcWrite (RTC_STAT_REG_ADDR, 0x20, Slaveaddress);
    }
     Day     = RtcRead (0x0B, Slaveaddress);
     Mounth  = RtcRead (0x0C, Slaveaddress);
     Year    = RtcRead (0x0D, Slaveaddress);
  }

  Second = RtcRead (RTC_A1M1_SECONDS_REG, Slaveaddress);
  Minute = RtcRead (RTC_A1M2_MINUTES_REG, Slaveaddress);
  Hour   = RtcRead (RTC_A1M3_HOURS_REG, Slaveaddress);

  Time->Second  = Bcd2bin (Second & 0x7F);
  Time->Minute  = Bcd2bin (Minute & 0x7F);
  Time->Hour    = Bcd2bin (Hour & 0x3F);
  Time->Day     = Bcd2bin (Day & 0x3F);
  Time->Month   = Bcd2bin (Mounth & 0x1F);
  Time->Year    = Bcd2bin (Year) + 2000;
  Time->Nanosecond = 0;

  return Status;
}


/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param[out]  Time              A pointer to storage to receive a snapshot of the current time.
  @param[out]  Capabilities      An optional pointer to a buffer to receive the real time clock
                                 device's capabilities.

  @retval EFI_SUCCESS            The operation completed successfully.
  @retval EFI_INVALID_PARAMETER  Time is NULL.
  @retval EFI_DEVICE_ERROR       The time could not be retrieved due to hardware error.
  @retval EFI_SECURITY_VIOLATION The time could not be retrieved due to an authentication failure.

**/
EFI_STATUS
EFIAPI
LibGetTime (
  OUT EFI_TIME                *Time,
  OUT EFI_TIME_CAPABILITIES   *Capabilities
  )
{
  EFI_STATUS Status;

  Status = EFI_UNSUPPORTED;
  Status = GetRtcTime (Time, Capabilities, SlaveAddress);

  return Status;
}

/**
  Sets the current local time and date information.

  @param[in]  Time              A pointer to the current time.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The time could not be set due due to hardware error.

**/
EFI_STATUS
EFIAPI
LibSetTime (
  IN  EFI_TIME                *Time
  )
{
  EFI_STATUS Status;

  Status = EFI_UNSUPPORTED;
  Status = SetRtcTime (Time, SlaveAddress);

  return Status;
}

/**
  Returns the current wakeup alarm clock setting.

  @param[out]  Enabled          Indicates if the alarm is currently enabled or disabled.
  @param[out]  Pending          Indicates if the alarm signal is pending and requires acknowledgement.
  @param[out]  Time             The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to a hardware error.

**/
EFI_STATUS
EFIAPI
LibGetWakeupTime (
  OUT BOOLEAN     *Enabled,
  OUT BOOLEAN     *Pending,
  OUT EFI_TIME    *Time
  )
{
  EFI_STATUS  Status;

  Status = EFI_UNSUPPORTED;
  Status = GetRtcWakeUpTime (Enabled, Pending, Time, SlaveAddress);

  return Status;
}


/**
  Sets the system wakeup alarm clock time.

  @param[in]   Enabled          Enable or disable the wakeup alarm.
  @param[out]  Time             If Enable is TRUE, the time to set the wakeup alarm for.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was enabled. If
                                Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be set due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
LibSetWakeupTime (
  IN BOOLEAN      Enabled,
  OUT EFI_TIME    *Time
  )
{
  EFI_STATUS Status;

  Status = EFI_UNSUPPORTED;
  Status = SetRtcWakeUpTime (Enabled, Time, SlaveAddress);

  return Status;
}

/**
  This is the declaration of an EFI image entry point. This can be the entry point to an application
  written to this specification, an EFI boot service driver, or an EFI runtime driver.

  @param[in]  ImageHandle           Handle that identifies the loaded image.
  @param[in]  SystemTable           System Table for this image.

  @retval EFI_SUCCESS           The operation completed successfully.

**/
EFI_STATUS
EFIAPI
LibRtcInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_STATUS       Status;
  EFI_HANDLE       Handle;

  // Initialize RTC Base Address
  SlaveAddress = PcdGet32 (PcdRtcI3cControllerSlaveAddress);

  // Install the protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiRealTimeClockArchProtocolGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
