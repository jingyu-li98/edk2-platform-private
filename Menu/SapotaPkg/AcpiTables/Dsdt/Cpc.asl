/** @file
The description of continuous performance control.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope(_SB)
{
  //Channel 0
  Name (CPC0, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0x0,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0x0,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0x0,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0x0,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0x0,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0x0,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0x0,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0x0,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0x0,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0x0,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0x0,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0x0,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0x0,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0x0,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0x0,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0x0,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0x0,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0x0,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0x0,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0x0,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0x0,) },//21
  })

  //Channel 1
  Name (CPC1, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0x1,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0x1,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0x1,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0x1,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0x1,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0x1,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0x1,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0x1,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0x1,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0x1,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0x1,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0x1,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0x1,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0x1,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0x1,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0x1,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0x1,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0x1,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0x1,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0x1,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0x1,) },//21
  })

  //Channel 2
  Name (CPC2, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0x2,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0x2,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0x2,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0x2,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0x2,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0x2,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0x2,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0x2,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0x2,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0x2,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0x2,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0x2,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0x2,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0x2,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0x2,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0x2,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0x2,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0x2,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0x2,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0x2,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0x2,) },//21
  })

  //Channel 3
  Name (CPC3, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0x3,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0x3,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0x3,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0x3,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0x3,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0x3,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0x3,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0x3,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0x3,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0x3,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0x3,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0x3,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0x3,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0x3,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0x3,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0x3,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0x3,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0x3,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0x3,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0x3,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0x3,) },//21
  })

  //Channel 4
  Name (CPC4, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0x4,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0x4,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0x4,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0x4,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0x4,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0x4,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0x4,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0x4,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0x4,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0x4,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0x4,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0x4,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0x4,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0x4,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0x4,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0x4,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0x4,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0x4,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0x4,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0x4,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0x4,) },//21
  })

  //Channel 5
  Name (CPC5, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0x5,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0x5,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0x5,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0x5,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0x5,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0x5,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0x5,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0x5,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0x5,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0x5,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0x5,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0x5,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0x5,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0x5,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0x5,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0x5,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0x5,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0x5,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0x5,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0x5,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0x5,) },//21
  })

  //Channel 6
  Name (CPC6, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0x6,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0x6,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0x6,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0x6,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0x6,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0x6,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0x6,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0x6,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0x6,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0x6,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0x6,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0x6,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0x6,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0x6,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0x6,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0x6,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0x6,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0x6,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0x6,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0x6,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0x6,) },//21
  })

  //Channel 7
  Name (CPC7, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0x7,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0x7,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0x7,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0x7,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0x7,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0x7,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0x7,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0x7,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0x7,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0x7,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0x7,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0x7,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0x7,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0x7,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0x7,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0x7,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0x7,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0x7,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0x7,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0x7,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0x7,) },//21
  })

  //Channel 8
  Name (CPC8, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0x8,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0x8,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0x8,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0x8,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0x8,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0x8,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0x8,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0x8,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0x8,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0x8,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0x8,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0x8,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0x8,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0x8,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0x8,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0x8,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0x8,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0x8,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0x8,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0x8,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0x8,) },//21
  })

  //Channel 9
  Name (CPC9, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0x9,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0x9,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0x9,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0x9,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0x9,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0x9,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0x9,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0x9,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0x9,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0x9,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0x9,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0x9,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0x9,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0x9,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0x9,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0x9,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0x9,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0x9,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0x9,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0x9,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0x9,) },//21
  })

  //Channel 10
  Name (CPCA, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0xA,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0xA,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0xA,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0xA,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0xA,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0xA,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0xA,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0xA,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0xA,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0xA,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0xA,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0xA,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0xA,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0xA,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0xA,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0xA,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0xA,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0xA,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0xA,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0xA,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0xA,) },//21
  })

  //Channel 11
  Name (CPCB, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0xB,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0xB,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0xB,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0xB,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0xB,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0xB,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0xB,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0xB,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0xB,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0xB,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0xB,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0xB,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0xB,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0xB,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0xB,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0xB,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0xB,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0xB,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0xB,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0xB,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0xB,) },//21
  })

  //Channel 12
  Name (CPCC, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0xC,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0xC,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0xC,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0xC,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0xC,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0xC,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0xC,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0xC,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0xC,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0xC,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0xC,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0xC,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0xC,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0xC,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0xC,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0xC,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0xC,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0xC,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0xC,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0xC,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0xC,) },//21
  })

  //Channel 13
  Name (CPCD, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0xD,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0xD,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0xD,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0xD,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0xD,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0xD,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0xD,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0xD,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0xD,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0xD,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0xD,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0xD,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0xD,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0xD,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0xD,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0xD,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0xD,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0xD,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0xD,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0xD,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0xD,) },//21
  })

  //Channel 14
  Name (CPCE, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0xE,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0xE,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0xE,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0xE,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0xE,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0xE,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0xE,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0xE,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0xE,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0xE,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0xE,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0xE,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0xE,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0xE,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0xE,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0xE,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0xE,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0xE,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0xE,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0xE,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0xE,) },//21
  })

  //Channel 15
  Name (CPCF, Package ()  // _CPC: Continuous Performance Control
    {   0x17,0x03,
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000000, 0xF,) },//1
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000004, 0xF,) },//2
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000008, 0xF,) },//3
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000000c, 0xF,) },//4
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000010, 0xF,) },//5
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000014, 0xF,) },//6
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000018, 0xF,) },//7
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000001c, 0xF,) },//8
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000020, 0xF,) },//9
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000024, 0xF,) },//10
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000028, 0xF,) },//11
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000030, 0xF,) },//12
    ResourceTemplate ()
    {   Register (PCC, 0x40,0x00, 0x0000000000000038, 0xF,) },//13
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000040, 0xF,) },//14
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000044, 0xF,) },//15
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000048, 0xF,) },//16
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000004c, 0xF,) },//17
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000050, 0xF,) },//18
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000054, 0xF,) },//19
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x0000000000000058, 0xF,) },//20
    ResourceTemplate ()
    {   Register (PCC, 0x20,0x00, 0x000000000000005c, 0xF,) },//21
  })
}
