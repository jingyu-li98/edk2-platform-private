#ifndef   _SE_LIB_H_
#define   _SE_LIB_H_

//#define TIME_WAKE_UP_S3
#define TIME_WAKE_UP_S3_CMD    0x81
//#define TIME_WAKE_UP_S45
#define TIME_WAKE_UP_S45_CMD   0x82

/*SE REG*/
#define SE_DEV_ADDR         (0x18)
#define SE_REG_S3_FLAG      (0x007002f00cUL)
#define SE_REG_HAND0        (0x0070021020UL) 
#define SE_REG_HAND1        (0x0070021024UL)
#define SE_REG_HAND2        (0x0070021028UL)
#define SE_REG_DATA         (0x0070021040UL)
#define SE_REG_CHECK        (0x007002f010UL)

INT32
ft_i2c_read_se(
  UINT32    dev_addr,
  UINT64   reg_addr,
  UINT32   *read_value
  );


INT32
ft_i2c_write_se(
  UINT32 dev_addr,
  UINT64 reg_addr,
  UINT64 write_value
  );

VOID
send_se_ctr(
  UINT32 cmd
  );

UINTN
read_se_state(
  VOID
  );



#endif
