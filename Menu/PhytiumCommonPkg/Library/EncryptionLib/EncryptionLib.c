
#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <PasswordConfigData.h>

#define ENCRYP_ENABLE  1

typedef struct {
  UINTN     Nk;            // Number of Cipher Key (in 32-bit words);
  UINT32    EncKey[60];    // Expanded AES encryption key
  UINT32    DecKey[60];    // Expanded AES decryption key (Not used here)
} AES_KEY;

UINT8 Aes128CbcKey[] = {
  0xc2, 0x86, 0x69, 0x6d, 0x88, 0x7c, 0x9a, 0xa0, 0x61, 0x1b, 0xbb, 0x3e, 0x20, 0x25, 0xa4, 0x5a
};

UINT8  Aes128CbcIvec[] = {
  0x56, 0x2e, 0x17, 0x99, 0x6d, 0x09, 0x3d, 0x28, 0xdd, 0xb3, 0xba, 0x69, 0x5a, 0x2e, 0x6f, 0x58
};

UINT8  Aes128CbcOutData1[128] = {0};
UINT8  Aes128CbcData1[128]    = {0};

/**
  This function processes to Encryption password.

  @param[in] PASSWORD_CONFIG_DATA           A pointer to the data off password.


  @retval   VOID

**/
VOID PasswordEncryption(PASSWORD_CONFIG_DATA    *PasswordConfigData)
{

   #if ENCRYP_ENABLE
   UINT8                 Index = 0;
   UINT8                 IndexD = 0;
   AES_KEY               *AesKey;
   UINT8                 AdminFlag = 0;
   UINT8                 UserFlag = 0;

   AesKey = AllocateZeroPool(sizeof(AES_KEY));
   AesInit((VOID *)AesKey,Aes128CbcKey,128);

   for(Index =0;Index<PASSWD_MAXLEN;Index++)
   {
    //DEBUG((DEBUG_INFO, "%x_%x\n",PasswordConfigData->AdminPassword[Index],PasswordConfigData->UserPassword[Index]));
    if(PasswordConfigData->AdminPassword[Index] != 0x00)
      AdminFlag = 1;
    if(PasswordConfigData->UserPassword[Index] != 0x00)
      UserFlag = 1;
   }
   //DEBUG ((DEBUG_ERROR, "\n---org data---encry %d %d\n",AdminFlag,UserFlag));

   if(AdminFlag == 1)
   {
           IndexD = 0;
           for(Index =0;Index<PASSWD_MAXLEN;Index++)
           {
              Aes128CbcData1[IndexD] = PasswordConfigData->AdminPassword[Index];
              IndexD = IndexD +1;
              Aes128CbcData1[IndexD] = (PasswordConfigData->AdminPassword[Index] >> 8);
              IndexD = IndexD +1;
           }

           AesCbcEncrypt(AesKey,Aes128CbcData1,sizeof(Aes128CbcData1),Aes128CbcIvec,Aes128CbcOutData1);
           CopyMem ((UINT8 *)PasswordConfigData->AdminPassword, (UINT8 *)Aes128CbcOutData1, (PASSWD_MAXLEN*2));

           #if 0
           for(Index =0;Index<PASSWD_MAXLEN;Index++)
           {
            DEBUG((DEBUG_INFO, "%x_",PasswordConfigData->AdminPassword[Index]));
           }

           DEBUG ((DEBUG_ERROR, "\n---admin encry1-4--- \n"));
           #endif
   }
   ///
   if(UserFlag == 1)
   {
           IndexD = 0;
           for(Index =0;Index<PASSWD_MAXLEN;Index++)
           {
              Aes128CbcData1[IndexD] = PasswordConfigData->UserPassword[Index];
              IndexD = IndexD +1;
              Aes128CbcData1[IndexD] = (PasswordConfigData->UserPassword[Index] >> 8);
              IndexD = IndexD +1;
           }

           AesCbcEncrypt(AesKey,Aes128CbcData1,sizeof(Aes128CbcData1),Aes128CbcIvec,Aes128CbcOutData1);
           CopyMem ((UINT8 *)PasswordConfigData->UserPassword, (UINT8 *)Aes128CbcOutData1, (PASSWD_MAXLEN*2));
          
	   #if 0 
	   for(Index =0;Index<PASSWD_MAXLEN;Index++)
           {
            DEBUG((DEBUG_INFO, "%x_",PasswordConfigData->UserPassword[Index]));
           }

           DEBUG ((DEBUG_ERROR, "\n user---encry1-4 \n"));
           #endif
   }

   FreePool(AesKey);
   //DEBUG ((DEBUG_ERROR, "---encry end \n"));
   #endif
}

