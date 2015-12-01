//
//  MemoryTrackAlloc.h
//  MemoryLib
//
//  Created by zhaojun on 15/12/1.
//  Copyright © 2015年 zhaojun. All rights reserved.
//

#ifndef MemoryTrackAlloc_h
#define MemoryTrackAlloc_h

#include <memory>
#include <mutex>

#include "MemoryMacro.h"

//1GB pool
#define TRACK_POOL_SIZE   1024 * 1024 * 1024

/** An allocation policy that preallocates a fixed amount of memory and always
 returns a deterministic pool. Very useful for catching memory corruption
 errors (but not memory leaks). It uses huge amounts of RAM, so you better
 use 64-bit builds.
 */
class TrackAllocPolicy
{
    static char*         MemoryPool;
    static size_t        CurrentOffset;
    static char          Magic[4];
    static size_t        RandomOffset;
    
public:
    static DECL_MALLOC void* allocateBytes(size_t count, const char* file = 0, int line = 0, const char* func = 0);
    
    static void deallocateBytes(void* ptr);
    
    static inline size_t getMaxAllocationSize()
    {
        return TRACK_POOL_SIZE;
    }
    
private:
    TrackAllocPolicy(){}
};

template<size_t Alignment = 0>
class TrackAlignedAllocPolicy
{
public:
    typedef int IsValidAlignment
    [Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];
    
    static inline DECL_MALLOC void* allocateBytes(size_t count, const char* file = 0, int line = 0, const char* func = 0)
    {
        size_t _alignment = Alignment ? Alignment : 16; //Should be OGRE_SIMD_ALIGNMENT, but compiler error
        uint8 *tmp = (uint8*)TrackAllocPolicy::allocateBytes( count + _alignment, file, line, func );
        
        //Align...
        uint8 *mem_block = (uint8*)( (uint32) *(tmp + _alignment - 1) & (uint32)(~(_alignment - 1)) );
        
        //Special case where malloc have already satisfied the alignment
        //We must add alignment to mem_block to mantain alignment AND
        //to avoid that afree causes an ACCESS VIOLATION becuase
        //(*(mem_block-1)) is beyond our visibility
        if (mem_block == tmp)
            mem_block += _alignment;
        
        //How far are from the real start of our memory
        //block?
        *(mem_block-1) = (uint8) (mem_block-tmp);
        
        return (void*)mem_block;
    }
    
    static inline void deallocateBytes(void* ptr)
    {
        uint8 *realAddress;
        
        if( !ptr )
            return;
        
        realAddress  = (uint8*)ptr;
        realAddress -= *(realAddress-1);
        
        TrackAllocPolicy::deallocateBytes( realAddress );
    }
    
    /// Get the maximum size of a single allocation
    static inline size_t getMaxAllocationSize()
    {
        return TRACK_POOL_SIZE;
    }
private:
    // No instantiation
    TrackAlignedAllocPolicy()
    { }
};


#endif /* MemoryTrackAlloc_h */
