/** @file
The description of thermalzone. Each die has one temperature sensors. 8 die has
8 temperature sensors. A temperature sensor corrersponds to a thermalzone.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/PcdLib.h>

#define  METHOD_TMP(MHU_BASE, SHMEM_BASE, SENSOR_ID)                     \
    Method (_TMP) {                                                      \
      /*MHU Address*/                                                    \
      OperationRegion(MHUB, SystemMemory, MHU_BASE, 0x1000)              \
      Field(MHUB, DWordAcc, NoLock, Preserve){                           \
        Offset (0x20),                                                   \
        SUST, 32,           /*0x20 : SCP_UBOOT_STAT*/                    \
        Offset (0x28),                                                   \
        SUSE, 32,           /*0x28 : SCP_UBOOT_SET*/                     \
        Offset (0x30),                                                   \
        SUCL, 32,           /*0x30 : SCP_UBOOT_CLEAR*/                   \
        Offset (0x120),                                                  \
        AUST, 32,           /*0x120 : AP_UBOOT_STAT*/                    \
        Offset (0x128),                                                  \
        AUSE, 32,           /*0x128 : AP_UBOOT_SET*/                     \
        Offset (0x130),                                                  \
        AUCL, 32,           /*0x130 : AP_UBOOT_CLEAR*/                   \
        Offset (0x508),                                                  \
        AOCG, 32,           /*0x508 : AP_OS_CONFIG*/                     \
        AUCG, 32,           /*0x50C : AP_UBOOT_CONFIG*/                  \
      }                                                                  \
      /*Share Memory Address Uboot channel*/                             \
      OperationRegion(SHRM, SystemMemory, SHMEM_BASE, 0x1000)            \
      Field(SHRM, DWordAcc, NoLock, Preserve){                           \
        RESA, 32,           /*Reserved A*/                               \
        STAT, 32,           /*Status*/                                   \
        RESB, 32,           /*Reserved B*/                               \
        RESC, 32,           /*Reserved C*/                               \
        FLAG, 32,           /*Flags*/                                    \
        LENG, 32,           /*Length*/                                   \
        HEAD, 32,           /*Message Header*/                           \
        PAY0, 32,           /*Payload[0]*/                               \
        PAY1, 32,           /*Payload[1]*/                               \
        PAY2, 32,           /*Payload[2]*/                               \
      }                                                                  \
      Store (SUST, Local0)                                               \
      Local0 &= 0x1                                                      \
      Local1 = 0x1F4                /*timeout : 500x10us*/               \
      while (LGreater (Local1, 0x0)) {                                   \
        Store (SUST, Local0)                                             \
        Local0 &= 0x1                                                    \
        If (LEqual (Local0, 0x0)) {  /*Check channel clear*/             \
          break;                                                         \
        }                                                                \
        If (LEqual (Local1, 0x0)) {                                      \
          return (0xFFFFFFFF)                                            \
        }                                                                \
        Local1 -= 1                                                      \
        Sleep (10)                                                       \
      }                                                                  \
      Store (0x1, AUCG)                                                  \
      Store (12, LENG)        /*Length = sizeof (MessageHeader) + 8*/    \
      Store (0, FLAG)         /*Flags = 0*/                              \
      Store ((0x15 << 10) | 0x6, HEAD)  /*Protocol ID 0x15, Message ID 0x6*/  \
      Store (SENSOR_ID, PAY0)         /*Sensor ID*/                      \
      Store (0, PAY1)         /*sync*/                                   \
      Store (STAT, Local0)                                               \
      Local0 &= ~(0x1)                                                   \
      Store (Local0, STAT)    /*Mark channel busy*/                      \
      Sleep (10)                                                         \
      Store ((0x15 << 10) | 0x6, AUSE)                                   \
      Sleep (10)                                                         \
      Local1 = 0x1F4                       /*timeout : 500x10 us*/       \
      While (LGreater (Local1, 0x0)) {                                   \
       Store (STAT, Local2)                                              \
       Local2 &= 0x1                                                     \
       If (LEqual (Local2, 0x1)) {                                       \
         break;                                                          \
       }                                                                 \
       If (LEqual (Local1, 0x0)) {                                       \
         Store (0xFFFFFFFF, SUCL)               /*clear channel*/        \
         return (0xFFFFFFFF)                                             \
       }                                                                 \
       Local1 -= 1                                                       \
       Sleep (10)                                                        \
      }                                                                  \
      Store (PAY0, Local3)                                               \
      If (LNotEqual (Local3, 0x0)) {                                     \
        Return (Local3)                                                  \
      }                                                                  \
      Store (PAY1, Local4)                                               \
      Store (0xFFFFFFFF, SUCL)               /*clear channel*/           \
      Return (_C2K (Local4))                                             \
    }

