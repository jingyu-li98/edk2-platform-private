/** @file
  This file provides basic bit manipulation macro definitions.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef BITS_H_
#define BITS_H_

#define BITS_PER_LONG       32
#define BITS_PER_LONG_LONG  64

#define BIT(n)             (1 << (n))
#define BITS_PER_BYTE       8


#define GENMASK(h, l) \
    ((((UINT32)~0) - ((UINT32)1 << (l)) + 1) & ((UINT32)~0 >> (BITS_PER_LONG - 1 - (h))))

#define GENMASK_ULL(h, l) \
    ((((UINT64)~0) - ((UINT64)1 << (l)) + 1) & \
    ((UINT64)~0 >> (BITS_PER_LONG_LONG - 1 - (h))))

#endif  /* BITS_H_ */
