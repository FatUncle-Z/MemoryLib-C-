//
//  MemoryTrackAlloc.cpp
//  MemoryLib
//
//  Created by zhaojun on 15/12/1.
//  Copyright © 2015年 zhaojun. All rights reserved.
//
#include <iostream>
#include <assert.h>

#include "MemoryTrackAlloc.h"

std::mutex Mutex;
char*     TrackAllocPolicy::MemoryPool      = 0;
size_t    TrackAllocPolicy::CurrentOffset   = 0;
char      TrackAllocPolicy::Magic[4]        = {0x4F, 0x47, 0x52, 0x45};
size_t    TrackAllocPolicy::RandomOffset    = 987026;

DECL_MALLOC void* TrackAllocPolicy::allocateBytes(size_t count, const char* file, int line, const char* func)
{
    if (!count)
    {
        return 0;
    }
    
    if (!MemoryPool)
    {
        //Initialize the pool
        MemoryPool = (char*) malloc(TRACK_POOL_SIZE);
        CurrentOffset = 0;
        RandomOffset  = 987026;
        
        Magic[0] = 0x4F;
        Magic[1] = 0x47;
        Magic[2] = 0x52;
        Magic[3] = 0x45;
        
        size_t j = 0;
        for (int i = 0; i < TRACK_POOL_SIZE; ++i)
        {
            MemoryPool[i] = Magic[j];
            j = (j+1)%4;
        }
    }
    //This isn't thread safe, but oh well, it's never meant to be
    //used in shipment, and it gets called at initialization
    
    Mutex.lock();
    size_t oldOffset = CurrentOffset;
    
    //Count is first 8 bytes more, then rounded up to multiple of size_t
    count += 8;
    size_t internalCount = ((count + sizeof(size_t) - 1) / sizeof(size_t)) * sizeof(size_t);
    
    CurrentOffset += internalCount;
    
    assert( CurrentOffset < TRACK_POOL_SIZE && "Memory corruption detected!!!" );
    size_t j = oldOffset % 4;
    for (size_t i = oldOffset; i < CurrentOffset; ++i)
    {
        assert( MemoryPool[i] == Magic[j] && "Memory corruption detected!!!" );
        j = (j + 1) % 4;
    }
    
    *(uint32*)(MemoryPool + oldOffset) = count;
    *(uint32*)(MemoryPool + oldOffset + 4) = count + RandomOffset;
    Mutex.unlock();
    
    return MemoryPool + oldOffset + 8;
}

void TrackAllocPolicy::deallocateBytes(void *ptr)
{
    if (!ptr)
    {
        return;
    }
    
    Mutex.lock();
    char* _ptr = (char*)ptr - 8;
    if (_ptr < MemoryPool || ptr >= MemoryPool + TRACK_POOL_SIZE)
    {
        assert(false && "We didn't create this pointer!!!");
    }
    
    uint32 count0 = *(uint32*)(_ptr);
    uint32 count1 = *(uint32*)(_ptr + 4);
    
    assert(count0 + RandomOffset == count1 && "Memory corruption detected!!!");
    assert(count0 < TRACK_POOL_SIZE && "Memory corruption detected!!!");
    
    size_t start = (_ptr - MemoryPool);
    size_t j = start % 4;
    for (size_t i = start; i < start+count0; ++i)
    {
        MemoryPool[i] = Magic[j];
        j = (j + 1) %4;
    }
    
    size_t internalCount = ((count0 + sizeof(size_t) - 1) / sizeof(size_t)) * sizeof(size_t);
    for (size_t i = start + count0; i < start + internalCount; ++i)
    {
        assert(MemoryPool[i] == Magic[j] && "Memory corruption detected!!!");
        j = (j + 1) % 4;
    }
    
    Mutex.unlock();
}