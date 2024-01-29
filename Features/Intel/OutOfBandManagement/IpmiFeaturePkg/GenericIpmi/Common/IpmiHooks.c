/** @file
  IPMI common hook functions

  @copyright
  Copyright 2014 - 2021 Intel Corporation. <BR>
  Copyright (c) 1985 - 2023, American Megatrends International LLC. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "IpmiHooks.h"

EFI_STATUS
EFIAPI
IpmiSendCommand (
  IN      IPMI_TRANSPORT  *This,
  IN      UINT8           NetFunction,
  IN      UINT8           Lun,
  IN      UINT8           Command,
  IN      UINT8           *CommandData,
  IN      UINT32          CommandDataSize,
  IN OUT  UINT8           *ResponseData,
  IN OUT  UINT32          *ResponseDataSize
  )

/*++

Routine Description:

  Send Ipmi Command in the right mode: HECI or KCS,  to the
  appropiate device, ME or BMC.

Arguments:

  This              - Pointer to IPMI protocol instance
  NetFunction       - Net Function of command to send
  Lun               - LUN of command to send
  Command           - IPMI command to send
  CommandData       - Pointer to command data buffer, if needed
  CommandDataSize   - Size of command data buffer
  ResponseData      - Pointer to response data buffer
  ResponseDataSize  - Pointer to response data buffer size

Returns:

  EFI_INVALID_PARAMETER - One of the input values is bad
  EFI_DEVICE_ERROR      - IPMI command failed
  EFI_BUFFER_TOO_SMALL  - Response buffer is too small
  EFI_UNSUPPORTED       - Command is not supported by BMC
  EFI_SUCCESS           - Command completed successfully

--*/
{
  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // This Will be unchanged ( BMC/KCS style )
  //
  return IpmiSendCommandToBmc (
                               This,
                               NetFunction,
                               Lun,
                               Command,
                               CommandData,
                               (UINT8)CommandDataSize,
                               ResponseData,
                               (UINT8 *)ResponseDataSize,
                               NULL
                               );
} // IpmiSendCommand()

EFI_STATUS
EFIAPI
IpmiSendCommand2 (
  IN      IPMI_TRANSPORT2  *This,
  IN      UINT8            NetFunction,
  IN      UINT8            Lun,
  IN      UINT8            Command,
  IN      UINT8            *CommandData,
  IN      UINT32           CommandDataSize,
  IN OUT  UINT8            *ResponseData,
  IN OUT  UINT32           *ResponseDataSize
  )

/*++

Routine Description:

  This API use the default interface (PcdDefaultSystemInterface) to send IPMI command
  in the right mode to the appropiate device, ME or BMC.

Arguments:

  This              - Pointer to IPMI protocol instance
  NetFunction       - Net Function of command to send
  Lun               - LUN of command to send
  Command           - IPMI command to send
  CommandData       - Pointer to command data buffer, if needed
  CommandDataSize   - Size of command data buffer
  ResponseData      - Pointer to response data buffer
  ResponseDataSize  - Pointer to response data buffer size

Returns:

  EFI_INVALID_PARAMETER - One of the input values is bad
  EFI_DEVICE_ERROR      - IPMI command failed
  EFI_BUFFER_TOO_SMALL  - Response buffer is too small
  EFI_UNSUPPORTED       - Command is not supported by BMC
  EFI_SUCCESS           - Command completed successfully

--*/
{
  IPMI_BMC_INSTANCE_DATA  *IpmiInstance;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  IpmiInstance = INSTANCE_FROM_IPMI_TRANSPORT2_THIS (This);

  if ((FixedPcdGet8 (PcdKcsInterfaceSupport) == 1) &&
      ((IpmiInstance->IpmiTransport2.InterfaceType == SysInterfaceKcs) &&
       (IpmiInstance->IpmiTransport2.Interface.KcsInterfaceState == IpmiInterfaceInitialized)))
  {
    return IpmiSendCommand (
                            &IpmiInstance->IpmiTransport,
                            NetFunction,
                            Lun,
                            Command,
                            CommandData,
                            CommandDataSize,
                            ResponseData,
                            ResponseDataSize
                            );
  }

  if ((FixedPcdGet8 (PcdBtInterfaceSupport) == 1) &&
      ((IpmiInstance->IpmiTransport2.InterfaceType == SysInterfaceBt) &&
       (IpmiInstance->IpmiTransport2.Interface.Bt.InterfaceState == IpmiInterfaceInitialized)))
  {
    return IpmiBtSendCommandToBmc (
                                   &IpmiInstance->IpmiTransport2,
                                   NetFunction,
                                   Lun,
                                   Command,
                                   CommandData,
                                   (UINT8)CommandDataSize,
                                   ResponseData,
                                   (UINT8 *)ResponseDataSize,
                                   NULL
                                   );
  }

  if ((FixedPcdGet8 (PcdSsifInterfaceSupport) == 1) &&
      ((IpmiInstance->IpmiTransport2.InterfaceType == SysInterfaceSsif) &&
       (IpmiInstance->IpmiTransport2.Interface.Ssif.InterfaceState == IpmiInterfaceInitialized)))
  {
    return IpmiSsifSendCommandToBmc (
                                     &IpmiInstance->IpmiTransport2,
                                     NetFunction,
                                     Lun,
                                     Command,
                                     CommandData,
                                     (UINT8)CommandDataSize,
                                     ResponseData,
                                     (UINT8 *)ResponseDataSize,
                                     NULL
                                     );
  }

  if ((FixedPcdGet8 (PcdIpmbInterfaceSupport) == 1) &&
      ((IpmiInstance->IpmiTransport2.InterfaceType == SysInterfaceIpmb) &&
       (IpmiInstance->IpmiTransport2.Interface.Ipmb.InterfaceState == IpmiInterfaceInitialized)))
  {
    return IpmiIpmbSendCommandToBmc (
                                     &IpmiInstance->IpmiTransport2,
                                     NetFunction,
                                     Lun,
                                     Command,
                                     CommandData,
                                     (UINT8)CommandDataSize,
                                     ResponseData,
                                     (UINT8 *)ResponseDataSize,
                                     NULL
                                     );
  }

  return EFI_UNSUPPORTED;
} // IpmiSendCommand2()

