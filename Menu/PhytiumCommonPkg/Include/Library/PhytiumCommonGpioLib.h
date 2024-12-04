/** @file
  Some devinitaions of Phytium GPIO controller.

  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef PHYTIUM_COMMON_GPIO_H_
#define PHYTIUM_COMMON_GPIO_H_

#define  PHYTIUM_GPIO_REGISTER_BASE          (UINTN)FixedPcdGet64 (PcdGpioControllerBase)
#define  PHYTIUM_DIE_OFFSET                  (UINTN)FixedPcdGet64 (PcdDieOffset)
#define  PHYTIUM_GPIO_REGISTER_OFFSET        0x1000
#define  PHYTIUM_GPIO_N_BASE(Die, Index)     (PHYTIUM_GPIO_REGISTER_BASE + ((UINT64) Die << PHYTIUM_DIE_OFFSET) + Index * PHYTIUM_GPIO_REGISTER_OFFSET)

#define  GPIO_MAX_INDEX_NUM                  (UINTN)FixedPcdGet64 (PcdGpioMaxIndexNumber)
#define  GPIO_MAX_PORT_NUM                   (UINTN)FixedPcdGet64 (PcdGpioMaxPortNumber)

#define  PHYTIUM_GPIO_SWPORTA_DR             (0x0)
#define  PHYTIUM_GPIO_SWPORTA_DDR            (0x4)
#define  PHYTIUM_GPIO_EXT_PORTA              (0x8)
#define  PHYTIUM_GPIO_INTEN                  (0x18)
#define  PHYTIUM_GPIO_INTMASK                (0x1C)
#define  PHYTIUM_GPIO_INTTYPE_LEVEL          (0x20)
#define  PHYTIUM_GPIO_INT_POLARITY           (0x24)
#define  PHYTIUM_GPIO_INTSTATUS              (0x28)
#define  PHYTIUM_GPIO_RAW_INTSTATUS          (0x2C)
#define  PHYTIUM_GPIO_LS_SYNC                (0x30)
#define  PHYTIUM_GPIO_DEBOUNCE               (0x34)
#define  PHYTIUM_GPIO_PORTA_EOI              (0x38)

#define  GPIO_DIRECTION_IN                    0
#define  GPIO_DIRECTION_OUT                   1

#define  GPIO_INTERRUPT_LEVEL                 0
#define  GPIO_INTERRUPT_EDGE                  1

#define  GPIO_INTERRUPT_TRIGGER_EDGE_FALLING  0
#define  GPIO_INTERRUPT_TRIGGER_EDGE_RISING   1

#define  GPIO_INTERRUPT_TRIGGER_LEVEL_LOW     0
#define  GPIO_INTERRUPT_TRIGGER_LEVEL_HIGH    1

#define  GPIO_LOW_LEVEL                  0
#define  GPIO_HIGH_LEVEL                 1

#define  BIT(N)                          (0x1 << N)


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
  );

/**
  When the port is output state, set a specify gpio output level, high or low.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.
  @param[in]    Level    Output level to set, 0-low, 1-high.

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
  );

/**
  When the port is input state, get a specify gpio input level, high or low.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.
  @param[out]   Level    0-low, 1-high.

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
  );
/**
  Enable the interrupt of target port, the port must be input mode.

  @param[in]    Index    Gpio group index, 0.
  @param[in]    Port     Gpio port number in a group, 0-31.

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
  );

/**
  Set the trigger type and sensitive type of interrupt.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.
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
  );

/**
  Mask the interrupt of target port.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.

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
  );

/**
  Clear the interrupt of target port.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.

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
  );

/**
  Get the status of target port's interrupt.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.
  @param[out]   Status   0-none, 1-have.

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
  );


/**
  Get the raw status of target port's interrupt.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.
  @param[out]   Status   0-none, 1-have.

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
  );

/**
  In a progress, enable a specific gpio's interrupt with specific type.
  1.Set the port direction to input state.
  2.Enable the port interrupt.
  3.Set the port interrupt's trigger type and sensitive type.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.
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
  );


/**
  In a progress, set a specify gpio output level, high or low.
  1.Set the port direction to output state.
  2.Set the port level to high or low.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.
  @param[in]    Level    Output level to set, 0-low, 1-high.

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
  );

/**
  In a progress, get a specify gpio input level, high or low.
  1.Set the port direction to input state.
  2.Get the port level.

  @param[in]    Die      Number of Die where gpio is, 0 - max.
  @param[in]    Index    Gpio group index, 0 - max.
  @param[in]    Port     Gpio port number in a group, 0 - max.
  @param[out]   Level    0-low, 1-high.

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
  );

#endif
