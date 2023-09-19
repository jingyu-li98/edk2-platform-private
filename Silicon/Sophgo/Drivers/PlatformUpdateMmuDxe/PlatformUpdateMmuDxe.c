/** @file

  This driver modifies the page table attribute based on the memory attribute of the C920.
  C920 has five customizable page properties that control whether the page is Strong order,
  Cacheable, Bufferable, Shareable,Trustable.

  Copyright (c) 2011-2020, ARM Limited. All rights reserved.
  Copyright (c) 2016, Linaro Limited. All rights reserved.
  Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2023, Ventana Micro Systems Inc. All Rights Reserved.<BR>
  Copyright (c) 2023, Academy of Intelligent Innovation, Shandong Universiy, China.P.R. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseRiscVMmuLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Register/RiscV64/RiscVEncoding.h>

#define RISCV_PG_V           BIT0
#define RISCV_PG_R           BIT1
#define RISCV_PG_W           BIT2
#define RISCV_PG_X           BIT3
#define RISCV_PG_G           BIT5
#define RISCV_PG_A           BIT6
#define RISCV_PG_D           BIT7
/* T-header C920 custom page properties */
#define THEAD_C920_PTE_SEC   BIT59 // Trustable
#define THEAD_C920_PTE_SH    BIT60 // Shareable
#define THEAD_C920_PTE_B     BIT61 // Bufferable
#define THEAD_C920_PTE_C     BIT62 // Cacheable
#define THEAD_C920_PTE_SO    BIT63 // Strong Order
#define PTE_ATTRIBUTES_MASK  0xE

#define PTE_PPN_MASK          0x3FFFFFFFFFFC00ULL
#define PTE_PPN_SHIFT         10
#define RISCV_MMU_PAGE_SHIFT  12

STATIC UINTN  mMaxRootTableLevel;
STATIC UINTN  mBitPerLevel;
STATIC UINTN  mTableEntryCount;

/**
  Determine if the MMU enabled or not.

  @retval TRUE  The MMU already enabled.
  @retval FALSE The MMU not enabled.

**/
STATIC
VOID
RiscVMmuDisabled (
  VOID
  )
{
  RiscVSetSupervisorAddressTranslationRegister (SATP_MODE_OFF << SATP64_MODE_SHIFT);
}

/**
  Determine if the MMU enabled or not.

  @retval TRUE  The MMU already enabled.
  @retval FALSE The MMU not enabled.

**/
STATIC
BOOLEAN
RiscVMmuEnabled (
  VOID
  )
{
  return ((RiscVGetSupervisorAddressTranslationRegister () &
           SATP64_MODE) != (SATP_MODE_OFF << SATP64_MODE_SHIFT));
}

/**
  Retrieve the root translate table.

  @return The root translate table.

**/
STATIC
UINTN
RiscVGetRootTranslateTable (
  VOID
  )
{
  return (RiscVGetSupervisorAddressTranslationRegister () & SATP64_PPN) <<
         RISCV_MMU_PAGE_SHIFT;
}

/**
  Determine if an entry is valid pte.

  @param    Entry   The entry value.

  @retval   TRUE    The entry is a valid pte.
  @retval   FALSE   The entry is not a valid pte.

**/
STATIC
BOOLEAN
IsValidPte (
  IN  UINTN  Entry
  )
{
  if (((Entry & RISCV_PG_V) == 0) ||
      (((Entry & (RISCV_PG_R | RISCV_PG_W)) == RISCV_PG_W)))
  {
    return FALSE;
  }

  return TRUE;
}

/**
  Set an entry to be a valid pte.

  @param  Entry   The entry value.

  @return         The entry value.

**/
STATIC
UINTN
SetValidPte (
  IN  UINTN  Entry
  )
{
  /* Set Valid and Global mapping bits */
  return Entry | RISCV_PG_G | RISCV_PG_V;
}

/**
  Determine if an entry is a block pte.

  @param    Entry   The entry value.

  @retval   TRUE    The entry is a block pte.
  @retval   FALSE   The entry is not a block pte.

**/
STATIC
BOOLEAN
IsBlockEntry (
  IN  UINTN  Entry
  )
{
  return IsValidPte (Entry) &&
         (Entry & (RISCV_PG_X | RISCV_PG_R));
}

/**
  Determine if an entry is a table pte.

  @param    Entry   The entry value.

  @retval   TRUE    The entry is a table pte.
  @retval   FALSE   The entry is not a table pte.

**/
STATIC
BOOLEAN
IsTableEntry (
  IN  UINTN  Entry
  )
{
  return IsValidPte (Entry) &&
         !IsBlockEntry (Entry);
}