EFI_STATUS
EFIAPI
IpmiSendCommand2Ex (
  IN      IPMI_TRANSPORT2        *This,
  IN      UINT8                  NetFunction,
  IN      UINT8                  Lun,
  IN      UINT8                  Command,
  IN      UINT8                  *CommandData,
  IN      UINT32                 CommandDataSize,
  IN OUT  UINT8                  *ResponseData,
  IN OUT  UINT32                 *ResponseDataSize,
  IN      SYSTEM_INTERFACE_TYPE  InterfaceType
  )
{
  /*++
  Routine Description:

    This API use the specific interface type to send IPMI command
    in the right mode to the appropiate device, ME or BMC.

  Arguments:

    This              - Pointer to IPMI protocol instance
    NetFunction       - Net Function of command to send
    Lun               - LUN of command to send
    Command           - IPMI command to send
    CommandData       - Pointer to command data buffer, if needed
    CommandDataSize   - Size of command data buffer
    ResponseData      - Pointer to response data buffer
    ResponseDataSize  - Pointer to response data buffer size
    InterfaceType     - BMC Interface type.

  Returns:

    EFI_INVALID_PARAMETER - One of the input values is bad
    EFI_DEVICE_ERROR      - IPMI command failed
    EFI_BUFFER_TOO_SMALL  - Response buffer is too small
    EFI_UNSUPPORTED       - Command is not supported by BMC
    EFI_SUCCESS           - Command completed successfully

  --*/

  IPMI_BMC_INSTANCE_DATA  *IpmiInstance;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  IpmiInstance = INSTANCE_FROM_IPMI_TRANSPORT2_THIS (This);

  if ((FixedPcdGet8 (PcdKcsInterfaceSupport) == 1) &&
      ((InterfaceType == SysInterfaceKcs) && (IpmiInstance->IpmiTransport2.Interface.KcsInterfaceState == IpmiInterfaceInitialized)))
  {
    return IpmiSendCommand (
                            &IpmiInstance->IpmiTransport,
                            NetFunction,
                            Lun,
                            Command,
                            CommandData,
                            CommandDataSize,
                            ResponseData,
                            ResponseDataSize
                            );
  }

  if ((FixedPcdGet8 (PcdBtInterfaceSupport) == 1) &&
      ((InterfaceType == SysInterfaceBt) && (IpmiInstance->IpmiTransport2.Interface.Bt.InterfaceState == IpmiInterfaceInitialized)))
  {
    return IpmiBtSendCommandToBmc (
                                   &IpmiInstance->IpmiTransport2,
                                   NetFunction,
                                   Lun,
                                   Command,
                                   CommandData,
                                   (UINT8)CommandDataSize,
                                   ResponseData,
                                   (UINT8 *)ResponseDataSize,
                                   NULL
                                   );
  }

  if ((FixedPcdGet8 (PcdSsifInterfaceSupport) == 1) &&
      ((InterfaceType == SysInterfaceSsif) && (IpmiInstance->IpmiTransport2.Interface.Ssif.InterfaceState == IpmiInterfaceInitialized)))
  {
    return IpmiSsifSendCommandToBmc (
                                     &IpmiInstance->IpmiTransport2,
                                     NetFunction,
                                     Lun,
                                     Command,
                                     CommandData,
                                     (UINT8)CommandDataSize,
                                     ResponseData,
                                     (UINT8 *)ResponseDataSize,
                                     NULL
                                     );
  }

  if ((FixedPcdGet8 (PcdIpmbInterfaceSupport) == 1) &&
      ((InterfaceType == SysInterfaceIpmb) && (IpmiInstance->IpmiTransport2.Interface.Ipmb.InterfaceState == IpmiInterfaceInitialized)))
  {
    return IpmiIpmbSendCommandToBmc (
                                     &IpmiInstance->IpmiTransport2,
                                     NetFunction,
                                     Lun,
                                     Command,
                                     CommandData,
                                     (UINT8)CommandDataSize,
                                     ResponseData,
                                     (UINT8 *)ResponseDataSize,
                                     NULL
                                     );
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
IpmiGetBmcStatus (
  IN IPMI_TRANSPORT   *This,
  OUT BMC_STATUS      *BmcStatus,
  OUT SM_COM_ADDRESS  *ComAddress
  )

/*++

Routine Description:

  Updates the BMC status and returns the Com Address

Arguments:

  This        - Pointer to IPMI protocol instance
  BmcStatus   - BMC status
  ComAddress  - Com Address

Returns:

  EFI_SUCCESS - Success

--*/
{
  if ((This == NULL) || (BmcStatus == NULL) || (ComAddress == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return IpmiBmcStatus (
                        This,
                        BmcStatus,
                        ComAddress,
                        NULL
                        );
}
