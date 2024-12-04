/** @file
  S3 operation interfaces.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef S3_RESUME_LIB_H_
#define S3_RESUME_LIB_H_

VOID
AsmS3SystemOffEntry (
  VOID
  );

VOID
AsmS3SystemResetEntry (
  VOID
  );

VOID
AsmS3SuspendStartEntry (
  VOID
  );

VOID
AsmS3SuspendFinishEntry (
  VOID
  );

VOID
AsmS3SuspendEndEntry (
  VOID
  );

VOID
S3SystemOffEntry (
  UINTN PfdiNum,
  UINTN GdBase
  );

VOID
S3SystemResetEntry (
  UINTN PfdiNum,
  UINTN GdBase
  );

VOID
S3SuspendStartEntry (
  UINTN PfdiNum,
  UINTN GdBase
  );

VOID
S3SuspendFinishEntry (
  UINTN PfdiNum,
  UINTN GdBase
  );

VOID
S3SuspendEndEntry (
  VOID
  );

#endif /* S3_RESUME_LIB_H_ */
