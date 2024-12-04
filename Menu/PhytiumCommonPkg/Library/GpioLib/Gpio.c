/** @file
  Phytium gpio controller library.

  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PhytiumCommonGpioLib.h>

/**
  Set a specify gpio pin direction, input or output.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.
  @param[in]    Dir      Direction to set. 0-input, 1-output.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is not in the range 0 - max.
                                       2.Gpio Port is not in the range 0 - max.
                                       3.The direction to set is not in and out.
**/
EFI_STATUS
SetGpioDirect (
  IN UINT8  Die,
  IN UINT8  Index,
  IN UINT8  Port,
  IN UINT8  Dir
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is incorrect!\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  if ((Dir != GPIO_DIRECTION_IN) && (Dir != GPIO_DIRECTION_OUT)) {
    DEBUG ((DEBUG_ERROR, "The direction to set is not in and out\n"));
  }

  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_SWPORTA_DDR);
  if (Dir == GPIO_DIRECTION_OUT) {
    Value |= (0x1 << Port);
  } else {
    Value &= ~(0x1 << Port);
  }
  MmioWrite32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_SWPORTA_DDR, Value);

  return EFI_SUCCESS;
}

/**
  Enable the interrupt of target port, the port must be input mode.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 - max.

**/
EFI_STATUS
GpioInterruptEnable (
  IN UINT8  Die,
  IN UINT8  Index,
  IN UINT8  Port
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is incorrect!\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_INTEN);
  Value |= (0x1 << Port);
  MmioWrite32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_INTEN, Value);

  return EFI_SUCCESS;
}

/**
  Set the trigger type and sensitive type of interrupt.

  @param[in]    Die                    Number of Die where gpio is, 0 - max.
  @param[in]    Index                  Gpio group index, 0 - max.
  @param[in]    Port                   Gpio port number in a group, 0 - max.
  @param[in]    TriggerType            Trigger type to set, 0-level, 1-edge.
  @param[in]    SensitiveType          Sensitive type to set, 0-low-level or edge-falling,
                                       1-level-high or edge-rising.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 -max.
                                       3.Interrupt's trigger type or sensitive type is incorrect.

**/
EFI_STATUS
SetGpioInterruptType (
  IN UINT8  Die,
  IN UINT8  Index,
  IN UINT8  Port,
  IN UINT8  TriggerType,
  IN UINT8  SensitiveType
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is incorrect!\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  if ((TriggerType != GPIO_INTERRUPT_LEVEL) && (TriggerType != GPIO_INTERRUPT_EDGE)) {
    DEBUG ((DEBUG_ERROR, "Interrupt's trigger type is incorrect!\n"));
    return EFI_INVALID_PARAMETER;
  }

  if ((SensitiveType != GPIO_INTERRUPT_TRIGGER_LEVEL_LOW) && (SensitiveType != GPIO_INTERRUPT_TRIGGER_LEVEL_HIGH)) {
    DEBUG ((DEBUG_ERROR, "Interrupt's sensitive type is incorrect!\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  //Set trigger type
  //
  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_INTTYPE_LEVEL);
  if (TriggerType == GPIO_INTERRUPT_EDGE) {
    Value |= (0x1 << Port);
  } else {
    Value &= ~(0x1 << Port);
  }
  MmioWrite32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_INTTYPE_LEVEL, Value);


  //
  //Set sensitive type
  //
   Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_INT_POLARITY);
   if ((SensitiveType == GPIO_INTERRUPT_TRIGGER_LEVEL_HIGH) || (SensitiveType == GPIO_INTERRUPT_TRIGGER_EDGE_RISING)) {
     Value |= (0x1 << Port);
   } else {
     Value &= ~(0x1 << Port);
   }
   MmioWrite32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_INT_POLARITY, Value);

  return EFI_SUCCESS;
}

