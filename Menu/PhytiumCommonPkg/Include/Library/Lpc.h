#ifndef _EFI_LPC_H_
#define _EFI_LPC_H_
#include <Library/DevicePathLib.h>

#pragma pack(1)
//
// HII specific Vendor Device Path definition.
//
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} LPC_VENDOR_DEVICE_PATH;
#pragma pack()


#define KEY_BOARD_VENDOR_GUID \
{ 0x4306bc67, 0x67dd, 0x44f6, {0x89, 0x1, 0xc5, 0x16, 0xc9, 0x35, 0xf3, 0x9a }}


LPC_VENDOR_DEVICE_PATH  mKeyboardVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    KEY_BOARD_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    { 
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};



//
// Define the LPC protocol GUID
//
#define EFI_LPC_PROTOCOL_GUID \
  { \
    0x1156efc6, 0xaa32, 0x5596, 0xb5, 0xd5, 0x26, 0x93, 0x2e, 0x83, 0xc3, 0x13 \
  }

//
// Extern the GUID for protocol users.
//
extern EFI_GUID                   gEfiLpcProtocolGuid;

struct _EFI_LPC_PROTOCOL {
  UINTN LPC_BASE;
};

typedef struct _EFI_LPC_PROTOCOL  EFI_LPC_PROTOCOL;

#endif
