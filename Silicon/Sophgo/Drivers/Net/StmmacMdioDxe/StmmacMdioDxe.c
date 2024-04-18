/** @file
 *
 *  STMMAC Ethernet Driver -- MDIO bus implementation
 *  Provides Bus interface for MII registers.
 *
 *  Copyright (c) 2024, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <Protocol/BoardDesc.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/Mdio.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "StmmacMdioDxe.h"

STATIC
EFI_STATUS
MdioCheckParam (
 IN INTN  PhyAddr,
 IN INTN  RegOff
 )
{
  if (PhyAddr > PHY_ADDR_MASK) {
    DEBUG ((
      DEBUG_ERROR,
      "Invalid PHY address %d\n",
      PhyAddr
      ));

    return EFI_INVALID_PARAMETER;
  }

  if (RegOff > PHY_REG_MASK) {
    DEBUG ((
      DEBUG_ERROR,
      "Invalid register offset %d\n",
      RegOff
      ));

    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MdioWaitReady (
  IN UINTN  MdioBase
  )
{
  UINT32  Timeout;
  UINT32  MdioReg;

  Timeout = STMMAC_MDIO_TIMEOUT;

  //
  // wait till the MII is not busy
  //
  do {
    //
    // read MII register
    //
    MdioReg = MmioRead32(MdioBase);
    if (Timeout-- == 100) {
      DEBUG ((
        DEBUG_ERROR,
	"MII busy Timeout\n"
	));

      return EFI_TIMEOUT;
    }
  } while (MdioReg & MII_BUSY);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MdioOperation (
  IN CONST SOPHGO_MDIO_PROTOCOL  *This,
  IN UINT32                      PhyAddr,
  IN UINT32                      MdioIndex,
  IN UINT32                      RegOff,
  IN BOOLEAN                     Write,
  IN OUT UINT32                  *Data
  )
{
  UINTN      MdioBase;
  UINT32     MdioReg;
  EFI_STATUS Status;

  MdioBase = This->BaseAddresses[MdioIndex];

  Status = MdioCheckParam (PhyAddr, RegOff);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "MdioDxe: wrong parameters\n"
      ));

    return Status;
  }

  //
  // Wait until any existing MII operation is complete.
  //
  Status = MdioWaitReady (MdioBase);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "MdioDxe: MdioWaitReady error\n"
      ));

    return Status;
  }

  /* fill the phy addr and reg offset and write opcode and data */
  MdioReg = (PhyAddr << MVEBU_SMI_DEV_ADDR_OFFS)
      | (RegOff << MVEBU_SMI_REG_ADDR_OFFS);
  if (Write) {
    MdioReg &= ~MVEBU_SMI_OPCODE_READ;
    MdioReg |= (*Data << MVEBU_SMI_DATA_OFFS);
  } else {
    MdioReg |= MVEBU_SMI_OPCODE_READ;
  }
  MiiConfig = ((Addr << MIIADDRSHIFT) & MII_ADDRMSK) |
              ((Reg << MIIREGSHIFT) & MII_REGMSK)|
               MII_WRITE |
               MII_CLKRANGE_150_250M |
               MII_BUSY;

  //
  // Write the desired value to the register first
  //
  MmioWrite32 (MdioBase + GMAC_MDIO_DATA, (Data & 0xFFFF));

  //
  // write this config to register
  //
  MmioWrite32 (MdioBase + GMAC_MDIO_ADDR, MiiConfig);

  //
  // Wait until any existing MII operation is complete.
  //
  Status = MdioWaitReady (MdioBase);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "MdioDxe: MdioWaitReady error\n"
      ));

    return Status;
  }

  if (!Write) {
    *Data = MmioRead32 (MdioBase) & MII_DATA_MASK;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
StmmacMdioRead (
  IN CONST SOPHGO_MDIO_PROTOCOL  *This,
  IN UINT32                      PhyAddr,
  IN UINT32                      MdioIndex,
  IN UINT32                      RegOff,
  IN UINT32                      *Data
  )
{
  EFI_STATUS Status;

  Status = MdioOperation (
            This,
            PhyAddr,
            MdioIndex,
            RegOff,
            FALSE,
            Data
            );

  return Status;
}

EFI_STATUS
StmmacMdioWrite (
  IN CONST SOPHGO_MDIO_PROTOCOL  *This,
  IN UINT32                      PhyAddr,
  IN UINT32                      MdioIndex,
  IN UINT32                      RegOff,
  IN UINT32                      Data
  )
{
  return MdioOperation (
            This,
            PhyAddr,
            MdioIndex,
            RegOff,
            TRUE,
            &Data
            );
}

EFI_STATUS
EFIAPI
MdioDxeInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT8                       Index;
  UINTN                       MdioDevCount;
  UINTN                       Capacity;
  INT32                       Node;
  CONST VOID                  *Prop;
  UINT32                      PropSize;
  FDT_CLIENT_PROTOCOL         *FdtClient;
  SOPHGO_MDIO_PROTOCOL        *Mdio;
  EFI_STATUS                  Status;
  EFI_STATUS                  FindNodeStatus;
  EFI_HANDLE                  Handle;

  Handle       = NULL;
  MdioDevCount = 0;
  Capacity     = 0;

  //
  // Obtain base addresses of all possible controllers
  //
  Status = gBS->LocateProtocol (
      &gFdtClientProtocolGuid,
      NULL,
      (VOID **) &FdtClient
      );
  ASSERT_EFI_ERROR (Status);

  for (FindNodeStatus = FdtClient->FindCompatibleNode (
                                     FdtClient,
                                     "bitmain,ethernet",
                                     &Node
                                     );

    !EFI_ERROR (FindNodeStatus);

    FindNodeStatus = FdtClient->FindNextCompatibleNode (
                                     FdtClient,
                                     "bitmain,ethernet",
                                     Node,
                                     &Node
                                     ))
  {
    if (MdioDevNum >= Capacity) {
      UINTN NewCapacity = Capacity == 0 ? 1 : ++Capacity;
      UINTN *NewArray = NULL;

      NewArray = AllocateZeroPool (NewCapacity * sizeof (UINTN));
      if (!NewArray) {
        DEBUG ((
          DEBUG_ERROR,
          "MdioDxe: Protocol allocation failed\n"
          ));

	if (Mdio->BaseAddresses) {
          FreePool (Mdio->BaseAddresses);
	}

        return EFI_OUT_OF_RESOURCES;
      }

      if (Mdio->BaseAddresses != NULL) {
        CopyMem (NewArray, Mdio->BaseAddresses, Size * sizeof (UINTN));

        FreePool (Mdio->BaseAddresses);
      }

      Mdio->BaseAddresses = NewArray;
      Capacity = NewCapacity;
    }

    Status = FdtClient->GetNodeProperty (
                                FdtClient,
                                Node,
                                "reg",
                                &Prop,
                                &PropSize
                                );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Get reg failed (Status = %r)\n",
        __func__,
        Status
        ));

      return Status;
    }

    Mdio->BaseAddresses[MdioDevNum] = SwapBytes64 (((CONST UINT64 *) Prop)[0]); 

    if (Mdio->BaseAddresses[MdioDevNum] >= (1UL << 39)) {
      break;
    }
    
    MdioDevNum ++;
  }

  Mdio = AllocateZeroPool (sizeof (SOPHGO_MDIO_PROTOCOL));
  if (Mdio == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "MdioDxe: Protocol allocation failed\n"
      ));

    return EFI_OUT_OF_RESOURCES;
  }

  Mdio->ControllerCount = MdioDevCount;
  Mdio->Read            = StmmacMdioRead;
  Mdio->Write           = StmmacMdioWrite;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gSophgoMdioProtocolGuid, Mdio,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "Failed to install interfaces\n"
      ));

    return Status;
  }

  return EFI_SUCCESS;
}