Scope (_SB)
{
  //die 0
  ThermalZone (TZ00) {
    Method (_PSV) { Return (_C2K(70)) }
    Method (_TSP) { Return (300) }
    Name (CRTT, FixedPcdGet64 (PcdDefaultPowerDownValue))
    Name (_PSL, Package (){
      \_SB.CL00.CP00,
      \_SB.CL00.CP01,
      \_SB.CL01.CP02,
      \_SB.CL01.CP03,
      \_SB.CL02.CP04,
      \_SB.CL02.CP05,
      \_SB.CL03.CP06,
      \_SB.CL03.CP07,
      \_SB.CL04.CP08,
      \_SB.CL04.CP09,
      \_SB.CL05.CP0A,
      \_SB.CL05.CP0B,
      \_SB.CL06.CP0C,
      \_SB.CL06.CP0D,
      \_SB.CL07.CP0E,
      \_SB.CL07.CP0F,
    })
    Method (_CRT) { Return (_C2K(CRTT)) }
    Method (_TZP) { Return (600) }
    //_TMP
    METHOD_TMP(0x38001000, 0x38004C00, 1)
  }

  //die 1
  ThermalZone (TZ10) {
    Method (_PSV) { Return (_C2K(70)) }
    Method (_TSP) { Return (300) }
    Name (CRTT, FixedPcdGet64 (PcdDefaultPowerDownValue))
    Name (_PSL, Package (){
      \_SB.CL10.CP10,
      \_SB.CL10.CP11,
      \_SB.CL11.CP12,
      \_SB.CL11.CP13,
      \_SB.CL12.CP14,
      \_SB.CL12.CP15,
      \_SB.CL13.CP16,
      \_SB.CL13.CP17,
      \_SB.CL14.CP18,
      \_SB.CL14.CP19,
      \_SB.CL15.CP1A,
      \_SB.CL15.CP1B,
      \_SB.CL16.CP1C,
      \_SB.CL16.CP1D,
      \_SB.CL17.CP1E,
      \_SB.CL17.CP1F,
    })
    Method (_CRT) { Return (_C2K(CRTT)) }
    Method (_TZP) { Return (600) }
    //_TMP
    METHOD_TMP(0x100038001000, 0x100038004C00, 1)
  }

  //die 2
  ThermalZone (TZ20) {
    Method (_PSV) { Return (_C2K(70)) }
    Method (_TSP) { Return (300) }
    Name (CRTT, FixedPcdGet64 (PcdDefaultPowerDownValue))
    Name (_PSL, Package (){
      \_SB.CL20.CP20,
      \_SB.CL20.CP21,
      \_SB.CL21.CP22,
      \_SB.CL21.CP23,
      \_SB.CL22.CP24,
      \_SB.CL22.CP25,
      \_SB.CL23.CP26,
      \_SB.CL23.CP27,
      \_SB.CL24.CP28,
      \_SB.CL24.CP29,
      \_SB.CL25.CP2A,
      \_SB.CL25.CP2B,
      \_SB.CL26.CP2C,
      \_SB.CL26.CP2D,
      \_SB.CL27.CP2E,
      \_SB.CL27.CP2F,
    })
    Method (_CRT) { Return (_C2K(CRTT)) }
    Method (_TZP) { Return (600) }
    //_TMP
    METHOD_TMP(0x200038001000, 0x200038004C00, 1)
  }

  //die 3
  ThermalZone (TZ30) {
    Method (_PSV) { Return (_C2K(70)) }
    Method (_TSP) { Return (300) }
    Name (CRTT, FixedPcdGet64 (PcdDefaultPowerDownValue))
    Name (_PSL, Package (){
      \_SB.CL30.CP30,
      \_SB.CL30.CP31,
      \_SB.CL31.CP32,
      \_SB.CL31.CP33,
      \_SB.CL32.CP34,
      \_SB.CL32.CP35,
      \_SB.CL33.CP36,
      \_SB.CL33.CP37,
      \_SB.CL34.CP38,
      \_SB.CL34.CP39,
      \_SB.CL35.CP3A,
      \_SB.CL35.CP3B,
      \_SB.CL36.CP3C,
      \_SB.CL36.CP3D,
      \_SB.CL37.CP3E,
      \_SB.CL37.CP3F,
    })
    Method (_CRT) { Return (_C2K(CRTT)) }
    Method (_TZP) { Return (600) }
    //_TMP
    METHOD_TMP(0x300038001000, 0x300038004C00, 1)
  }

  //die 4
  ThermalZone (TZ40) {
    Method (_PSV) { Return (_C2K(70)) }
    Method (_TSP) { Return (300) }
    Name (CRTT, FixedPcdGet64 (PcdDefaultPowerDownValue))
    Name (_PSL, Package (){
      \_SB.CL40.CP40,
      \_SB.CL40.CP41,
      \_SB.CL41.CP42,
      \_SB.CL41.CP43,
      \_SB.CL42.CP44,
      \_SB.CL42.CP45,
      \_SB.CL43.CP46,
      \_SB.CL43.CP47,
      \_SB.CL44.CP48,
      \_SB.CL44.CP49,
      \_SB.CL45.CP4A,
      \_SB.CL45.CP4B,
      \_SB.CL46.CP4C,
      \_SB.CL46.CP4D,
      \_SB.CL47.CP4E,
      \_SB.CL47.CP4F,
    })
    Method (_CRT) { Return (_C2K(CRTT)) }
    Method (_TZP) { Return (600) }
    //_TMP
    METHOD_TMP(0x400038001000, 0x400038004C00, 1)
  }

  //die 5
  ThermalZone (TZ50) {
    Method (_PSV) { Return (_C2K(70)) }
    Method (_TSP) { Return (300) }
    Name (CRTT, FixedPcdGet64 (PcdDefaultPowerDownValue))
    Name (_PSL, Package (){
      \_SB.CL50.CP50,
      \_SB.CL50.CP51,
      \_SB.CL51.CP52,
      \_SB.CL51.CP53,
      \_SB.CL52.CP54,
      \_SB.CL52.CP55,
      \_SB.CL53.CP56,
      \_SB.CL53.CP57,
      \_SB.CL54.CP58,
      \_SB.CL54.CP59,
      \_SB.CL55.CP5A,
      \_SB.CL55.CP5B,
      \_SB.CL56.CP5C,
      \_SB.CL56.CP5D,
      \_SB.CL57.CP5E,
      \_SB.CL57.CP5F,
    })
    Method (_CRT) { Return (_C2K(CRTT)) }
    Method (_TZP) { Return (600) }
    //_TMP
    METHOD_TMP(0x500038001000, 0x500038004C00, 1)
  }

  //die 6
  ThermalZone (TZ60) {
    Method (_PSV) { Return (_C2K(70)) }
    Method (_TSP) { Return (300) }
    Name (CRTT, FixedPcdGet64 (PcdDefaultPowerDownValue))
    Name (_PSL, Package (){
      \_SB.CL60.CP60,
      \_SB.CL60.CP61,
      \_SB.CL61.CP62,
      \_SB.CL61.CP63,
      \_SB.CL62.CP64,
      \_SB.CL62.CP65,
      \_SB.CL63.CP66,
      \_SB.CL63.CP67,
      \_SB.CL64.CP68,
      \_SB.CL64.CP69,
      \_SB.CL65.CP6A,
      \_SB.CL65.CP6B,
      \_SB.CL66.CP6C,
      \_SB.CL66.CP6D,
      \_SB.CL67.CP6E,
      \_SB.CL67.CP6F,
    })
    Method (_CRT) { Return (_C2K(CRTT)) }
    Method (_TZP) { Return (600) }
    //_TMP
    METHOD_TMP(0x600038001000, 0x600038004C00, 1)
  }

  //die 7
  ThermalZone (TZ70) {
    Method (_PSV) { Return (_C2K(70)) }
    Method (_TSP) { Return (300) }
    Name (CRTT, FixedPcdGet64 (PcdDefaultPowerDownValue))
    Name (_PSL, Package (){
      \_SB.CL70.CP70,
      \_SB.CL70.CP71,
      \_SB.CL71.CP72,
      \_SB.CL71.CP73,
      \_SB.CL72.CP74,
      \_SB.CL72.CP75,
      \_SB.CL73.CP76,
      \_SB.CL73.CP77,
      \_SB.CL74.CP78,
      \_SB.CL74.CP79,
      \_SB.CL75.CP7A,
      \_SB.CL75.CP7B,
      \_SB.CL76.CP7C,
      \_SB.CL76.CP7D,
      \_SB.CL77.CP7E,
      \_SB.CL77.CP7F,
    })
    Method (_CRT) { Return (_C2K(CRTT)) }
    Method (_TZP) { Return (600) }
    //_TMP
    METHOD_TMP(0x700038001000, 0x700038004C00, 1)
  }

  //-55 to 200
  Method (_C2K, 1, Serialized) {
    And (Arg0 >> 31, 0x1, Local3)
    If (LNotEqual(Local3, 0)) {
      And (Arg0, 0xFFFFFFFF, Local1)
      SubTract (0xFFFFFFFF, Local1, Local1)
      Add (Local1, 1, Local2)
      Subtract (0xAAC, Multiply (Local2, 0xA), Local0)
    } Else {
      Add (Multiply (Arg0, 0x0A), 0xAAC, Local0)
    }
    If (LLessEqual (Local0, 0x0886)) {
      Store (0x0886, Local0)
    } Else {
      If (LGreater (Local0, 0x127C)) {
        Store (0x127C, Local0)
      }
    }
    Return (Local0)
  }
}
