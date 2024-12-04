/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __EC_H__
#define __EC_H__
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/EcLib.h>

#define  EC_SC    0x66
#define  EC_DATA  0x62

//ACPI Embedded controller commands
#define SMC_READ_EC  0x80 //Read EC
#define SMC_WRITE_EC 0x81 //Write EC
#define SUCCESS 1

#endif
