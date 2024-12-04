#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmSmcLib.h>
#include <Ec.h>

#define  LPC_BASE_ADDR       FixedPcdGet64 (PcdLpcBaseAddress)
#define  EC_COMMAND_PORT     LPC_BASE_ADDR + EC_SC
#define  EC_DATA_PORT        LPC_BASE_ADDR + EC_DATA


//Wait till EC I/P buffer is free
STATIC
UINT8
EcIbFree (
  )
{
  UINT8 Status;
  do {
    Status = MmioRead8 (EC_COMMAND_PORT);
    DEBUG ((EFI_D_INFO, "%a(), Status = %08x\n", __FUNCTION__, Status));
  } while (Status & 2);
  return SUCCESS;
}

//Wait till EC O/P buffer is full
STATIC
UINT8
EcObFull (
  )
{
  UINT8 Status;
  do {
    Status = MmioRead8 (EC_COMMAND_PORT);
    DEBUG ((EFI_D_INFO, "%a(), Status = %08x\n", __FUNCTION__, Status));
  } while(!(Status & 1));
  return SUCCESS;
}

//Send EC command
UINT8
EcWriteCmd (
  UINT8 Cmd
  )
{
  EcIbFree ();
  MmioWrite8 (EC_COMMAND_PORT, Cmd);
  return SUCCESS;
}

//Write Data from EC data port
UINT8
EcWriteData (
  UINT8 Data
  )
{
  EcIbFree ();
  MmioWrite8 (EC_DATA_PORT, Data);
  return SUCCESS;
}

//Read Data from EC data Port
UINT8
EcReadData (
  UINT8 *PData
  )
{
  //暂时修改
  *PData = MmioRead8 (EC_DATA_PORT);
  EcObFull ();
  *PData = MmioRead8 (EC_DATA_PORT);
  return SUCCESS;
}

//Read Data from EC Memory from location pointed by Index
UINT8
EcReadMem (
  UINT8 Index,
  UINT8 *Data
  )
{
  UINT8 Cmd;
  Cmd = SMC_READ_EC;
  EcWriteCmd (Cmd);
  EcWriteData (Index);
  EcReadData (Data);
  return SUCCESS;
}

//Write Data to EC memory at location pointed by Index
UINT8
EcWriteMem (
  UINT8 Index,
  UINT8 Data
  )
{
  UINT8 Cmd;
  Cmd = SMC_WRITE_EC;
  EcWriteCmd (Cmd);
  EcWriteData (Index);
  EcWriteData (Data);
  return SUCCESS;
}

EFI_STATUS
EcInit (
  VOID
  )
{
  UINT32  LpcBaseAddr;
  LpcBaseAddr = FixedPcdGet64 (PcdLpcBaseAddress);
  DEBUG ((EFI_D_INFO, "LpcBaseAddr : 0x%08x\n", LpcBaseAddr));
  //mEcDataPort = LpcBaseAddr + EC_DATA;
  //mEcCommandPort = LpcBaseAddr + EC_SC;
  DEBUG ((EFI_D_INFO, "Ec Data Port : 0x%08x\n", EC_DATA_PORT));
  DEBUG ((EFI_D_INFO, "Ec Command Port : 0x%08x\n", EC_COMMAND_PORT));
  //MmioWrite32 (0x2807e214, 0xf0005);
  return EFI_SUCCESS;
}
