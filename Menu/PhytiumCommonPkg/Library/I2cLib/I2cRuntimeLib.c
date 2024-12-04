/** @file
  I2C Runtime operation interfaces.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/TimerLib.h>

#include "I2cLibInternal.h"

STATIC EFI_EVENT  mI2cRuntimeVirtualAddrChangeEvent;
STATIC UINTN      mI2cBaseAddress;

/**
  Enable I2C.

  @param[in]    Info    A pointer to I2C_INFO.

**/
STATIC
VOID
I2cEnable (
  IN I2C_INFO  *Info
  )
{
  UINT32 Enbl;

  //
  //Enable i2c
  //
  Enbl = MmioRead32 (mI2cBaseAddress + IC_ENABLE);
  Enbl |= IC_ENABLE_0B;
  MmioWrite32 (mI2cBaseAddress + IC_ENABLE, Enbl);
}

/**
  Disable I2C.

  @param[in]    Info    A pointer to I2C_INFO.

**/
STATIC
VOID
I2cDisable (
  IN I2C_INFO  *Info
  )
{
  UINT32 Enbl;

  //
  //Disable i2c
  //
  Enbl = MmioRead32 (mI2cBaseAddress + IC_ENABLE);
  Enbl &= ~IC_ENABLE_0B;
  MmioWrite32 (mI2cBaseAddress + IC_ENABLE, Enbl);
}

/**
  Set i2c speed, standard, high or fast. Config the high and low level duration
  according to the speed.

  @param[in]    Info    A pointer to I2C_INFO.

**/
STATIC
VOID
SetSpeed (
  IN I2C_INFO  *Info
  )
{
  UINT32  Cntl;
  UINT32  Hcnt;
  UINT32  Lcnt;
  UINT32  Enbl;
  UINT32  Spklen;

  //
  //to set speed cltr must be disabled
  //
  Enbl = MmioRead32 (mI2cBaseAddress + IC_ENABLE);
  Enbl &= ~IC_ENABLE_0B;
  MmioWrite32 (mI2cBaseAddress + IC_ENABLE, Enbl);

  Cntl = (MmioRead32 (mI2cBaseAddress + IC_CON) & (~IC_CON_SPD_MSK));

  switch (Info->Speed) {
  case I2C_MAX_SPEED:
    Cntl |= IC_CON_SPD_HS;
    Spklen = PcdGet32 (PcdI2cHsSpkLen);
    Hcnt   = PcdGet32 (PcdI2cHsSclHcnt);
    Lcnt   = PcdGet32 (PcdI2cHsSclLcnt);
    MmioWrite32 (mI2cBaseAddress + IC_HS_SCL_HCNT, Hcnt);
    MmioWrite32 (mI2cBaseAddress + IC_HS_SCL_LCNT, Lcnt);
    MmioWrite32 (mI2cBaseAddress + IC_HS_SPKLEN, Spklen);
    break;
  case I2C_STANDARD_SPEED:
    Cntl |= IC_CON_SPD_SS;
    Spklen = PcdGet32 (PcdI2cFsSpkLen);
    Hcnt   = PcdGet32 (PcdI2cSsSclHcnt);
    Lcnt   = PcdGet32 (PcdI2cSsSclLcnt);
    MmioWrite32 (mI2cBaseAddress + IC_SS_SCL_HCNT, Hcnt);
    MmioWrite32 (mI2cBaseAddress + IC_SS_SCL_LCNT, Lcnt);
    MmioWrite32 (mI2cBaseAddress + IC_FS_SPKLEN, Spklen);
    break;
  case I2C_FAST_SPEED:
  default:
    Cntl |= IC_CON_SPD_FS;
    Spklen = PcdGet32 (PcdI2cFsSpkLen);
    Hcnt   = PcdGet32 (PcdI2cFsSclHcnt);
    Lcnt   = PcdGet32 (PcdI2cFsSclLcnt);
    MmioWrite32 (mI2cBaseAddress + IC_FS_SCL_HCNT, Hcnt);
    MmioWrite32 (mI2cBaseAddress + IC_FS_SCL_LCNT, Lcnt);
    MmioWrite32 (mI2cBaseAddress + IC_FS_SPKLEN, Spklen);
    break;
  }

  MmioWrite32 (mI2cBaseAddress + IC_CON, Cntl);
  //
  //Enable back i2c now speed set
  //
  Enbl |= IC_ENABLE_0B;
  MmioWrite32 (mI2cBaseAddress + IC_ENABLE, Enbl);
}

