/* @file
** DcCore.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DC_CORE_H_
#define    _DC_CORE_H_

#include "../PhyGopDxe.h"

VOID
CoreInit (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num
  );

VOID
ConfClearSet (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           IsClear,
  IN     UINT32            ClearValue
  );

VOID
ConfDisplaySetHorizontal (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            LinePixels,
  IN     UINT32            TotalPixels,
  IN     UINT32            HsyncStart,
  IN     UINT32            HsyncEnd,
  IN     BOOLEAN           HsyncPolarity
  );

VOID
ConfDisplaySetVertical (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            LinePixels,
  IN     UINT32            TotalPixels,
  IN     UINT32            VsyncStart,
  IN     UINT32            VsyncEnd,
  IN     BOOLEAN           VsyncPolarity
  );

VOID
ConfFramebufferSetConfig (
  IN OUT PHY_PRIVATE_DATA  Private,
  IN     UINT8             Num,
  IN     VIDEO_FORMAT      Format,
  IN     PHY_TILING_TYPE   Tiling
  );

VOID
ConfFramebufferSetStride (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            Stride
  );

VOID
ConfFramebufferSetAddress (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT64            Addr
  );

VOID
ConfFramebufferReset (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
ConfFramebufferSetFramebuffer (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     DC_FRAMEBUFFER    *Framebuffer
  );

VOID
DcConfOverlayCommit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
ConfOutputEnable (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           Enable
  );

VOID
ConfOutputSelect (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     PHY_OUTPUT_TYPE   Output_type
  );

VOID
ConfOutputDpformat (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     DP_FORMAT         Format
  );

VOID
ConfPanelSetConfig (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           DepPolarity,
  IN     BOOLEAN           DapPolarity,
  IN     BOOLEAN           ClockpPolarity
  );

VOID
ConfDitherEnable (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           Enable
  );

VOID
ConfGammaEnable (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN UINT8                 Num,
  IN BOOLEAN               Enable
  );

VOID
ConfGammaSetConfig (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            Index,
  IN     UINT32            Red,
  IN     UINT32            Green,
  IN     UINT32            Blue
  );

VOID
ConfCursorEnable (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           Enable
  );

VOID
ConfCursorSetType (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     PHY_CURSOR_TYPE   Type
  );

VOID
ConfCursorSetHotspot (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            X,
  IN     UINT32            Y
  );

VOID
ConfCursorSetColor (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            Bg,
  IN     UINT32            Fg
  );

VOID
ConfCursorSetAddress (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT64            Addr
  );

VOID
ConfCursorSetPos (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            X,
  IN     UINT32            Y
  );

VOID
ConfCommit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
ConfInterruptEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN BOOLEAN           Enable
  );

UINT32
ConfInterruptGet (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
ConfInfoGet (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num
  );

VOID
DcConfOverlay (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     DC_OVERLAY        *Overlay
  );

VOID
ConfShadowRegisterPendingEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN BOOLEAN           Enable
  );

UINT32
ConfFlipInProgressValue (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

UINT32
ConfFrameCounterGet (
  PHY_PRIVATE_DATA  *Private,
  UINT8             Num
  );

UINT32
CoreCommit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

UINT32
DcConfigInit (
  IN OUT PHY_PRIVATE_DATA   *Private,
  IN     UINT8              Num,
  IN     PHY_GOP_MODE       *GopMode
  );
#endif
