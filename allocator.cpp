/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator:Daniel Solovich $
   $Notice: (C) Copyright 2016 by Solovich, Inc. All Rights Reserved. $
   ======================================================================== */
#include "allocator.h"


void *Base = NULL;

internal block * 
FindBlock(block **Last, u32 Size)
{
    block *Block = (block*)Base;
    while(Block &&
          !(Block->Free && Block->Size >= Size))
    {
        *Last = Block;
        Block = Block->Next; 
    }
    return(Block); 
}


internal block *
ExtendHeap(block *Last, u32 Size)
{
    block *Block;
    Block = (block*)sbrk(0);
    if(sbrk((int)BLOCK_SIZE + Size) == (void*)-1)
    {
        return(NULL); 
    }
    Block->Size = Size;
    Block->Next = NULL;
    if(Last)
    {
        Last->Next = Block; 
    }
    Block->Free = false;
    return(Block); 
}


internal void
SplitBlock(block *Block, u32 Size)
{
    block *New;
    New = (block*)(Block->Data + Size);
    New->Size = Block->Size - Size - BLOCK_SIZE;
    New->Next = Block->Next;
    New->Free = true;
    Block->Size = Size;
    Block->Next = New; 
}

void * 
Allocate(u32 Size)
{
    block *Block, *Last;
    u32 S;
    S = Align4(Size);
    if(Base)
    {
        Last = (block*)Base;
        Block = FindBlock(&Last, S);
        if(Block)
        {
            if((Block->Size - S) >= (BLOCK_SIZE + 4))
            {
                SplitBlock(Block, S);
            }
            Block->Free = false; 
        }
        else
        {
            Block = ExtendHeap(Last, S);
            if(!Block)
            {
                return(NULL); 
            }
        }
    }
    else
    {
        Block = ExtendHeap(NULL, S);
        if(!Block)
        {
            return(NULL); 
        }
        Base = (void*)Block;
    }
    return(Block->Data); 
}


internal block *
Fusion(block *Block)
{
    if(Block->Next &&
       Block->Next->Free)
    {
        Block->Size += BLOCK_SIZE + Block->Next->Size;
        Block->Next = Block->Next->Next;
        if(Block->Next)
        {
            Block->Next->Prev = Block;
        }
    }
    return(Block); 
}


internal void *
GetBlock(void *Ptr)
{
    char *Tmp;
    Tmp = (char*)Ptr;
    return(Ptr = Tmp -= BLOCK_SIZE);
}

internal b8
ValidAddres(void *Ptr)
{
    if(Base)
    {
        if(Ptr > Base &&
           Ptr < sbrk(0))
        {            
            return(Ptr == ((block*)GetBlock(Ptr))->Ptr);
        }
    }
    return(false); 
}


void
Deallocate(void *Ptr)
{
    block *Block;
    if(ValidAddres(Ptr))
    {
        Block = (block*)GetBlock(Ptr);
        Block->Free = true;
        if(Block->Prev &&
           Block->Prev->Free)
        {
            Block = Fusion(Block->Prev);
        }
        if(Block->Next)
        {
            Fusion(Block);
        }
        else
        {
            if(Block->Prev)
            {
                Block->Prev->Next = NULL;                
            }
            else
            {
                Base = NULL; 
            }
            brk(Block);
        }
    }
}

internal void
CopyBlock(block *Source, block *Dist)
{
    u32 *SourceData, *DistData;
    SourceData = (u32*)Source->Next;
    DistData = (u32*)Dist->Next;
    for(u32 i = 0;
        i * 4 < Source->Size && i * 4 < Dist->Size;
        ++i)
    {
        DistData[i] = SourceData[i]; 
    }
}

void *
Reallocate(void *Ptr, u32 Size)
{
    u32 S;
    block *Block, *New;
    void *NewPtr;
    if(!Ptr)
    {
        return(Allocate(Size));
    }
    if(ValidAddres(Ptr))
    {
        S = Align4(Size);
        Block = (block*)GetBlock(Ptr);
        if(Block->Size >= S)
        {
            if(Block->Size - S >= (BLOCK_SIZE + 4))
            {
                SplitBlock(Block, S);
            }
        }
        else
        {
            if(Block->Next &&
               Block->Next->Free &&
               (Block->Size+ BLOCK_SIZE + Block->Next->Size) >= S)
            {
                Fusion(Block);
                if(Block->Size - S >= (BLOCK_SIZE + 4))
                {
                    SplitBlock(Block, S);
                }
            }
            else
            {
                NewPtr = Allocate(S);
                if(!NewPtr)
                {
                    return(NULL); 
                }
                New = (block*)GetBlock(NewPtr);
                CopyBlock(Block, New);
                Deallocate(Ptr);
                return(NewPtr); 
            }
        }
        return(Ptr); 
    }
    return(NULL); 
}

