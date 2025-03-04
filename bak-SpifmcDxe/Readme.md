# Overview

SpifmcDxe is a general DXE driver for SPI Fcontroller core self-innovated.

This driver implements communication based on the standard SPI protocol and supports operations on storage devices such as NOR flash via SPI.

# How to use standard SpifmcDxe and NorFlashDxe to communicate

## 1. Locate SPI Master Protocol and Nor Flash Protocol

```
  STATIC   SPI_NOR                        *Nor;
  STATIC   SOPHGO_NOR_FLASH_PROTOCOL      *NorFlashProtocol;
  STATIC   SOPHGO_SPI_MASTER_PROTOCOL     *SpiMasterProtocol;

  //
  // Locate SPI Master protocol
  //
  Status = gBS->LocateProtocol (
                  &gSophgoSpiMasterProtocolGuid,
                  NULL,
                  (VOID *)&SpiMasterProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Cannot locate SPI Master protocol!\n",
      __func__
      ));
    return FALSE;
  }

  //
  // Locate Nor Flash protocol
  //
  Status = gBS->LocateProtocol (
                  &gSophgoNorFlashProtocolGuid,
                  NULL,
                  (VOID *)&NorFlashProtocol
                  );
  if (EFI_ERROR (Status)) {
    Print (L"  Cannot locate Nor Flash protocol!\n");
    DEBUG ((
      DEBUG_ERROR,
      "%a: Cannot locate Nor Flash protocol!\n",
      __func__
      ));
    return FALSE;
  }
```

## 2. Setup SPI slave

```
  UINT8            SelectedFlashNumber;

  //
  // Setup and probe Nor flash
  //
  Nor = SpiMasterProtocol->SetupDevice (
                  SpiMasterProtocol,
                  Nor,
		  SelectedFlashNumber
                  );

  if (Nor == NULL) {
    DEBUG ((
      "%a: Nor Flash %d not found!\n",
      __func__,
      SelectedFlashNumber
      ));
    return FALSE;
  }

/**
  Probe and initialize nor flash.

  @retval  EFI_SUCCESS       The Nor Flash probe successfully.
  @retval  EFI_NOT_FOUND     Nor flash does not exist.
  @retval  EFI_DEVICE_ERROR  Nor flash initialized failed.
**/
STATIC
EFI_STATUS
NorFlashProbe (
  VOID
  )
{
  EFI_STATUS  Status;

  //
  // Read Nor Flash ID
  //
  Status = NorFlashProtocol->GetFlashid (Nor, TRUE);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Read Nor flash ID failed!\n",
      __func__
      ));
    return EFI_NOT_FOUND;
  }

  //
  // Initialize Nor Flash
  //
  Status = NorFlashProtocol->Init (NorFlashProtocol, Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Cannot initialize flash device\n",
      __func__
      ));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
```

## 3. Read ==> Erase ==> Write

```
  UINTN       BlockSize;

  BlockSize = Nor->Info->SectorSize;
  TempBuffer = NULL;
  TempBuffer = AllocatePool (BlockSize);
  Status = EFI_SUCCESS;
  Count  = (Size / BlockSize);

  //
  // make sure Address & Size is BlockSize align
  //
  if (((Address % BlockSize) != 0)
	|| ((Size % BlockSize) != 0) ) {
    Count ++;
    Address = Address & (~ (BlockSize - 1));
  }

  for (Index = 0; Index < Count; Index ++) {
    if (TempBuffer) {
      NorFlashProtocol->ReadData (
		      Nor,
		      Address + Index * BlockSize,
		      BlockSize,
		      TempBuffer
		      );
      if (CompareMem (TempBuffer, Buffer + Index * BlockSize, BlockSize) == 0) {
        gST->ConOut->SetCursorPosition (gST->ConOut, Columns, Rows);
        Print (L"%s %02d%%%", String, ((Index + 1) * 100) / Count);
        continue;
      }
    }

    Status = NorFlashProtocol->Erase (
		    Nor,
		    Address + Index * BlockSize,
		    BlockSize
		    );
    if (EFI_ERROR (Status)) {
      Print (L"\r%s Fail!\n", String);
      goto ProExit;
    }

    Status = NorFlashProtocol->WriteData (
		    Nor,
		    Address + Index * BlockSize,
		    BlockSize,
		    Buffer + Index * BlockSize
		    );
    if (EFI_ERROR (Status)) {
      Print (L"\r%s Fail!\n", String);
      goto ProExit;
    }

    Print (L"%s %02d%%%", String, ((Index + 1) * 100) / Count);
  }

```

## 5. Free the memory if SPI transfer is no longer needed

```
  Status = SpiProtocol->SpiCleanupDevice (
                          SpiProtocol,
                          SpiSlave);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,"SpiCleanupDevice error\n"));
    return Status;
  }

  FreePool (TxBuffer);
  FreePool (RxBuffer);
  FreePool (SpiTransfer);

```

# How to communicate with nor flash

## 1. Locate protocol

```
  SOPHGO_SPI_PROTOCOL *SpiProtocol = NULL;
  Status = gBS->LocateProtocol (
                  &gSophgoSpiProtocolGuid,
                  NULL,
                  (VOID **)&SpiProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,"Cannot locate Spi Master protocol\n"));
    return Status;
  }
```

## 2. Setup SPI slave

