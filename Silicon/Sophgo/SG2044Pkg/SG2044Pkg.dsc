[Defines]
  PLATFORM_NAME                  = SG2044Pkg
  PLATFORM_GUID                  = 12345678-1234-1234-1234-123456789012
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SG2044Pkg
  SUPPORTED_ARCHITECTURES        = RISCV64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER              = DEFAULT

[LibraryClasses]
  PasswordHashLib|Silicon/Sophgo/Library/PasswordHashLib/PasswordHashLib.inf

[Components]
  Silicon/Sophgo/SG2044Pkg/Drivers/PasswordConfigDxe/PasswordConfigDxe.inf

[BuildOptions]
  MSFT:*_*_*_CC_FLAGS = /W4
  GCC:*_*_*_CC_FLAGS = -Wall -Wextra