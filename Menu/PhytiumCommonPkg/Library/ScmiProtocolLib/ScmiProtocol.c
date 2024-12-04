/**
  Realize sending and receiving SCMI messages at the protocol layer.

  Copyright (C) 2022-2023, Phytium Technology Co, Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Uefi.h>

#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/ScmiLib.h>

/**
  Flush L3 cache function. Phytium SCMI private protocol :
  Protocol Id is 0x81, message ID is 0x9.For PD2308.

  @oaram[in]    Base        A pointer to SCMI_ADDRESS_BASE.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiFlushL3Cache (
  IN  SCMI_ADDRESS_BASE  *Base,
  OUT INT32              *ScmiStatus
  )
{
  EFI_STATUS  Status;
  UINT32  PayloadIn[3];
  UINT32  PayloadOut[32];
  UINT32  Len;
  UINT32  Cluster;
  UINT64  Result;

  Result = 0;
  Cluster = 0;

  Result = ArmReadMpidr ();
  Cluster = (Result & 0xff0000) >> 16;
  DEBUG ((DEBUG_INFO, "the cluster is %d\n", Cluster));

  PayloadIn[0] = Cluster;
  PayloadIn[1] = 0;
  PayloadIn[2] = 0;

  Status = ScmiCommandExecute (
             Base,
             SCMI_PROTOCOL_ID_FT_DEFINED,
             FLUSH_L3_CACHE,
             3,
             PayloadIn,
             &Len,
             PayloadOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *ScmiStatus = PayloadOut[0];

  return Status;
}

/**
  Enable or Disable cppc freq function. Phytium SCMI private protocol :
  Protocol Id is 0x81, message ID is 0x9.For E2000.

  @oaram[in]    Base        A pointer to SCMI_ADDRESS_BASE.
  @param[in]    Enable      1 - Enable, 0 - Disable.
  @param[out]   ScmiStatus  A pointer to enum SCMI_ERROR.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiEnableCppc (
  IN  SCMI_ADDRESS_BASE  *Base,
  IN  UINT32             Enable,
  OUT INT32              *ScmiStatus
  )
{
  UINT32  Payload[32];
  UINT32  Len;
  EFI_STATUS  Status;

  Status = ScmiCommandExecute (
             Base,
             SCMI_PROTOCOL_ID_FT_DEFINED,
             CPPC_ENABLE_SWITCH,
             1,
             &Enable,
             &Len,
             Payload
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ScmiStatus = Payload[0];

  return Status;
}

/**
  Power control at S3 S4 or S5. Phytium SCMI private protocol :
  Protocol Id is 0x81, message ID is 0x5.For E2000.

  @oaram[in]    Base        A pointer to SCMI_ADDRESS_BASE.
  @param[in]    Enable      Power state, 0 - S0, 1 - S3, 2- S4, 3 - S5.
  @param[in]    WakeEnable  Wake enable bit mask. 1 - enable, 0 - disable.
                            bit 0 - USB0,
                            bit 1 - USB1,
                            bit 2 - MAC 0,
                            bit 3 - MAC 1,
                            bit 4 - MAC 2,
                            bit 5 - MAC 3,
                            bit 6 - GPIO0
                            bit 7 - keyboard,
                            bit 8 - primary core,
                            bit 31:9 - reserved.
  @param[in]    WakeGpio    Pad number of GPIO that support wakeup.
  @param[out]   ScmiStatus  A pointer to enum SCMI_ERROR.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiPowerStateS3S4S5 (
  IN  SCMI_ADDRESS_BASE  *Base,
  IN  UINT32             PowerState,
  IN  UINT32             WakeEnable,
  IN  UINT32             WakeGpio,
  OUT INT32              *ScmiStatus
  )
{
  UINT32  PayloadIn[3];
  UINT32  PayloadOut[32];
  UINT32  Len;
  EFI_STATUS  Status;

  PayloadIn[0] = PowerState;
  PayloadIn[1] = WakeEnable;
  PayloadIn[2] = WakeGpio;

  Status = ScmiCommandExecute (
             Base,
             SCMI_PROTOCOL_ID_FT_DEFINED,
             S3_S4_S5_CTRL_105,
             3,
             PayloadIn,
             &Len,
             PayloadOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ScmiStatus = PayloadOut[0];

  return Status;
}

/**
  Power control at S3 S4 or S5. Phytium SCMI private protocol :
  Protocol Id is 0x81, message ID is 0x2.For PD2308.

  @oaram[in]    Base        A pointer to SCMI_ADDRESS_BASE.
  @param[in]    Enable      Power state, 3 - S3.
  @param[in]    WakeEnable  Wake enable bit mask. 1 - enable, 0 - disable.
  @param[in]    WakeGpio    Pad number of GPIO that support wakeup.
  @param[out]   ScmiStatus  A pointer to enum SCMI_ERROR.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiPowerStateS3S4S5V2 (
  IN  SCMI_ADDRESS_BASE  *Base,
  IN  UINT32             PowerState,
  IN  UINT32             WakeEnable,
  IN  UINT32             WakeGpio,
  OUT INT32              *ScmiStatus
  )
{
  UINT32  PayloadIn[3];
  UINT32  PayloadOut[32];
  UINT32  Len;
  EFI_STATUS  Status;

  PayloadIn[0] = PowerState;
  PayloadIn[1] = WakeEnable;
  PayloadIn[2] = WakeGpio;

  Status = ScmiCommandExecute (
             Base,
             SCMI_PROTOCOL_ID_FT_DEFINED,
             S3_S4_S5_CTRL_105_V2,
             3,
             PayloadIn,
             &Len,
             PayloadOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ScmiStatus = PayloadOut[0];

  return Status;
}



/**
  PMBus config. Phytium SCMI private protocol : Protocol Id is 0x81, message ID
  is 0x12.For S5000C.

  @oaram[in]    Base        A pointer to SCMI_ADDRESS_BASE.
  @param[in]    Config      A pointer to PMBUS_CONFIG.
  @param[out]   ScmiStatus  A pointer to enum SCMI_ERROR.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiPmBusConfig (
  IN  SCMI_ADDRESS_BASE  *Base,
  IN  PMBUS_CONFIG       *Config,
  OUT INT32              *ScmiStatus
  )
{
  UINT32      PayloadIn[5];
  UINT32      PayloadOut[32];
  UINT32      Len;
  EFI_STATUS  Status;

  CopyMem (PayloadIn, Config, sizeof (PMBUS_CONFIG));

  Status = ScmiCommandExecute (
             Base,
             SCMI_PROTOCOL_ID_FT_DEFINED,
             PMBUS_CONFIG_FUNC,
             5,
             PayloadIn,
             &Len,
             PayloadOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ScmiStatus = PayloadOut[0];

  return Status;
}

/**
  Dvfs config. Phytium SCMI private protocol : Protocol Id is 0x81, message ID
  is 0x13. For S5000C.

  @oaram[in]    Base        A pointer to SCMI_ADDRESS_BASE.
  @param[in]    Enable      Dvfs enable. 0-disable, 1-enable.
  @param[out]   ScmiStatus  A pointer to enum SCMI_ERROR.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.

**/
EFI_STATUS
ScmiDvfsEnable (
  IN  SCMI_ADDRESS_BASE  *Base,
  IN  UINT32             Enable,
  OUT INT32              *ScmiStatus
  )
{
  UINT32      PayloadIn[1];
  UINT32      PayloadOut[32];
  UINT32      Len;
  EFI_STATUS  Status;

  PayloadIn[0] = Enable;

  Status = ScmiCommandExecute (
             Base,
             SCMI_PROTOCOL_ID_FT_DEFINED,
             DVFS_ENABLE,
             1,
             PayloadIn,
             &Len,
             PayloadOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ScmiStatus = PayloadOut[0];

  return Status;
}

/**
  Force powerdown temperature config. Phytium SCMI private protocol : Protocol
  Id is 0x81, message ID is 0x14. For S5000C.

  @oaram[in]    Base        A pointer to SCMI_ADDRESS_BASE.
  @param[in]    Temp        Force powerdown temperature.
  @param[out]   ScmiStatus  A pointer to enum SCMI_ERROR.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.

**/
EFI_STATUS
ScmiPowerDownTempSet (
  IN  SCMI_ADDRESS_BASE  *Base,
  IN  UINT32             Temp,
  OUT INT32              *ScmiStatus
  )
{
  UINT32      PayloadIn[1];
  UINT32      PayloadOut[32];
  UINT32      Len;
  EFI_STATUS  Status;

  PayloadIn[0] = Temp;

  Status = ScmiCommandExecute (
             Base,
             SCMI_PROTOCOL_ID_FT_DEFINED,
             POWER_DOWN_TEMP_CONFIG,
             1,
             PayloadIn,
             &Len,
             PayloadOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ScmiStatus = PayloadOut[0];

  return Status;
}
