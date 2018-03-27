#if !defined(MALLOC_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Daniel Solovich $
   $Notice: (C) Copyright 2016 by Solovich, Inc. All Rights Reserved. $
   ======================================================================== */

#define Align4(x) (((((x) -1)>>2)<<2)+4)
#define internal static

#include <sys/types.h>
#include <unistd.h>

typedef unsigned int u32;
typedef bool b8; 

struct block
{
    u32 Size;
    block *Next;
    block *Prev;
    void *Ptr; // NOTE(dan): Pointer to the allocated block. 
    b8 Free;
    char Data[1]; 
}; 

#define BLOCK_SIZE sizeof(block)

void *Allocate(u32 Size);
void Deallocate(void *Ptr);
void *Reallocate(void *Ptr, u32 Size);

#define MALLOC_H
#endif
