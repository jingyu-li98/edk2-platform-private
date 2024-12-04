
#ifndef _SCREEN_PRINT_LIB_H_
#define _SCREEN_PRINT_LIB_H_

/**

  Print screen and save as BMP file to USB file system

  @param  None.

  @retval EFI_SUCCESS           Save success.
  @retval EFI_NOT_READY         GOP is not ready.
  @retval EFI_NO_MEDIA          No USB file system.
  @retval EFI_DEVICE_ERROR      Save file or print screen error.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to allocate buffer.

**/
EFI_STATUS
EFIAPI
ScreenPrintToBmp (
  VOID
  );

#endif