/**
  Fix and set i2c speed to bus, standard, high or fast.

  @param[in]    Info    A pointer to I2C_INFO.

  @retval     EFI_SCEESSS    Success.
**/
EFI_STATUS
I2cSetBusSpeed (
  IN I2C_INFO  *Info
  )
{
  if (Info->Speed >= I2C_MAX_SPEED) {
    Info->Speed = I2C_MAX_SPEED;
    SetSpeed (Info);
  } else if (Info->Speed >= I2C_FAST_SPEED) {
    Info->Speed = I2C_FAST_SPEED;
    SetSpeed (Info);
  } else {
    Info->Speed = I2C_STANDARD_SPEED;
    SetSpeed(Info);
  }

  return EFI_SUCCESS;
}

/**
  Get i2c bus speed, standard, fast or high.

  @param[in]    Info    A pointer to I2C_INFO.

  @retval    I2C_MaX_SPEED - 3.4M.
             I2C_STANDARD_SPEED - 100K.
             I2C_FAST_SPEED - 100K.
**/
UINT32
I2cGetBusSpeed (
  IN I2C_INFO  *Info
  )
{
  UINT32 Cntl;

  Cntl = (MmioRead32 (mI2cBaseAddress + IC_CON) & IC_CON_SPD_MSK);

  if (Cntl == IC_CON_SPD_HS) {
    return I2C_MAX_SPEED;
  } else if (Cntl == IC_CON_SPD_FS) {
    return I2C_FAST_SPEED;
  } else if (Cntl == IC_CON_SPD_SS) {
    return I2C_STANDARD_SPEED;
  }

  return 0;
}

/**
  Set i2c slave device address to controller.

  @param[in]    Info    A pointer to I2C_INFO.

**/
STATIC
VOID
I2cSetaddress (
  IN I2C_INFO  *Info
  )
{
  MmioWrite32 (mI2cBaseAddress + IC_TAR, Info->SlaveAddress);
}

/**
  Flushes the i2c RX FIFO.

  @param[in]    Info    A pointer to I2C_INFO.

**/
STATIC
VOID
I2cFlushRxfifo (
  IN I2C_INFO  *Info
  )
{
  while (MmioRead32 (mI2cBaseAddress + IC_STATUS) & IC_STATUS_RFNE) {
    MmioRead32 (mI2cBaseAddress + IC_DATA_CMD);
  }
}

/**
  Waits for bus busy.

  @param[in]    Info    A pointer to I2C_INFO.

  @retval  None.
**/
STATIC
EFI_STATUS
I2cWaitForBb (
  IN I2C_INFO  *Info
  )
{
  UINT32  Timeout;

  Timeout = I2C_BYTE_TIMEOUT;

  while ((MmioRead32 (mI2cBaseAddress + IC_STATUS) & IC_STATUS_MA) ||
         !(MmioRead32 (mI2cBaseAddress + IC_STATUS) & IC_STATUS_TFE)) {
    //
    //Evaluate timeout
    //
    MicroSecondDelay (I2C_WAIT_TIME);
    if (Timeout-- == 0) {
      return EFI_TIMEOUT;
    }
  }

  return EFI_SUCCESS;
}

/**
  Check parameters for I2cRead and I2cWrite.

  @param[in]  Addr    Slave address.
  @param[in]  Alen    Slave address length.
  @param[in]  Buffer  Data to write.
  @param[in]  Len     Length of data to write.

  @retval     0       Success.
              Other   Failed.
**/
STATIC
EFI_STATUS
CheckParams (
  IN UINT32 Addr,
  IN UINT32 Alen,
  IN UINT8  *Buffer,
  IN UINT32 Len
  )
{
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Alen > 1) {
    return EFI_INVALID_PARAMETER;
  }

  if (Addr + Len > 256) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

