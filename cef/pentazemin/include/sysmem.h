#ifndef SYSMEM_H
#define SYSMEM_H

#include <pspsdk.h>

typedef struct PartitionData {
    u32 unk[5];
    u32 size;
} PartitionData;

typedef struct SysMemPartition {
    struct SysMemPartition *next;
    u32    address;
    u32 size;
    u32 attributes;
    PartitionData *data;
} SysMemPartition;

#endif