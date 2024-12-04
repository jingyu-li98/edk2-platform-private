#ifndef   _EC_LIB_H_
#define   _EC_LIB_H_

#define   EC_VERSION_MINOR      0x00
#define   EC_VERSION_SENIOR     0x01


UINT8
EcWriteCmd (
  UINT8 Cmd
  );

UINT8
EcWriteData (
  UINT8 Data
  );

UINT8
EcReadData (
  UINT8 *PData
  );

UINT8
EcReadMem (
  UINT8 Index,
  UINT8 *Data
  );

UINT8
EcWriteMem (
  UINT8 Index,
  UINT8 Data
  );

#endif
