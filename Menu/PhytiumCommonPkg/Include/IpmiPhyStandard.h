/** @file
IPMI Information Header File.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  IPMI_PHYSTANDARD_H_
#define  IPMI_PHYSTANDARD_H_

#pragma pack(1)

typedef struct {
  UINT8  CompletionCode;
  UINT8  DeviceId;
  UINT8  DeviceRevision      :4;
  UINT8  Reserved            :3;
  UINT8  ProvideSDRsDevice   :1;
  UINT8  MajorFmRevision   :7;
  UINT8  DeviceAvailable   :1;
  UINT8  MinorFmRevision;
  UINT8  IpmiVersionMostSigBits  :4;
  UINT8  IpmiVersionLeastSigBits :4;
  UINT8  SensorDevice        :1;
  UINT8  SDRRepositoryDevice :1;
  UINT8  SELDevice           :1;
  UINT8  FRUInventoryDevice  :1;
  UINT8  IPMBEventReceiver   :1;
  UINT8  IPMBEventGenerator  :1;
  UINT8  Bridge              :1;
  UINT8  ChassisDevice       :1;
  UINT8  ManufacturerId[3];
  UINT8  ProductId[2];
  UINT8  AuxInfo[4];
} IPMI_BMC_INFO;

typedef struct {
  UINT8  CpuIndex;
  UINT8  CpuPresent;
  UINT8  Reserved[2];
  UINT8  CpuTypeString[32];
  UINT8  Reserved2[30];
  UINT16 CpuTypeCode;
  UINT8  CoreFrequency[2];
  UINT8  CoreCount;
} IPMI_SET_CPU_INFO;

typedef struct {
  UINT8  DIMMSlotIndex;
  UINT8  DIMMChanelNum;
  UINT8  CPUNum;
  UINT8  DIMMPresent;
  UINT8  DIMMType;
  UINT8  DIMMSpeed;
  UINT8  DIMMSize;
  UINT32 ModuleManufactureID;
  UINT32 DRAMManufactureID;
  UINT8  Reserved[8];
  UINT8  DIMMPartNumber[24];
  UINT64 SerialNum;
} IPMI_SET_SPD_INFO;

 typedef struct {
  UINT8  BIOSVersionString[32];
  UINT32 BIOSVendorID;
  UINT8  BIOSMajorVersion[2];
  UINT8  BIOSMinorVersion[2];
  UINT8  BIOSBuildDate[32];
  UINT16 PBFMajorVersion;
  UINT16 PBFMinorVersion;
  UINT8  PBFBuildDate[32];
} IPMI_SET_BIOS_INFO;

typedef struct {
  UINT8  PCIeSlotIndex;
  UINT8  Segment;
  UINT8  BusNumber;
  UINT8  DeviceNumber;
  UINT8  FunctionNumber;
  UINT8  PCIeDeviePresent;
  UINT16 VendorID;
  UINT16 DeviceID;
  UINT16 SubVendorID;
  UINT16 SubDeviceID;
  UINT8  ProgrammingIF;
  UINT8  SubClass;
  UINT8  BaseClass;
  UINT8  MACAddress[6];
} IPMI_SET_PCIE_DEVICE_INFO;

typedef struct {
  UINT8  PCIeSlotIndex;
  UINT8  Segment;
  UINT8  BusNumber;
  UINT8  DeviceNumber;
  UINT8  FunctionNumber;
  UINT8  VendorID[2];
  UINT8  DeviceID[2];
  UINT8  SubVendorID[2];
  UINT8  SubDevieID[2];
  UINT8  MACAddress[6];
} IPMI_SET_PCIE_NET_DEVICE_INFO;

typedef struct {
  UINT8  HddNumber;
  UINT8  HddType;
  UINT8  HddSN[20];
  UINT8  HddFWVersion[8];
  UINT8  HddModelName[40];
  UINT16 HddSize;
} IPMI_SET_HDD_DEVICE_INFO;

typedef struct {
  UINT8  CompletionCode;
  UINT8  Result;
  UINT8  Param;
} IPMI_GET_SELF_TEST_RESULT;

typedef struct {
  UINT8    SsionId;
  UINT8    Data[250];
} IPMI_SEND_CPER_INFO;

typedef struct {
  UINT8    UserId;
  UINT8    Operation;
  UINT8    PasswordData[20];
} IPMI_SET_USER_PASSWORD_CMD;

#pragma pack()

#endif
