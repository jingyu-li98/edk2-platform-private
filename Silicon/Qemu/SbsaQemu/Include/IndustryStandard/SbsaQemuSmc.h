/** @file
*
*  Copyright (c) 2023, Linaro Ltd. All rights reserved.<BR>
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#ifndef SBSA_QEMU_SMC_H_
#define SBSA_QEMU_SMC_H_

#include <IndustryStandard/ArmStdSmc.h>

#define SIP_SVC_VERSION  SMC_SIP_FUNCTION_ID(1)
#define SIP_SVC_GET_GIC  SMC_SIP_FUNCTION_ID(100)
#define SIP_SVC_GET_GIC_ITS  SMC_SIP_FUNCTION_ID(101)

#endif /* SBSA_QEMU_SMC_H_ */