/**
  Transfer init.

  @param[in] Info  A pointer to I2C_INFO.
  @param[in] Addr  Address to read from.
  @param[in] Alen  Address length.

  @retval    0     Success.
**/
STATIC
EFI_STATUS
I2cXferInit (
  IN I2C_INFO  *Info,
  IN UINT32    Addr,
  IN UINT32    Alen
  )
{
  EFI_STATUS  Status;
  UINT8       Index;

  Status = I2cWaitForBb (Info);
  if (Status) {
    return EFI_TIMEOUT;
  }

  I2cDisable (Info);
  I2cSetaddress (Info);
  I2cEnable (Info);
  /// REG addr
  for (Index = Alen; Index > 0; Index--){
    Addr = (Addr >> (Index-1) * 8) & 0xFF;
    MmioWrite32 (mI2cBaseAddress + IC_DATA_CMD, Addr);
  }

  return EFI_SUCCESS;
}

/**
  I2c transfer finish.

  @param[in] Info  A pointer to I2C_INFO.

  @retval  None.
**/
STATIC
EFI_STATUS
I2cXferFinish (
  I2C_INFO  *Info
  )
{
  EFI_STATUS  Status;
  UINTN       Timeout;

  Timeout = I2C_STOPDET_TIMEOUT;

  while (1) {
    if ((MmioRead32 (mI2cBaseAddress + IC_RAW_INTR_STAT) & IC_STOP_DET)) {
      MmioRead32 (mI2cBaseAddress + IC_CLR_STOP_DET);
      break;
    } else {
      MicroSecondDelay (I2C_WAIT_TIME);
      if (Timeout-- == 0) {
        break;
      }
    }
  }

  Status = I2cWaitForBb (Info);
  if (Status) {
    return EFI_TIMEOUT;
  }

  I2cFlushRxfifo (Info);

  /* Wait for read/write operation to complete on actual memory */
  MicroSecondDelay (I2C_FLUSH_WAIT_TIME);

  return EFI_SUCCESS;
}

/**
  I2c read data.

  @param[in]     Info    A pointer to I2C_INFO.
  @param[in]     Addr    Address to read from
  @param[in]     Alen    Address length.
  @param[in,out] Buffer  Buffer for read data
  @param[in]     Len     Number of bytes to be read

  @retval EFI_SUCCESS    Success.
  @retval EFI_TIMEOUT    Error happened,bus time out.

**/
EFI_STATUS
I2cRead (
  IN I2C_INFO   *Info,
  IN UINT64     Addr,
  IN UINT32     Alen,
  IN OUT UINT8  *Buffer,
  IN UINT32     Len
  )
{
  EFI_STATUS  Status;
  UINT32      Timeout;

  Status = I2cXferInit(Info, Addr , Alen);
  if (Status) {
    return EFI_TIMEOUT;
  }

  Timeout = I2C_BYTE_TIMEOUT;
  while (Len != 0) {
    if (Len == 1) {
      MmioWrite32 (mI2cBaseAddress + IC_DATA_CMD, IC_CMD | IC_STOP);
    } else {
      MmioWrite32 (mI2cBaseAddress + IC_DATA_CMD, IC_CMD);
    }
    if (MmioRead32(mI2cBaseAddress + IC_STATUS) & IC_STATUS_RFNE) {

      *Buffer  = (UINT8) MmioRead32 (mI2cBaseAddress + IC_DATA_CMD);
      Len--;
      Buffer++;
      Timeout = I2C_BYTE_TIMEOUT;
    } else {
      MicroSecondDelay(I2C_WAIT_TIME);
      if (Timeout-- == 0) {
        return 1;
      }
      if (MmioRead32 (mI2cBaseAddress + IC_STATUS) & IC_STATUS_RFNE) {
           *Buffer = (UINT8) MmioRead32 (mI2cBaseAddress + IC_DATA_CMD);
          Len--;
          Buffer ++;
          Timeout = I2C_BYTE_TIMEOUT;
      }
    }
  }
  return I2cXferFinish (Info);
}

