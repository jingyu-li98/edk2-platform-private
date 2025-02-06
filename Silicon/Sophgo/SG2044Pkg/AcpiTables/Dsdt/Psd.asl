/** @file
The description of power state dependencies.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope(_SB)
{
  NAME (PSD0, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0x0,               //domainid 0
      0xFD,
      0x08               //cpu num @this domain
    }
  })

  NAME (PSD1, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0x1,                //domainid 1
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSD2, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0x2,                //domainid 2
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSD3, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0x3,                //domainid 3
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSD4, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0x4,                //domainid 4
      0xFD,
      0x08                //cpu num @this domain
    }
  })
  NAME (PSD5, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0x5,                //domainid 5
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSD6, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0x6,                //domainid 6
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSD7, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0x7,                //domainid 7
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSD8, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0x8,                //domainid 8
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSD9, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0x9,                //domainid 9
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSDA, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0xA,                //domainid 10
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSDB, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0xB,                //domainid 11
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSDC, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0xC,                //domainid 12
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSDD, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0xD,                //domainid 13
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSDE, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0xE,                //domainid 14
      0xFD,
      0x08                //cpu num @this domain
    }
  })

  NAME (PSDF, Package ()  // _PSD: Power State Dependencies
  {
    Package (0x05)
    {
      0x05,
      Zero,
      0xF,                //domainid 15
      0xFD,
      0x08                //cpu num @this domain
    }
  })
}