/**
  Set an entry to be a table pte.

  @param  Entry   The entry value.

  @return         The entry value.

**/
STATIC
UINTN
SetTableEntry (
  IN  UINTN  Entry
  )
{
  Entry  = SetValidPte (Entry);
  Entry &= ~(RISCV_PG_X | RISCV_PG_W | RISCV_PG_R);

  return Entry;
}

/**
  Replace an existing entry with new value.

  @param  Entry               The entry pointer.
  @param  Value               The new entry value.
  @param  RegionStart         The start of region that new value affects.
  @param  IsLiveBlockMapping  TRUE if this is live update, FALSE otherwise.

**/
STATIC
VOID
ReplaceTableEntry (
  IN  UINTN    *Entry,
  IN  UINTN    Value,
  IN  UINTN    RegionStart,
  IN  BOOLEAN  IsLiveBlockMapping
  )
{
  *Entry = Value;

  if (IsLiveBlockMapping && RiscVMmuEnabled ()) {
    RiscVLocalTlbFlush (RegionStart);
  }
}

/**
  Get an ppn value from an entry.

  @param  Entry   The entry value.

  @return         The ppn value.

**/
STATIC
UINTN
GetPpnfromPte (
  IN UINTN  Entry
  )
{
  return ((Entry & PTE_PPN_MASK) >> PTE_PPN_SHIFT);
}

/**
  Set an ppn value to a entry.

  @param  Entry   The entry value.
  @param  Address The address.

  @return The new entry value.

**/
STATIC
UINTN
SetPpnToPte (
  UINTN  Entry,
  UINTN  Address
  )
{
  UINTN  Ppn;

  Ppn = ((Address >> RISCV_MMU_PAGE_SHIFT) << PTE_PPN_SHIFT);
  ASSERT (~(Ppn & ~PTE_PPN_MASK));
  Entry &= ~PTE_PPN_MASK;
  return Entry | Ppn;
}

/**
  Free resources of translation table recursively.

  @param  TranslationTable  The pointer of table.
  @param  Level             The current level.

**/
STATIC
VOID
FreePageTablesRecursive (
  IN  UINTN  *TranslationTable,
  IN  UINTN  Level
  )
{
  UINTN  Index;

  if (Level < mMaxRootTableLevel - 1) {
    for (Index = 0; Index < mTableEntryCount; Index++) {
      if (IsTableEntry (TranslationTable[Index])) {
        FreePageTablesRecursive (
          (UINTN *)(GetPpnfromPte ((TranslationTable[Index])) <<
                    RISCV_MMU_PAGE_SHIFT),
          Level + 1
          );
      }
    }
  }

  FreePages (TranslationTable, 1);
}

