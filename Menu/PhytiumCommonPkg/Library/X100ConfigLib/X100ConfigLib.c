/** @file
  X100 config Lib, config the X100 according to the variable.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <AdvancedConfigData.h>

#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/X100Lib.h>

#include <Protocol/PciIo.h>

/**
  Check whether the X100 exists

  @param[in, out]

  @retval  TRUE   X100 exists.
           FALSE  X100 don't exists.

**/
BOOLEAN
EFIAPI
IsX100Existed (
  VOID
  )
{
  UINT32                      Status;
  UINTN                       NumOfPciHandles;
  EFI_HANDLE                  *HandleBuffer;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  UINT32                      Index;
  UINT16                      DeviceID;
  UINT16                      VendorID;

  Status = 0;
  NumOfPciHandles = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &NumOfPciHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }
  DEBUG ((DEBUG_INFO, "Pci handles number is %d\n", NumOfPciHandles));

  for (Index = 0; Index < NumOfPciHandles; Index ++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (!EFI_ERROR (Status)) {
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x0,
                   1,
                   &VendorID
                   );
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x2,
                   1,
                   &DeviceID
                   );
      //DEBUG ((DEBUG_INFO, "Device ID : 0x%04x, Vendor ID : 0x%04x\n", DeviceID, VendorID));
      if ((DeviceID == 0xdc20) && (VendorID == 0x1db7)) {
        return TRUE;
      }
    }
  }

  return FALSE;
}


/**
  Get the Dp channel

  @retval  EFI_SUCCESS    Get the Dp channel successfully.
           EFI_NOT_FOUND  Get the Dp channel failed.

**/
EFI_STATUS
EFIAPI
GetDpChannel (
  VOID
  )
{
  EFI_STATUS                  Status;
  UINTN                       NumOfPciHandles;
  EFI_HANDLE                  *HandleBuffer;
  X100_PCI_INFO               X100PciInfo;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  UINT32                      Index;
  UINT16                      DeviceID;
  UINT16                      VendorID;
  UINT16                      SubsystemID;
  UINT8                       DpChannel;
  UINTN                       VarSize;

  DpChannel = 0;
  NumOfPciHandles = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &NumOfPciHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  DEBUG ((DEBUG_INFO, "Pci handles number is %d\n", NumOfPciHandles));

  for (Index = 0; Index < NumOfPciHandles; Index ++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (!EFI_ERROR (Status)) {
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x0,
                   1,
                   &VendorID
                   );
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x2,
                   1,
                   &DeviceID
                   );
      DEBUG ((DEBUG_INFO, "Device ID : 0x%04x, Vendor ID : 0x%04x\n", DeviceID, VendorID));
      if ((DeviceID == 0xdc22) && (VendorID == 0x1db7)) {
        PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x2E,
                   1,
                   &SubsystemID
                   );
        DEBUG((DEBUG_INFO, "Subsystem ID:0x%x\n", SubsystemID));
        for (Index = 0; Index < 3; Index++) {
            if((SubsystemID >> Index) & 0x1) {
                DpChannel++;
            }
        }
        VarSize = sizeof(X100_PCI_INFO);
        Status = gRT->GetVariable (
                        VAR_X100_PCI_INFO,
                        &gX100PciInfoVarGuid,
                        NULL,
                        &VarSize,
                        &X100PciInfo
                  );
        X100PciInfo.DpChannel = DpChannel;
        Status = gRT->SetVariable (
                        VAR_X100_PCI_INFO,
                        &gX100PciInfoVarGuid,
                        PLATFORM_SETUP_VARIABLE_FLAG,
                        sizeof (X100_PCI_INFO),
                        &X100PciInfo
                  );
        break;
      }
    }
  }
  return Status;
}