/**
  Mask the interrupt of target port.

  @param[in]    Die                    Number of Die where gpio is, 0 - max.
  @param[in]    Index                  Gpio group index, 0 - max.
  @param[in]    Port                   Gpio port number in a group, 0 - max.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 - max.

**/
EFI_STATUS
MaskGpioInterrupt (
  IN UINT8  Die,
  IN UINT8  Index,
  IN UINT8  Port
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is incorrect!\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_INTMASK);
  Value |= (0x1 << Port);
  MmioWrite32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_INTMASK, Value);

  return EFI_SUCCESS;
}

/**
  Clear the interrupt of target port.

  @param[in]    Die                    Number of Die where gpio is, 0 - max.
  @param[in]    Index                  Gpio group index, 0 - max.
  @param[in]    Port                   Gpio port number in a group, 0 - max.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 - max.

**/
EFI_STATUS
ClearGpioInterrupt (
  IN UINT8  Die,
  IN UINT8  Index,
  IN UINT8  Port
  )
{
  UINT32  Value;

  Value = 0;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is incorrect!\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  Value |= (0x1 << Port);

  MmioWrite32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_PORTA_EOI, Value);

  return EFI_SUCCESS;
}

/**
  Get the status of target port's interrupt.

  @param[in]    Die                    Number of Die where gpio is, 0 - max.
  @param[in]    Index                  Gpio group index, 0 - max.
  @param[in]    Port                   Gpio port number in a group, 0 - max.
  @param[out]   Status                 0-none, 1-have.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 - max.

**/
EFI_STATUS
GetGpioInterruptStatus (
  IN  UINT8  Die,
  IN  UINT8  Index,
  IN  UINT8  Port,
  OUT UINT8  *Status
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is incorrect!\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_INTSTATUS);

  *Status = ((Value & BIT(Port)) >> Port);

  return EFI_SUCCESS;
}

/**
  Get the raw status of target port's interrupt.

  @param[in]    Die                    Number of Die where gpio is, 0 - max.
  @param[in]    Index                  Gpio group index, 0 - max.
  @param[in]    Port                   Gpio port number in a group, 0 - max.
  @param[out]   Status                 0-none, 1-have.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 - max.

**/
EFI_STATUS
GetGpioInterruptRAWStatus (
  IN  UINT8  Die,
  IN  UINT8  Index,
  IN  UINT8  Port,
  OUT UINT8  *Status
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is incorrect!\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_RAW_INTSTATUS);

  *Status = ((Value & BIT(Port)) >> Port);

  return EFI_SUCCESS;
}

/**
  When the port is output state, set a specify gpio output level, high or low.

  @param[in]    Die                    Number of Die where gpio is, 0 - max.
  @param[in]    Index                  Gpio group index, 0 - max.
  @param[in]    Port                   Gpio port number in a group, 0 - max.
  @param[in]    Level                  Output level to set, 0-low, 1-high.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 - max.
                                       3.Level to set is not high and low.

**/
EFI_STATUS
SetGpioOutputLevel (
  IN UINT8  Die,
  IN UINT8  Index,
  IN UINT8  Port,
  IN UINT8  Level
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  if ((Level != GPIO_LOW_LEVEL) && (Level != GPIO_HIGH_LEVEL)) {
    DEBUG ((DEBUG_ERROR, "Level to set is not high and low\n"));
    return EFI_INVALID_PARAMETER;
  }

  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_SWPORTA_DR);
  if (Level == GPIO_HIGH_LEVEL) {
    Value |= (0x1 << Port);
  } else {
    Value &= ~(0x1 << Port);
  }
  MmioWrite32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_SWPORTA_DR, Value);

  return EFI_SUCCESS;
}

