/** @file
  The Differentiated System Description Table.

  Copyright (C) 2022, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <AcpiPlatform.h>

DefinitionBlock("DsdtTable.aml", "DSDT", 2, "PHYLTD", "PHYTDSDT", EFI_ACPI_PHYTIUM_OEM_REVISION) {
  Scope(_SB)
  {
    Method (_OSC, 4, Serialized)  // _OSC: Operating System Capabilities
    {
      CreateDWordField (Arg3, Zero, STS0)
      CreateDWordField (Arg3, 0x04, CAP0)
      If ((Arg0 == ToUUID ("0811b06e-4a27-44f9-8d60-3cbbc22e7b48")))
      {
        //DBGC (0x02, "_OSC OSPM Arg1:", Arg1)
        //DBGC (0x02, "CAP0:", CAP0)
        //DBGC (0x02, "STS0:", STS0)
        If ((Arg1 != One))
        {
          STS0 &= 0xFFFFFFFFFFFFFFE0
          STS0 |= 0x0A
        }
        ElseIf ((CAP0 & 0x0100))
        {
          CAP0 &= 0xFFFFFFFFFFFFFEFF
          STS0 &= 0xFFFFFFFFFFFFFFE0
          STS0 |= 0x12
        }
      }
      Else
      {
        STS0 &= 0xFFFFFFFFFFFFFFE0
        STS0 |= 0x06
      }

      Return (Arg3)
    }
//   Device(PWRB){
// 	  Name(_HID,"PNP0C0C")
// 	  Name(_UID,Zero)     
// 	  Method(_STA,0x0,NotSerialized){
// 		Return(0xF)
// 	  }
//   }
  
//   Device (GED0) {
// 	  Name (_HID, "ACPI0013")
// 	  Name (_UID, Zero)
// 	  Name (_CRS, ResourceTemplate () {
// 	   Interrupt (ResourceConsumer, Edge, ActiveHigh, ExclusiveAndWake) { 37 }
// 	  })

// 	  OperationRegion (PMUX, SystemMemory, 0x380031d4, 0x40)
// 	  Field (PMUX, DWordAcc, NoLock, Preserve){
// 		Offset (0x0),
// 		SMUX, 32,
// 	  }

// 	  OperationRegion (GPIO, SystemMemory, 0x2000a000, 0x40)
// 	  Field (GPIO, DWordAcc, NoLock, Preserve){
// 		Offset (0x0),
// 		PADT,	32,
// 		Offset (0x4),
// 		PADR,	32,
// 		Offset (0x8),
// 		PAIN,	32,
// 		Offset (0x18),
// 		PAIE,	32,
// 		Offset (0x1c),
// 		PAIM,	32,
// 		Offset (0x20),
// 		PAIL,	32,
// 		Offset (0x24),
// 		PAIP,	32,
// 		Offset (0x28),
// 		PAST, 32,
// 		Offset (0x2c),
// 		PARS, 32,
// 		Offset (0x30),
// 		PALS,	32,
// 		Offset (0x34),
// 		PADB,	32,
// 		Offset (0x38),
// 		PCLR,	32,
// 	  }

// 	  Method (_INI) {
// 		/*set pin mux gpio29*/
// 		SMUX = 0x11 
// 		PADR &= 0xdfffffff  //input
// 		PAIE |= 0x20000000  //enable interrupt
// 		PADR |= 0x10000000  //gpio28 output
// 		PADT |= 0x10000000   //gpio28 output high
// 		/*PA_IM |= 0xffffffff*/

// 		/* enable the power button on line 8 as edge active low */
// 		PAIL |= 0x20000000
// 		PAIP &= 0xdfffffff
// 		PADB |= 0x1
// 		PCLR |= 0x1
// 		PAIM &= ~0x20000000
// 		PAIE |= 0x20000000  //enable interrupt
// 	  }
	  
// 	  Method (_STA, 0x0, NotSerialized){
// 		Return (0xF)
// 	  }

// 	  Method (_EVT, 0x1) {
// 		Switch(ToInteger(Arg0)){
// 	      Case(37){
				
// 				Store(PAST,Local0)
// 				And(Local0, 0x20000000, Local0)
// 				Switch(ToInteger(Local0)){
// 				Case(0x20000000){  /*be gpio29 ? */
// 					Notify(\_SB.PWRB, 0x80)   
		
// 					Store(PCLR,Local0)
// 					Or(Local0,0x1,Local0)	//clear interrupt
// 					Store(Local0,PCLR)
// 				}
// 			}
// 			Return
// 	      }
// 		}
// 	  }
// 	}

  }
  include ("Uart.asl")
  include ("Psd.asl")
  include ("Cpc.asl")
  include ("Die0Cpu.asl")
  include ("Die1Cpu.asl")
  include ("Die2Cpu.asl")
  include ("Die3Cpu.asl")
  include ("Die4Cpu.asl")
  include ("Die5Cpu.asl")
  include ("Die6Cpu.asl")
  include ("Die7Cpu.asl")
  include ("ThermalZone.asl")
  include ("Ceu.asl")
  include ("Pmu.asl")
  //include ("Pwrbtn.asl")
  include ("Kcs.asl")
  //include ("I2c.asl")
  //include ("Spi.asl")
}