/**
  Set Device Power State

  @param [in] PciIo         pionter of EFI_PCI_IO_PROTOCOL
  @param [in] PowerState    PowerState to set.

**/
VOID
SetDevicePowerState (
  EFI_PCI_IO_PROTOCOL  *PciIo,
  UINT8                PowerState
  )
{
  UINT8  Point;
  UINT8  PointNew;
  UINT8  CapId;
  UINT8  Temp;

  PciIo->Pci.Read (
               PciIo,
               EfiPciIoWidthUint8,
               0x34,
               1,
               &PointNew
               );
  while(1) {
    DEBUG ((DEBUG_INFO, "Pci Cap Point : 0x%02x\n", PointNew));
    if (PointNew == 0x0) {
      break;
    }
    Point = PointNew;
    PciIo->Pci.Read (
                 PciIo,
                 EfiPciIoWidthUint8,
                 Point,
                 1,
                 &CapId
                 );
    PciIo->Pci.Read (
                 PciIo,
                 EfiPciIoWidthUint8,
                 Point + 1,
                 1,
                 &PointNew
                 );
    DEBUG ((DEBUG_INFO, "CapId : 0x%02x, point next : 0x%02x\n", CapId, PointNew));
    if (CapId == 0x01) {
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint8,
                   Point + 4,
                   1,
                   &Temp
                   );
      DEBUG ((DEBUG_INFO, "power state : 0x%02x\n", Temp));
      Temp &= 0xFC;
      Temp |= PowerState;
      PciIo->Pci.Write (
                   PciIo,
                   EfiPciIoWidthUint8,
                   Point + 4,
                   1,
                   &Temp
                   );
    }
  }
}

/**
  Check whether the X100 exists

  @param[in] Config   struct of X100_CONFIG
  @param[in] PciIo    pionter of EFI_PCI_IO_PROTOCOL

  @retval  EFI_SUCCESS    config DC according to the X100 config successfully.
           other          X100 don't exists.

**/
EFI_STATUS
SetX100ConfigToDc (
  IN X100_CONFIG          *Config,
  IN EFI_PCI_IO_PROTOCOL  *PciIo
  )
{
  UINT32      Index;
  UINT8       DpUseStatus;
  UINT8       DownSpreadStatus;
  UINT32      Data;
  EFI_STATUS  Status;
  UINT32      Temp;

  Status = EFI_SUCCESS;
  DownSpreadStatus = 0;
  Temp = 0x03;
  PciIo->Pci.Write (
               PciIo,
               EfiPciIoWidthUint8,
               0x4,
               1,
               &Temp
               );
  PciIo->Pci.Read (
               PciIo,
               EfiPciIoWidthUint32,
               0x2C,
               1,
               &Data
               );
  DpUseStatus = (UINT8)(Data >> 16);
  for (Index = 0; Index < 3; Index++) {
    Config->X100DpConfig[Index].IsUsed = (DpUseStatus >> Index) & 0x1;
    DownSpreadStatus |= (Config->X100DpConfig[Index].DownSpreadEnable << Index);
    DEBUG ((DEBUG_INFO, "Config->X100DpConfig[%d].DownSpreadEnable : %x\n", Index, Config->X100DpConfig[Index].DownSpreadEnable));
    DEBUG ((DEBUG_INFO, "Config->X100DpConfig[%d].IsUsed : %x\n", Index, Config->X100DpConfig[Index].IsUsed));
  }
  DEBUG ((DEBUG_INFO, "X100 Dp DownSpread Config : 0x%x\n", DownSpreadStatus));

  Status = PciIo->Mem.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,        //Point to Bar 0(MMIO base address),
                        0x4048,
                        1,
                        &Data
                        );
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_INFO, "Get Dp 4048 reg Failed!\n"));
    return Status;
  }
  DEBUG ((DEBUG_INFO, "Data : %x\n", Data));
  Data &= ~(0x7 << 3);
  DEBUG ((DEBUG_INFO, "Data : %x\n", Data));
  Data |= (DownSpreadStatus << 3);
  DEBUG ((DEBUG_INFO, "Data : %x\n", Data));
  Status = PciIo->Mem.Write (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,      //Point to Bar 0(MMIO base address)
                        0x4048,
                        1,
                        &Data
                        );
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_INFO, "Get Dp DownSpread Ctrl Failed!\n"));
    return Status;
  }
  Temp = 0x0;
  PciIo->Pci.Write (
               PciIo,
               EfiPciIoWidthUint8,
               0x4,
               1,
               &Temp
               );
  return Status;
}

