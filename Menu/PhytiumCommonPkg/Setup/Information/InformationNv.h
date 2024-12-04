/** @file

This is a device manager driver might export data to the HII protocol to be
later utilized by the Setup Protocol.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef _INFORMATION_NV_H_
#define _INFORMATION_NV_H_

#define INFORMATION_SET_FORMSET_GUID { \
  0x05b69b9d, 0xbfae, 0x8cc2,{ 0xe7, 0x38, 0x5e, 0x7f, 0x9d, 0x99, 0xbc, 0xef } \
}

#ifdef LS7A1000
#define MAX_SATA_COUNT           0x3
#else
#define MAX_SATA_COUNT           0x4
#endif

#endif