/**
  Set SPD page.

  @param[in] Info     A pointer to I2C_INFO.
  @param[in] PageNum  The number of Page.

**/
VOID
SpdSetpage (
  IN I2C_INFO  *Info,
  IN UINT32    PageNum
  )
{
  I2cEnable (Info);

  if (PageNum == 0x1) {
    MmioWrite32 (mI2cBaseAddress + IC_TAR, 0x37);
   } else {
     MmioWrite32 (mI2cBaseAddress + IC_TAR, 0x36);
   }

  MmioWrite32 (mI2cBaseAddress + IC_DATA_CMD, 0x0);
  MmioWrite32 (mI2cBaseAddress + IC_DATA_CMD, 0x0|IC_STOP);

  /*******************For NoAck Response************************/

  while (MmioRead32 (mI2cBaseAddress + IC_STATUS) != 0x6);

  if (MmioRead32 (mI2cBaseAddress +IC_INTR_STAT) & 0x40) {
    volatile UINT32 Index;
    for (Index = 0; Index < 100; Index++);
    MmioRead32(mI2cBaseAddress + IC_INTR_STAT);
    MmioRead32(mI2cBaseAddress + IC_STATUS);
    MmioRead32(mI2cBaseAddress + IC_CLR_TX_ABRT);
    MmioRead32(mI2cBaseAddress + IC_INTR_STAT);
    MmioRead32(mI2cBaseAddress + IC_STATUS);
  }

}

/**
  Rtc i2c write data.

  @param[in] Info     A pointer to I2C_INFO.
  @param[in] Addr     Address to read from.
  @param[in] Alen     Address length.
  @param[in] Buffer   Buffer for read data.
  @param[in] Len      Number of bytes to be read.

  @retval EFI_SUCCESS    Success.
  @retval EFI_TIMEOUT    Error happened,bus time out.

**/
EFI_STATUS
I2cWrite (
  IN I2C_INFO  *Info,
  IN UINT64    Addr,
  IN UINT32    Alen,
  IN UINT8     *Buffer,
  IN UINT32    Len
  )
{
  EFI_STATUS  Status;
  UINT32      Nb;
  UINT32      Timeout;
  UINT32      Cnt;

  Nb = Len;
  Cnt = 0;

  Status = CheckParams (Addr, Alen, Buffer, Len);
  if (Status)
    return EFI_INVALID_PARAMETER;

  Status = I2cXferInit (Info, Addr, Alen);
  if (Status) {
    return EFI_TIMEOUT;
  }

  Timeout = Nb * I2C_BYTE_TIMEOUT;
  while (Len != 0) {
    if (MmioRead32 (mI2cBaseAddress + IC_STATUS) & IC_STATUS_TFNF) {
      if (Len > 1){
        if(((Cnt + 1) % 8 == 0)) {
          MmioWrite32(mI2cBaseAddress + IC_DATA_CMD, *Buffer | IC_STOP);  //send stop
          MicroSecondDelay (100000);
        } else if (Cnt !=0 && Cnt % 8 == 0) {
          I2cXferInit (Info, Addr + Cnt, Alen);
          if (MmioRead32(mI2cBaseAddress + IC_STATUS) & IC_STATUS_TFNF) {
            MmioWrite32 (mI2cBaseAddress + IC_DATA_CMD, *Buffer);
          }
        } else {
          MmioWrite32 (mI2cBaseAddress + IC_DATA_CMD, *Buffer);
        }
      } else {
        if (Cnt !=0 && Cnt % 8 == 0) {
          I2cXferInit (Info, Addr + Cnt, Alen);
          if (MmioRead32 (mI2cBaseAddress + IC_STATUS) & IC_STATUS_TFNF) {
            MmioWrite32 (mI2cBaseAddress + IC_DATA_CMD, *Buffer| IC_STOP);
          }
        } else {
          MmioWrite32 (mI2cBaseAddress + IC_DATA_CMD, *Buffer| IC_STOP);
        }
      }
      Buffer++;
      Cnt++;
      Len--;
      Timeout = Nb * I2C_BYTE_TIMEOUT;
    } else {
      MicroSecondDelay (I2C_WAIT_TIME);
      if (Timeout-- == 0) {
        return EFI_TIMEOUT;
      }
    }
  }
  return I2cXferFinish (Info);
}