/**
  Enable the devices of X100.

  @retval  EFI_SUCCESS    Enable the devices of X100 successfully.
           other          Enable the devices of X100 failed.

**/
EFI_STATUS
ControlX100DevEnable (
  VOID
)
{
  UINT32                      Status;
  UINTN                       NumOfPciHandles;
  EFI_HANDLE                  *HandleBuffer;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  UINT32                      Index;
  UINT16                      DeviceID;
  UINT16                      VendorID;
  UINTN                       Seg;
  UINTN                       Bus;
  UINTN                       Func;
  UINTN                       Dev;
  X100_CONFIG                 X100ConfigData;
  UINTN                       VarSize;
  X100_PCI_INFO               X100PciInfo;
  UINT32                      Num;
  UINT8                       Temp;
  UINT32                      SataPi;

  ZeroMem (&X100PciInfo, sizeof(X100_PCI_INFO));
  X100PciInfo.UsbSwitch.Enable = 1;
  X100PciInfo.SataSwitch.Enable = 1;
  VarSize = sizeof(X100_CONFIG);
  Status = 0;
  NumOfPciHandles = 0;
  HandleBuffer = NULL;

  Status = gRT->GetVariable (
                  VAR_X100_CONFIG,
                  &gAdvancedX100VarGuid,
                  NULL,
                  &VarSize,
                  &X100ConfigData
                  );
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  for (Num = 0; Num < 4; Num++) {
    X100ConfigData.SataPort[Num].IsUsed = 0;
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &NumOfPciHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  DEBUG ((DEBUG_INFO, "Pci handles number is %d\n", NumOfPciHandles));
  DEBUG ((DEBUG_INFO, "Config->X100DpConfig[0].DownSpreadEnable : %x\n", X100ConfigData.X100DpConfig[0].DownSpreadEnable));
  DEBUG ((DEBUG_INFO, "Config->X100DpConfig[0].IsUsed : %x\n", X100ConfigData.X100DpConfig[0].IsUsed));
  DEBUG ((DEBUG_INFO, "Config->X100DpConfig[1].DownSpreadEnable : %x\n", X100ConfigData.X100DpConfig[1].DownSpreadEnable));
  DEBUG ((DEBUG_INFO, "Config->X100DpConfig[1].IsUsed : %x\n", X100ConfigData.X100DpConfig[1].IsUsed));
  DEBUG ((DEBUG_INFO, "Config->X100DpConfig[2].DownSpreadEnable : %x\n", X100ConfigData.X100DpConfig[2].DownSpreadEnable));
  DEBUG ((DEBUG_INFO, "Config->X100DpConfig[2].IsUsed : %x\n", X100ConfigData.X100DpConfig[2].IsUsed));
  DEBUG ((DEBUG_INFO, "Pcie x2-0 Enable : %d\n", X100ConfigData.PcieX2Dn4Enable));
  DEBUG ((DEBUG_INFO, "Pcie x2-1 Enable : %d\n", X100ConfigData.PcieX2Dn5Enable));
  DEBUG ((DEBUG_INFO, "Pcie x1-0 Enable : %d\n", X100ConfigData.PcieX1Dn6Enable));
  DEBUG ((DEBUG_INFO, "Pcie x1-1 Enable : %d\n", X100ConfigData.PcieX1Dn7Enable));
  DEBUG ((DEBUG_INFO, "Pcie x1-2 Enable : %d\n", X100ConfigData.PcieX1Dn8Enable));
  DEBUG ((DEBUG_INFO, "Pcie x1-3 Enable : %d\n", X100ConfigData.PcieX1Dn9Enable));
  for (Index = 0; Index < NumOfPciHandles; Index ++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (!EFI_ERROR (Status)) {
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x0,
                   1,
                   &VendorID
                   );
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x2,
                   1,
                   &DeviceID
                   );
      //DEBUG ((DEBUG_INFO, "Device ID : 0x%04x, Vendor ID : 0x%04x\n", DeviceID, VendorID));
      if ((DeviceID == 0xdc3a) && (VendorID == 0x1db7)) {
        X100PciInfo.X100IsExisted = 1;
        PciIo->GetLocation (PciIo, &Seg, &Bus, &Dev, &Func);
        //DEBUG  ((DEBUG_INFO, "----------------------\n"));
        //DEBUG  ((DEBUG_INFO, "bus : %d\n", Bus));
        //DEBUG  ((DEBUG_INFO, "device : %d\n", Dev));
        //DEBUG  ((DEBUG_INFO, "function : %d\n", Func));
        //DEBUG  ((DEBUG_INFO, "----------------------\n"));
        switch (Dev) {
        //display
        case 1:
          X100PciInfo.DisplaySwitch.Enable = X100ConfigData.DisplayEnable;
          X100PciInfo.DisplaySwitch.Seg = Seg;
          X100PciInfo.DisplaySwitch.Bus = Bus;
          X100PciInfo.DisplaySwitch.Dev = Dev;
          X100PciInfo.DisplaySwitch.Func = Func;
          if (X100ConfigData.DisplayEnable == 0) {
            SetDevicePowerState (PciIo, 3);
            gBS->Stall (50 * 1000);
          }
          break;
        //usb
        case 2:
          X100PciInfo.UsbSwitch.Enable = X100ConfigData.UsbEnable;
          X100PciInfo.UsbSwitch.Seg = Seg;
          X100PciInfo.UsbSwitch.Bus = Bus;
          X100PciInfo.UsbSwitch.Dev = Dev;
          X100PciInfo.UsbSwitch.Func = Func;
          if (X100ConfigData.UsbEnable == 0) {
            SetDevicePowerState (PciIo, 3);
            gBS->Stall (50 * 1000);
          }
          break;
        //sata
        case 3:
          X100PciInfo.SataSwitch.Enable = X100ConfigData.SataEnable;
          X100PciInfo.SataSwitch.Seg = Seg;
          X100PciInfo.SataSwitch.Bus = Bus;
          X100PciInfo.SataSwitch.Dev = Dev;
          X100PciInfo.SataSwitch.Func = Func;
          if (X100ConfigData.SataEnable == 0) {
            SetDevicePowerState (PciIo, 3);
            gBS->Stall (50 * 1000);
          }
          break;
        case 4:
          if (X100ConfigData.PcieX2Dn4Enable == 0) {
            SetDevicePowerState (PciIo, 3);
            gBS->Stall (50 * 1000);
          }
          break;
        case 5:
          if (X100ConfigData.PcieX2Dn5Enable == 0) {
            SetDevicePowerState (PciIo, 3);
            gBS->Stall (50 * 1000);
          }
          break;
        case 6:
          if (X100ConfigData.PcieX1Dn6Enable == 0) {
            SetDevicePowerState (PciIo, 3);
            gBS->Stall (50 * 1000);
          }
          break;
        case 7:
          if (X100ConfigData.PcieX1Dn7Enable == 0) {
            SetDevicePowerState (PciIo, 3);
            gBS->Stall (50 * 1000);
          }
          break;
        case 8:
          if (X100ConfigData.PcieX1Dn8Enable == 0) {
            SetDevicePowerState (PciIo, 3);
            gBS->Stall (50 * 1000);
          }
          break;
        case 9:
          if (X100ConfigData.PcieX1Dn9Enable == 0) {
            SetDevicePowerState (PciIo, 3);
            gBS->Stall (50 * 1000);
          }
          break;
        default:
          break;
        }
      }
      //
      //display
      //
      if ((DeviceID == 0xdc22) && (VendorID == 0x1db7)) {
        SetX100ConfigToDc (&X100ConfigData, PciIo);
      }
      //
      //Sata Control
      //
      SataPi = 0;
      if ((DeviceID == 0xdc26) && (VendorID == 0x1db7)) {
        PciIo->Pci.Read (
                     PciIo,
                     EfiPciIoWidthUint8,
                     0x2E,
                     1,
                     &Temp
                     );
        //test
        //Temp = 0xF;
        DEBUG ((DEBUG_INFO, "X100 Sata used : %x\n", Temp));
        //test
        for (Num = 0; Num < 4; Num++) {
          X100ConfigData.SataPort[Num].IsUsed = (Temp >> Num) & 0x1;
          if (X100ConfigData.SataPort[Num].IsUsed == 0) {
            X100ConfigData.SataPort[Num].Enable = 0;
          }
          DEBUG ((DEBUG_INFO, "X100 Sata Port[%d] Used : %d, Enable : %d\n", Num,
                      X100ConfigData.SataPort[Num].IsUsed, X100ConfigData.SataPort[Num].Enable));
          SataPi |= ((X100ConfigData.SataPort[Num].Enable & 0x1) << Num);
        }
        DEBUG ((DEBUG_INFO, "X100 Sata Pi : %0x\n", SataPi));
        Temp = 0x03;
        PciIo->Pci.Write (
                     PciIo,
                     EfiPciIoWidthUint8,
                     0x4,
                     1,
                     &Temp
                     );
        PciIo->Mem.Write (
                     PciIo,
                     EfiPciIoWidthUint32,
                     5,
                     0xC,
                     1,
                     &SataPi
                     );
        Temp = 0x0;
        PciIo->Pci.Write (
                     PciIo,
                     EfiPciIoWidthUint8,
                     0x4,
                     1,
                     &Temp
                     );
      }
    }
  }
  Status = gRT->SetVariable (
                  VAR_X100_PCI_INFO,
                  &gX100PciInfoVarGuid,
                  PLATFORM_SETUP_VARIABLE_FLAG,
                  sizeof (X100_PCI_INFO),
                  &X100PciInfo
                  );
  ASSERT_EFI_ERROR(Status);

  Status = gRT->SetVariable (
                  VAR_X100_CONFIG,
                  &gAdvancedX100VarGuid,
                  PLATFORM_SETUP_VARIABLE_FLAG,
                  VarSize,
                  &X100ConfigData
                  );
  ASSERT_EFI_ERROR(Status);

  return EFI_SUCCESS;
}

