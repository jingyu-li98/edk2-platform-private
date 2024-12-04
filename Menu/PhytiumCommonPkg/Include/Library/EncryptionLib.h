
#ifndef _SCREEN_PRINT_LIB_H_
#define _SCREEN_PRINT_LIB_H_

/**
  This function processes to Encryption password.

  @param[in] PASSWORD_CONFIG_DATA           A pointer to the data off password.


  @retval   VOID

**/
VOID PasswordEncryption(PASSWORD_CONFIG_DATA    *PasswordConfigData);


/**
  This function processes to decode password.

  @param[in] PASSWORD_CONFIG_DATA           A pointer to the data off password.


  @retval   VOID

**/

VOID PasswordDecode(PASSWORD_CONFIG_DATA    *PasswordConfigData);

#endif

