#if !defined(MALLOC_H)

#include "shared_includes.h"

#define Align4(x) (((((x) -1)>>2)<<2)+4)
#define internal static

#include <sys/types.h>
#include <unistd.h>


typedef unsigned int u32;
typedef bool b8; 
typedef size_t memory_index;

struct block
{
    memory_index Size;
    block *Next;
    void *Ptr; // NOTE(dan): Pointer to the allocated block. 
    b32 Free;
    char Data[1]; 
}; 

#define BLOCK_SIZE 20

void *Allocate(memory_index Size);
void Deallocate(void *Ptr);
void *Reallocate(void *Ptr, memory_index Size);

#define MALLOC_H
#endif