/**
  Update region mapping recursively.

  @param  RegionStart           The start address of the region.
  @param  RegionEnd             The end address of the region.
  @param  AttributeSetMask      The attribute mask to be set.
  @param  AttributeClearMask    The attribute mask to be clear.
  @param  PageTable             The pointer of current page table.
  @param  Level                 The current level.
  @param  TableIsLive           TRUE if this is live update, FALSE otherwise.

  @retval EFI_OUT_OF_RESOURCES  Not enough resource.
  @retval EFI_SUCCESS           The operation succesfully.

**/
STATIC
EFI_STATUS
UpdateRegionMappingRecursive (
  IN  UINTN    RegionStart,
  IN  UINTN    RegionEnd,
  IN  UINTN    AttributeSetMask,
  IN  UINTN    AttributeClearMask,
  IN  UINTN    *PageTable,
  IN  UINTN    Level,
  IN  BOOLEAN  TableIsLive
  )
{
  EFI_STATUS  Status;
  UINTN       BlockShift;
  UINTN       BlockMask;
  UINTN       BlockEnd;
  UINTN       *Entry;
  UINTN       EntryValue;
  UINTN       *TranslationTable;
  BOOLEAN     NextTableIsLive;

  ASSERT (Level < mMaxRootTableLevel);
  ASSERT (((RegionStart | RegionEnd) & EFI_PAGE_MASK) == 0);

  BlockShift = (mMaxRootTableLevel - Level - 1) * mBitPerLevel + RISCV_MMU_PAGE_SHIFT;
  BlockMask  = MAX_ADDRESS >> (64 - BlockShift);

  DEBUG (
    (
     DEBUG_VERBOSE,
     "%a(%d): %llx - %llx set %lx clr %lx\n",
     __func__,
     Level,
     RegionStart,
     RegionEnd,
     AttributeSetMask,
     AttributeClearMask
    )
    );

  for ( ; RegionStart < RegionEnd; RegionStart = BlockEnd) {
    BlockEnd = MIN (RegionEnd, (RegionStart | BlockMask) + 1);
    Entry    = &PageTable[(RegionStart >> BlockShift) & (mTableEntryCount - 1)];

    //
    // If RegionStart or BlockEnd is not aligned to the block size at this
    // level, we will have to create a table mapping in order to map less
    // than a block, and recurse to create the block or page entries at
    // the next level. No block mappings are allowed at all at level 0,
    // so in that case, we have to recurse unconditionally.
    //
    if ((Level == 0) ||
        (((RegionStart | BlockEnd) & BlockMask) != 0) || IsTableEntry (*Entry))
    {
      ASSERT (Level < mMaxRootTableLevel - 1);
      if (!IsTableEntry (*Entry)) {
        //
        // No table entry exists yet, so we need to allocate a page table
        // for the next level.
        //
        TranslationTable = AllocatePages (1);
        if (TranslationTable == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }

        ZeroMem (TranslationTable, EFI_PAGE_SIZE);

        if (IsBlockEntry (*Entry)) {
          //
          // We are splitting an existing block entry, so we have to populate
          // the new table with the attributes of the block entry it replaces.
          //
          Status = UpdateRegionMappingRecursive (
                     RegionStart & ~BlockMask,
                     (RegionStart | BlockMask) + 1,
                     *Entry & PTE_ATTRIBUTES_MASK,
                     PTE_ATTRIBUTES_MASK,
                     TranslationTable,
                     Level + 1,
                     FALSE
                     );
          if (EFI_ERROR (Status)) {
            //
            // The range we passed to UpdateRegionMappingRecursive () is block
            // aligned, so it is guaranteed that no further pages were allocated
            // by it, and so we only have to free the page we allocated here.
            //
            FreePages (TranslationTable, 1);
            return Status;
          }
        }

        NextTableIsLive = FALSE;
      } else {
        TranslationTable = (UINTN *)(GetPpnfromPte (*Entry) << RISCV_MMU_PAGE_SHIFT);
        NextTableIsLive  = TableIsLive;
      }

      //
      // Recurse to the next level
      //
      Status = UpdateRegionMappingRecursive (
                 RegionStart,
                 BlockEnd,
                 AttributeSetMask,
                 AttributeClearMask,
                 TranslationTable,
                 Level + 1,
                 NextTableIsLive
                 );
      if (EFI_ERROR (Status)) {
        if (!IsTableEntry (*Entry)) {
          //
          // We are creating a new table entry, so on failure, we can free all
          // allocations we made recursively, given that the whole subhierarchy
          // has not been wired into the live page tables yet. (This is not
          // possible for existing table entries, since we cannot revert the
          // modifications we made to the subhierarchy it represents.)
          //
          FreePageTablesRecursive (TranslationTable, Level + 1);
        }

        return Status;
      }

      if (!IsTableEntry (*Entry)) {
        EntryValue = SetPpnToPte (0, (UINTN)TranslationTable);
        EntryValue = SetTableEntry (EntryValue);
        ReplaceTableEntry (
          Entry,
          EntryValue,
          RegionStart,
          TableIsLive
          );
      }
    } else {
      EntryValue = (*Entry & ~AttributeClearMask) | AttributeSetMask;
      //
      // We don't have page fault exception handler when a virtual page is accessed and
      // the A bit is clear, or is written and the D bit is clear.
      // So just set A for read and D for write permission.
      //
      if ((AttributeSetMask & RISCV_PG_R) != 0) {
        EntryValue |= RISCV_PG_A;
      }

      if ((AttributeSetMask & RISCV_PG_W) != 0) {
        EntryValue |= RISCV_PG_D;
      }

      EntryValue = SetPpnToPte (EntryValue, RegionStart);
      EntryValue = SetValidPte (EntryValue);
      ReplaceTableEntry (Entry, EntryValue, RegionStart, TableIsLive);
    }
  }

  return EFI_SUCCESS;
}