/**
  I2c controller init.

  @param[in] Info     A pointer to I2C_INFO.

**/
VOID
I2cInit (
  IN I2C_INFO  *Info
  )
{

  /* Disable i2c */
  I2cDisable (Info);

  MmioWrite32 (mI2cBaseAddress + IC_CON, IC_CON_SD | IC_CON_RE | IC_CON_MM);

  MmioWrite32 (mI2cBaseAddress + IC_RX_TL, IC_RX_TL);
  MmioWrite32 (mI2cBaseAddress + IC_TX_TL, IC_TX_TL);

  I2cSetBusSpeed (Info);

  MmioWrite32 (mI2cBaseAddress + IC_INTR_MASK, IC_STOP_DET);

  MmioWrite32 (mI2cBaseAddress + IC_SAR, Info->SlaveAddress);

  I2cEnable (Info);
}

/**
  Config I2c as slave mode.

  @param[in] Info     A pointer to I2C_INFO.

  @retval    EFI_SUCCESS    Success.
**/
EFI_STATUS
EnableI2cAsSlaver (
  IN I2C_INFO  *Info
  )
{
  I2cDisable (Info);
  //
  //set slave address
  //
  MmioWrite32 (mI2cBaseAddress + IC_SAR, Info->SlaveAddress);
  //
  //config slave mode
  //
  MmioWrite32 (mI2cBaseAddress + IC_CON, 0);
  //
  //set speed
  //
  SetSpeed (Info);
  I2cEnable (Info);

  return EFI_SUCCESS;
}

/**
  Fixup internal data so that EFI can be call in virtual mode.
  Call the passed in Child Notify event and convert any pointers in
  lib to virtual mode.

  @param[in]    Event   The Event that is being processed
  @param[in]    Context Event Context

**/
VOID
EFIAPI
LibI2cRuntimeVirtualNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  //
  // Only needed if you are going to support the OS calling RTC functions in virtual mode.
  // You will need to call EfiConvertPointer (). To convert any stored physical addresses
  // to virtual address. After the OS transitions to calling in virtual mode, all future
  // runtime calls will be made in virtual mode.
  //

  EfiConvertPointer (0x0, (VOID**)&mI2cBaseAddress);

  return;
}


/**
  This is the declaration of an EFI image entry point. This can be the entry point to an application
  written to this specification, an EFI boot service driver, or an EFI runtime driver.

  @param  ImageHandle           Handle that identifies the loaded image.
  @param  SystemTable           System Table for this image.

  @retval EFI_SUCCESS           The operation completed successfully.

**/
EFI_STATUS
EFIAPI
I2cRuntimeLibConstruct (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS       Status;

  // Initialize I2c Runtime Base Address
  mI2cBaseAddress   = PcdGet64 (PcdI2cRuntimeBaseAddress);

  //
  // Declare the controller as EFI_MEMORY_RUNTIME
  //
  Status = gDS->AddMemorySpace (
                  EfiGcdMemoryTypeMemoryMappedIo,
                  mI2cBaseAddress,
                  SIZE_4KB,
                  EFI_MEMORY_UC | EFI_MEMORY_RUNTIME
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gDS->SetMemorySpaceAttributes (
                  mI2cBaseAddress,
                  SIZE_4KB, EFI_MEMORY_UC | EFI_MEMORY_RUNTIME
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Register for the virtual address change event
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  LibI2cRuntimeVirtualNotifyEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &mI2cRuntimeVirtualAddrChangeEvent
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