```
  SPI_DEVICE  *SpiSlave;
  UINT32      SpiBus = 1;
  UINT8       Cs     = 1;
  UINT8       Mode   = 3;

  SpiSlave = AllocateZeroPool (sizeof (SPI_DEVICE));

  Status = SpiProtocol->SpiSetupDevice (
                          SpiProtocol,
                          SpiSlave,
                          SpiBus,
                          Cs,
                          Mode);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,"Cannot setup spi slave\n"));
    return Status;
  }
```

## 3. Setup flash operation struct

```
  SPI_MEM_OP  *OpFlash;
  OpFlash = AllocateZeroPool (sizeof (SPI_MEM_OP));
```
## 4. flash operation

To implement this operation, the following parameters need to be determined from the corresponding data manual:

* number of command bytes
* command value
* number of address bytes
* address value
* number of dummy bytes
* data direction (read or write data)
* number of data bytes to read or write
* transfer speed

### 4.1 Fast read

```
  #define FAST_READ_DATA 0xb

  gBS->SetMem (OpFlash, sizeof (SPI_MEM_OP), 0);

  OpFlash->Cmd.NBytes  = 1;
  OpFlash->Cmd.OpCode  = FAST_READ_DATA;

  OpFlash->Addr.NBytes = 3;
  OpFlash->Addr.Val    = 0x1200;

  OpFlash->Dummy.NBytes = 1;

  OpFlash->Data.Dir    = SPI_MEM_DATA_IN;
  OpFlash->Data.NBytes = 256;
  OpFlash->Data.Buf.In = AllocateZeroPool (OpFlash->Data.NBytes);

  OpFlash->SpeedHz     = 10 * 1000 * 1000;

  SpiProtocol->SpiExecMemOp (SpiProtocol, SpiSlave, OpFlash);

  Buffer = (UINT8 *)(OpFlash->Data.Buf.In);
  for (UINT32 Loop = 0; Loop < OpFlash->Data.NBytes; ++Loop) {
    if ((Loop != 0) && ((Loop % 0x20) == 0))
      DEBUG ((DEBUG_ERROR, "\n"));
    DEBUG ((DEBUG_ERROR, "%2x ", Buffer[Loop]));
  }
  DEBUG ((DEBUG_ERROR, "\n"));
```

### 4.2 Read manufacturer/device ID

```
  #define READ_DEVICE_ID 0x90

  gBS->SetMem (OpFlash, sizeof (SPI_MEM_OP), 0);

  OpFlash->Cmd.NBytes  = 1;
  OpFlash->Cmd.OpCode  = READ_DEVICE_ID;

  OpFlash->Addr.NBytes = 3;
  OpFlash->Addr.Val    = 0;

  OpFlash->Data.Dir    = SPI_MEM_DATA_IN;
  OpFlash->Data.NBytes = 2;
  OpFlash->Data.Buf.In = AllocateZeroPool (OpFlash->Data.NBytes);

  OpFlash->SpeedHz     = 10 * 1000 * 1000;

  SpiProtocol->SpiExecMemOp (SpiProtocol, SpiSlave, OpFlash);

  Buffer = (UINT8 *)(OpFlash->Data.Buf.In);
  for (UINT32 Loop = 0; Loop < OpFlash->Data.NBytes; ++Loop) {
    if ((Loop != 0) && ((Loop % 0x20) == 0))
      DEBUG ((DEBUG_ERROR, "\n"));
    DEBUG ((DEBUG_ERROR, "%2x ", Buffer[Loop]));
  }
  DEBUG ((DEBUG_ERROR, "\n"));
```

### 4.3 Write enable

```
  #define WRITE_ENABLE 0x6

  gBS->SetMem (OpFlash, sizeof (SPI_MEM_OP), 0);

  OpFlash->Cmd.NBytes  = 1;
  OpFlash->Cmd.OpCode  = WRITE_ENABLE;

  OpFlash->SpeedHz     = 10 * 1000 * 1000;

  SpiProtocol->SpiExecMemOp (SpiProtocol, SpiSlave, OpFlash);
```

### 4.4 Erase sector

```
  #define SECTOR_ERASE 0x20

  gBS->SetMem (OpFlash, sizeof (SPI_MEM_OP), 0);

  OpFlash->Cmd.NBytes  = 1;
  OpFlash->Cmd.OpCode  = SECTOR_ERASE;

  OpFlash->Addr.NBytes = 3;
  OpFlash->Addr.Val    = 0x1200;

  OpFlash->SpeedHz     = 10 * 1000 * 1000;

  SpiProtocol->SpiExecMemOp (SpiProtocol, SpiSlave, OpFlash);
```

### 4.5 Write data

```
  #define PAGE_PROGRAM 0x2

  gBS->SetMem (OpFlash, sizeof (SPI_MEM_OP), 0);

  OpFlash->Cmd.NBytes  = 1;
  OpFlash->Cmd.OpCode  = PAGE_PROGRAM;

  OpFlash->Addr.NBytes = 3;
  OpFlash->Addr.Val    = 0x1200;

  OpFlash->Data.Dir    = SPI_MEM_DATA_OUT;
  OpFlash->Data.NBytes = 256;
  UINT8 *OutBuffer     = AllocateZeroPool (OpFlash->Data.NBytes);

  OpFlash->SpeedHz     = 10 * 1000 * 1000;

  for (UINT32 Loop = 0; Loop < OpFlash->Data.NBytes; Loop++)
    OutBuffer[Loop] = Loop & 0xff;
  OpFlash->Data.Buf.Out = OutBuffer;

  SpiProtocol->SpiExecMemOp (SpiProtocol, SpiSlave, OpFlash);
```