/**
  Update region mapping at root table.

  @param  RegionStart           The start address of the region.
  @param  RegionLength          The length of the region.
  @param  AttributeSetMask      The attribute mask to be set.
  @param  AttributeClearMask    The attribute mask to be clear.
  @param  RootTable             The pointer of root table.
  @param  TableIsLive           TRUE if this is live update, FALSE otherwise.

  @retval EFI_INVALID_PARAMETER The RegionStart or RegionLength was not valid.
  @retval EFI_OUT_OF_RESOURCES  Not enough resource.
  @retval EFI_SUCCESS           The operation succesfully.

**/
STATIC
EFI_STATUS
UpdateRegionMapping (
  IN  UINTN    RegionStart,
  IN  UINTN    RegionLength,
  IN  UINTN    AttributeSetMask,
  IN  UINTN    AttributeClearMask,
  IN  UINTN    *RootTable,
  IN  BOOLEAN  TableIsLive
  )
{
  if (((RegionStart | RegionLength) & EFI_PAGE_MASK) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  return UpdateRegionMappingRecursive (
           RegionStart,
           RegionStart + RegionLength,
           AttributeSetMask,
           AttributeClearMask,
           RootTable,
           0,
           TableIsLive
           );
}

/**
  Convert GCD attribute to RISC-V page attribute.

  @param  GcdAttributes The GCD attribute.

  @return               The RISC-V page attribute.

**/
STATIC
UINTN
GcdAttributeToPageAttribute (
  IN UINTN  GcdAttributes
  )
{
  UINTN  RiscVAttributes;

  // switch (GcdAttributes & EFI_MEMORY_CACHETYPE_MASK) {
  //   case EFI_MEMORY_UC:
  //     RiscVAttributes = RISCV_PG_R | RISCV_PG_W |
  //                       THEAD_C920_PTE_SO | THEAD_C920_PTE_SH;
  //     break;
  //   case EFI_MEMORY_WC:
  //     RiscVAttributes = RISCV_PG_R | RISCV_PG_W | RISCV_PG_X |
  //                       THEAD_C920_PTE_B | THEAD_C920_PTE_SH;
  //     break;
  //   case EFI_MEMORY_WT:
  //     RiscVAttributes = RISCV_PG_R | RISCV_PG_W | RISCV_PG_X |
  //                       THEAD_C920_PTE_B | THEAD_C920_PTE_C | THEAD_C920_PTE_SH;
  //     break;
  //   case EFI_MEMORY_WB:
  //     RiscVAttributes = RISCV_PG_R | RISCV_PG_W | RISCV_PG_X |
  //                       THEAD_C920_PTE_B | THEAD_C920_PTE_C | THEAD_C920_PTE_SH;
  //     break;
  //   default:
  //     RiscVAttributes = 0; // not support EFI_MEMORY_UCE
  //     break;
  // }
  // the attributes is only for un-cacheable device IO
  if ( GcdAttributes == EFI_MEMORY_UC ) {
     RiscVAttributes = RISCV_PG_R | RISCV_PG_W |
                       THEAD_C920_PTE_SO | THEAD_C920_PTE_SH;
  // } else if ( GcdAttributes == EFI_MEMORY_WC || GcdAttributes == EFI_MEMORY_XP ) {
  } else if ( GcdAttributes == EFI_MEMORY_WC ) {
    // the attributes is only for un-cacheable memory.
    RiscVAttributes = RISCV_PG_R | RISCV_PG_W |
                      THEAD_C920_PTE_B | THEAD_C920_PTE_SH;
    // RiscVAttributes = RISCV_PG_R | RISCV_PG_W |
    //                   THEAD_C920_PTE_SO | THEAD_C920_PTE_SH;
  } else {
    // the attributes is only for cacheable memory.
    RiscVAttributes = RISCV_PG_R | RISCV_PG_W | RISCV_PG_X |
                      THEAD_C920_PTE_B | THEAD_C920_PTE_C | THEAD_C920_PTE_SH;
  }

  // Determine protection attributes
  if ((GcdAttributes & EFI_MEMORY_RO) != 0) {
    RiscVAttributes &= ~(RISCV_PG_W);
  }

  // Process eXecute Never attribute
  if ((GcdAttributes & EFI_MEMORY_XP) != 0) {
    RiscVAttributes &= ~RISCV_PG_X;
    // RiscVAttributes |= THEAD_C920_PTE_B | THEAD_C920_PTE_SH;
  }

  return RiscVAttributes;
}

VOID
SyncIs (
  VOID
  )
{
  asm volatile (".long 0x01b0000b"); // sync.i
  // asm volatile (".long 0x0190000b"); // sync.s
}

/**
  The API to set a GCD attribute on an memory region.

  @param  BaseAddress             The base address of the region.
  @param  Length                  The length of the region.
  @param  Attributes              The GCD attributes.

  @retval EFI_INVALID_PARAMETER   The BaseAddress or Length was not valid.
  @retval EFI_OUT_OF_RESOURCES    Not enough resource.
  @retval EFI_SUCCESS             The operation succesfully.

**/
EFI_STATUS
EFIAPI
RiscVSetMemoryAttributes (
  IN EFI_PHYSICAL_ADDRESS  BaseAddress,
  IN UINTN                 Length,
  IN UINTN                 Attributes
  )
{
  UINTN        PageAttributesSet;
  EFI_STATUS   Status;
  DEBUG ((DEBUG_VERBOSE, "%a[%d] Initialied Attribute Value\t=\t0x%lx\n", __func__, __LINE__, Attributes));
  PageAttributesSet = GcdAttributeToPageAttribute (Attributes);

  if (!RiscVMmuEnabled ()) {
    return EFI_SUCCESS;
  }

  DEBUG (
    (
     DEBUG_WARN,
     "%a: Set %llX page (Length = %lX) attribute 0x%X\n",
     __func__,
     BaseAddress,
     Length,
     PageAttributesSet
    )
    );

  Status = UpdateRegionMapping (
           BaseAddress,
           Length,
           PageAttributesSet,
           PTE_ATTRIBUTES_MASK,
           (UINTN *)RiscVGetRootTranslateTable (),
           TRUE
           );
  ASSERT_EFI_ERROR (Status);

  RiscVLocalTlbFlushAll ();

  SyncIs ();

  return Status;
}

/**
  The API to configure and enable RISC-V MMU with the highest mode supported.

  @retval EFI_OUT_OF_RESOURCES    Not enough resource.
  @retval EFI_SUCCESS             The operation succesfully.

**/
EFI_STATUS
EFIAPI
PlatformUpdateMmu (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  *MemoryMap;
  UINTN                            NumberOfDescriptors;
  UINTN                            Index;
  EFI_STATUS                       Status;

  Status = EFI_SUCCESS;

  if(RiscVMmuEnabled () && PcdGetBool (PcdForceNoMMU)){

    FreePageTablesRecursive ((UINTN *)RiscVGetRootTranslateTable (), 0);

    RiscVMmuDisabled ();
    DEBUG ((
       DEBUG_INFO,
       "%a: SG2042 MMU disabled\n",
       __func__
      ));

    return Status;
  }

  if (RiscVMmuEnabled ()) {
    mMaxRootTableLevel = 3;
    mBitPerLevel       = 9;
    mTableEntryCount   = 512;

    NumberOfDescriptors = 0;
    MemoryMap           = NULL;
    Status              = gDS->GetMemorySpaceMap (
                                 &NumberOfDescriptors,
                                 &MemoryMap
                                 );
    ASSERT_EFI_ERROR (Status);

    for (Index = 0; Index < NumberOfDescriptors; Index++) {
      if (MemoryMap[Index].GcdMemoryType == EfiGcdMemoryTypeMemoryMappedIo) {
        // Update Read/Write/Strong Order attribute for memory mapped IO
        UpdateRegionMapping (
          MemoryMap[Index].BaseAddress,
          MemoryMap[Index].Length,
          RISCV_PG_R | RISCV_PG_W | THEAD_C920_PTE_SO,
          PTE_ATTRIBUTES_MASK,
          (UINTN *)RiscVGetRootTranslateTable (),
          TRUE
          );
      } else if (MemoryMap[Index].GcdMemoryType == EfiGcdMemoryTypeSystemMemory) {
        // Update Read/Write/Execute/Sharable/Cacheable attribute for system memory
        UpdateRegionMapping (
          MemoryMap[Index].BaseAddress,
          MemoryMap[Index].Length,
          RISCV_PG_R | RISCV_PG_W | RISCV_PG_X | THEAD_C920_PTE_B | THEAD_C920_PTE_C | THEAD_C920_PTE_SH,
          PTE_ATTRIBUTES_MASK,
          (UINTN *)RiscVGetRootTranslateTable (),
          TRUE
          );
      }
    }

    DEBUG ((
      DEBUG_INFO,
      "%a: SG2042 Update memory attribute\n",
      __func__
      ));

    FreePool ((VOID *)MemoryMap);
  }

  return Status;
}