/**
  When the port is input state, get a specify gpio input level, high or low.

  @param[in]    Die                    Number of Die where gpio is, 0 - max.
  @param[in]    Index                  Gpio group index, 0 - max.
  @param[in]    Port                   Gpio port number in a group, 0 - max.
  @param[out]   Level                  0-low, 1-high.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 - max.

**/
EFI_STATUS
GetGpioInputLevel (
  IN  UINT8  Die,
  IN  UINT8  Index,
  IN  UINT8  Port,
  OUT UINT8  *Level
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Die, Index) + PHYTIUM_GPIO_EXT_PORTA);

  *Level = ((Value & BIT(Port)) >> Port);

  return EFI_SUCCESS;
}

/**
  In a progress, enable a specific gpio's interrupt with specific type.
  1.Set the port direction to input state.
  2.Enable the port interrupt.
  3.Set the port interrupt's trigger type and sensitive type.

  @param[in]    Die                    Number of Die where gpio is, 0 - max.
  @param[in]    Index                  Gpio group index, 0 - max.
  @param[in]    Port                   Gpio port number in a group, 0 - max.
  @param[in]    TriggerType            Trigger type to set, 0-level, 1-edge.
  @param[in]    SensitiveType          Sensitive type to set, 0-low-level or edge-falling,
                                       1-level-high or edge-rising.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 - max.
                                       3.Interrupt type is incorrect.

**/
EFI_STATUS
SetGpioInterruptDirect (
  IN UINT8  Die,
  IN UINT8  Index,
  IN UINT8  Port,
  IN UINT8  TriggerType,
  IN UINT8  SensitiveType
  )
{
  EFI_STATUS  Status;

  //
  //Set gpio direction
  //
  Status = SetGpioDirect (Die, Index, Port, GPIO_DIRECTION_IN);
  if (EFI_ERROR(Status)){
    return Status;
  }

  //
  //Enable gpio interrupt
  //
  GpioInterruptEnable (Die, Index, Port);

  //
  //Set gpio interrupt type
  //
  Status = SetGpioInterruptType (Die, Index, Port, TriggerType, SensitiveType);

  return Status;
}

/**
  In a progress, set a specify gpio output level, high or low.
  1.Set the port direction to output state.
  2.Set the port level to high or low.

  @param[in]    Die                    Number of Die where gpio is, 0 - max.
  @param[in]    Index                  Gpio group index, 0 - max.
  @param[in]    Port                   Gpio port number in a group, 0 - max.
  @param[in]    Level                  Output level to set, 0-low, 1-high.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 - max.
                                       3.Level to set is not high and low.

**/
EFI_STATUS
DirectSetGpioOutPutLevel (
  IN UINT8  Die,
  IN UINT8  Index,
  IN UINT8  Port,
  IN UINT8  Level
  )
{

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  if ((Level != GPIO_LOW_LEVEL) && (Level != GPIO_HIGH_LEVEL)) {
    DEBUG ((DEBUG_ERROR, "Level to set is not high and low\n"));
    return EFI_INVALID_PARAMETER;
  }

  SetGpioDirect (Die, Index, Port, GPIO_DIRECTION_OUT);
  SetGpioOutputLevel (Die, Index, Port, Level);

  return EFI_SUCCESS;
}

/**
  In a progress, get a specify gpio input level, high or low.
  1.Set the port direction to input state.
  2.Get the port level.

  @param[in]    Die                    Number of Die where gpio is, 0 - max.
  @param[in]    Index                  Gpio group index, 0 - max.
  @param[in]    Port                   Gpio port number in a group, 0 - max.
  @param[out]   Level                  0-low, 1-high.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is incorrect.
                                       2.Gpio Port is not in the range 0 - max.

**/
EFI_STATUS
DirectGetGpioInputLevel (
  IN UINT8   Die,
  IN UINT8   Index,
  IN UINT8   Port,
  OUT UINT8  *Level
  )
{
  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - max\n"));
    return EFI_INVALID_PARAMETER;
  }

  SetGpioDirect (Die, Index, Port, GPIO_DIRECTION_IN);
  GetGpioInputLevel (Die, Index, Port, Level);

  return EFI_SUCCESS;
}
