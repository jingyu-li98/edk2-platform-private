/** @file
Define some base function about Silicon.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef SILICON_LIB_H_
#define SILICON_LIB_H_

/**
  Find the core mask bit according MPIDR.

  @param[in]    Mpidr    MPIDR of core.

  @retval       Mask Bit Index of core map.
**/
UINT32
FindMapMask (
  IN UINT64  Mpidr
  );

#endif
