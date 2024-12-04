/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DP_FUN_H_
#define    _DP_FUN_H_

/**
  This 8-bit value contains information about the main link video stream clocking and color representation.
  The clocking mode is selected with bit 0 of the MAIN_STREAM_MISC0 register in the Main Stream Attributes
  block. When set to a '1', the link and stream clock are synchronous in which case the MVid and NVid values
  are a constant.In synchronous clock mode, the transmitter core uses the MVid and NVid register values
  programmed by the host processor via the APB interface.When bit 0 of the MAIN_STREAM_MISC0 register is set
  to a '0', then asynchronous clock mode is enabled and the relationship between MVid and NVid are not fixed.
  In this mode, the transmitter core will transmit a fixed value for NVid and the MVid value provided as a
  part of the clocking interface.
  bit 7:5 BIT_DEPTH: number of bits per color
          000 = 6 bits
          001 = 8 bits
          010 = 10 bits
  bit 2:1 COMPONENT_FORMAT: color representation format
          00 = RGB
          01 = YCbCr 4:2:2
          10 = YCbCr 4:4:4
          11 = Reserved
  bit 0   SYNCHRONOUS_CLOCK: clocking mode for the user data
          0 = asynchronous clock
          1 = synchronous clock

  @param[in]  BitDepth    Bit depth per color.For example , 6 means that Bit depth per color is 6 bits.
  @param[in]  CFormat     Color representation format. 0 - RGB , 1 - YCbCr 4:2:2 , 2 - YCbCr 4:4:4 , other - Reserved.
  @param[in]  ClockMode   Clocking mode for the user data. 0 - asynchronous clock , 1 - synchronous clock.

  @retval     Value of MAIN_STREAM_MISC0 register
**/
UINT8
SetMainStreamMisc0 (
  IN UINT8  BitDepth,
  IN UINT8  CFormat,
  IN UINT8  ClockMode
  );

/**
  Calculate the value of MVID.It is effective in the asynchronous clock mode.The clocking values for the main stream
  video must be calculated by the host processor and provided in this register for transmission to the sink device.
  For most applications, this value can be set to the user data pixel clock value in KHz.
  MVID = pixel clock(MHz) * 100

  @param[in]  PixelClock     pixel clock in MHz unit.

  @retval     The value of MVID register.
**/
UINT32
SetMVID (
  IN UINT32  PixelClock
  );

/**
  Calculate the value of TRANSFER_UNIT_CONFIG_SRC_0 register.A transfer unit is a DisplayPort packet and represents
  both valid data symbols and stuffing symbols. This register value sets the size of a transfer unit in the transmitter
  framing logic.Due to the design of the core, only values that are a multiple of 4 are supported by this register.
  When configured for the Sparse TU mode, only the TRANSFER_UNIT_SIZE field of this register is valid. All other fields
  are ignored.
  bit 27:24   FRAC_SYMBOLS_PER_TU: sets the fractional number of valid symbols per TU in
              units of 1/16th. An accumulator is used to increase the number of data per transaction unit by 1 every 1/N TUs.
  bit 23:16   SYMBOLS_PER_TU: configures the number of valid symbols to transmit with each transaction unit. For most
              applications, this register can be set to a value of 64 (0x40) which allows the core to automatically manage
              the TU rate control. For bandwidth limited applications, this value can be set to any number less than or equal
              to 64.
  bit 6:0     TRANSFER_UNIT_SIZE: this number must be in the range of 32 to 64 for DisplayPort compliance and is typically
              set to a fixed value which depends on the inbound video mode.Larger values should be used for video modes with
              slower pixel clock rates. Smaller values should be used for video modes with higher pixel clock rates. The size
              of the TU is limited to integer multiples of 4. Settings that are not a multiple of 4 will be truncated to the
              lowest multiple.
  valid symbols per TU = tuSize * (pixelclock * bitDepth*3/8)/(lanecount*linkRate*100)

  @param[in]  PixelClock     pixel clock in KHz unit.
  @param[in]  LinkRate       Main link bandwidth in 10 Mbps unit.
  @param[in]  BitDepth       Bit depth per color.For example , 6 means that Bit depth per color is 6 bits.
  @param[in]  LaneCount      The number of lanes , 1.
  @param[in]  TuSize         Transfer unit size.This parameter is usually 64.

  @retval     The value of TRANSFER_UNIT_CONFIG_SRC_0 register.
**/
UINT32
SetTransUintSRC0 (
  IN UINT32 PixelClock,
  IN UINT32 LinkRate,
  IN UINT8  BitDepth,
  IN UINT32 LaneCount,
  IN UINT8  TuSize
  );

/**
  Calculate the value of NVID.The second clocking value for the Main Stream Attributes is set based upon the link rate. This
  value, when used in conjunction with the M-VID value allows the sink device to recover the  frequency of the user data pixel
  clock.
  NVID = linkRate(Gbps) * 10000

  @param[in]  linkRate     Main link bandwidth in 10Mbps unit, 162, 270, 540 or 810.

  @retval     The value of NVID register.
**/
UINT32
SetNVID (
  IN UINT32  LinkRate
  );

/**
  Calculate the value of USER_DATA_COUNT register.Determines the total data count for the transmitter framing logic to read
  from the user FIFO before sending a blanking start symbol. In other words, this value is the total number of 16-bit
  words in a line of active data. The calculated value should be rounded up.
  SYMBOL_COUNT = ((HRES * bits per pixel) + 7) / 8
  USER_DATA_COUNT = (SYMBOL_COUNT + lane_count - 1) / lane_count

  @param[in]  hres       Horizontal resolution of the main stream video source.

  @param[in]  bitDepth   Bit depth per color.For example , 6 means that Bit depth per color is 6 bits.

  @param[in]  laneCount  The number of lanes , 1.

  @retval     The value of USER_DATA_COUNT register.
**/
UINT32
SetUserDataCount (
  IN UINT32  Hres,
  IN UINT8   BitDepth,
  IN UINT32  LaneCount
  );

#endif