/**
  Modify PciRoot Port Tag.

  @retval  EFI_SUCCESS    Modify PciRoot Port Tag successfully.
           other          Modify PciRoot Port Tag failed.

**/
EFI_STATUS
ModifyPciRootPortTag (
  VOID
  )
{
  UINT32                      Status;
  UINTN                       NumOfPciHandles;
  EFI_HANDLE                  *HandleBuffer;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  UINT32                      Index;
  UINT16                      DeviceID;
  UINT16                      VendorID;
  UINT8                       Temp;
  UINT32                      PciExpressCapabilityOffset;

  Temp = 0;
  Status = 0;
  NumOfPciHandles = 0;
  HandleBuffer = NULL;

  //modify pci rootport tag only x100 is existed.
  if (IsX100Existed () == 0) {
    return EFI_SUCCESS;
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &NumOfPciHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return 0;
  }

  DEBUG ((DEBUG_INFO, "Pci handles number is %d\n", NumOfPciHandles));

  for (Index = 0; Index < NumOfPciHandles; Index ++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (!EFI_ERROR (Status)) {
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x0,
                   1,
                   &VendorID
                   );
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x2,
                   1,
                   &DeviceID
                   );

      if (VendorID == 0x17CD) {
        PciExpressCapabilityOffset = 0x80;
        PciIo->Pci.Read (
                     PciIo,
                     EfiPciIoWidthUint8,
                     PciExpressCapabilityOffset + 0x09,
                     1,
                     &Temp
                     );
        Temp &= 0xFE;
        PciIo->Pci.Write (
                     PciIo,
                     EfiPciIoWidthUint8,
                     PciExpressCapabilityOffset + 0x09,
                     1,
                     &Temp
                     );
      }
    }
  }
  return EFI_SUCCESS;
}