/**
  This function processes to decode password.

  @param[in] PASSWORD_CONFIG_DATA           A pointer to the data off password.


  @retval   VOID

**/

VOID PasswordDecode(PASSWORD_CONFIG_DATA    *PasswordConfigData)
{

   #if ENCRYP_ENABLE
   UINT8                 Index = 0;
   UINT8                 IndexD = 0;
   AES_KEY               *AesKey;
   UINT8                 AdminFlag = 0;
   UINT8                 UserFlag = 0;

   AesKey = AllocateZeroPool(sizeof(AES_KEY));
   AesInit((VOID *)AesKey,Aes128CbcKey,128);

   for(Index =0;Index<PASSWD_MAXLEN;Index++)
   {
    //DEBUG((DEBUG_INFO, "%x_%x:",PasswordConfigData->AdminPassword[Index],PasswordConfigData->UserPassword[Index]));
    if(PasswordConfigData->AdminPassword[Index] != 0x00)
      AdminFlag = 1;
    if(PasswordConfigData->UserPassword[Index] != 0x00)
      UserFlag = 1;
   }
   //DEBUG ((DEBUG_ERROR, "\n---org data---decode %d %d\n",AdminFlag,UserFlag));

   if(AdminFlag == 1)
   {
	   IndexD = 0;
	   for(Index =0;Index<PASSWD_MAXLEN;Index++)
	   {
	      Aes128CbcData1[IndexD] = PasswordConfigData->AdminPassword[Index];
	      IndexD = IndexD +1;
	      Aes128CbcData1[IndexD] = (PasswordConfigData->AdminPassword[Index] >> 8);
	      IndexD = IndexD +1;
	   }

	   AesCbcDecrypt(AesKey,Aes128CbcData1,sizeof(Aes128CbcData1),Aes128CbcIvec,Aes128CbcOutData1);
	   CopyMem ((UINT8 *)PasswordConfigData->AdminPassword, (UINT8 *)Aes128CbcOutData1, (PASSWD_MAXLEN*2));
           #if 0
	   for(Index =0;Index<PASSWD_MAXLEN;Index++)
	   {
	    DEBUG((DEBUG_INFO, "%x_",PasswordConfigData->AdminPassword[Index]));
	   }

	   DEBUG ((DEBUG_ERROR, "\n---admin decode1-4 \n"));
           #endif
   }

   if(UserFlag == 1)
   {
           IndexD = 0;
           for(Index =0;Index<PASSWD_MAXLEN;Index++)
           {
              Aes128CbcData1[IndexD] = PasswordConfigData->UserPassword[Index];
              IndexD = IndexD +1;
              Aes128CbcData1[IndexD] = (PasswordConfigData->UserPassword[Index] >> 8);
              IndexD = IndexD +1;
           }

           AesCbcDecrypt(AesKey,Aes128CbcData1,sizeof(Aes128CbcData1),Aes128CbcIvec,Aes128CbcOutData1);
           CopyMem ((UINT8 *)PasswordConfigData->UserPassword, (UINT8 *)Aes128CbcOutData1, (PASSWD_MAXLEN*2));
           #if 0
           for(Index =0;Index<PASSWD_MAXLEN;Index++)
           {
            DEBUG((DEBUG_INFO, "%x_",PasswordConfigData->UserPassword[Index]));
           }

           DEBUG ((DEBUG_ERROR, "\n user---decode1-4 \n"));
           #endif
   }


   FreePool(AesKey);
   //DEBUG ((DEBUG_ERROR, "---decode end \n"));
   #endif
}



