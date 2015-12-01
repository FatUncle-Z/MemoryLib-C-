//
//  MemoryNedPool.cpp
//  MemoryLib
//
//  Created by zhaojun on 15/11/30.
//  Copyright © 2015年 zhaojun. All rights reserved.
//

#include "MemoryNedPool.h"
#include "MemoryConfig.h"

#if USE_MEMORY_TRACKER
#   include "MemoryTracker.h"
#endif

#include "nedmalloc/nedmalloc.c"
namespace _NedPoolIntern
{
    const size_t s_poolCount = 14;  // Needs to be greater than 4
    void* s_poolFootprint = reinterpret_cast<void*>(0xBB1AA45A);
    nedalloc::nedpool* s_pools[s_poolCount + 1] = {0};
    nedalloc::nedpool* s_poolsAligned[s_poolCount + 1] = {0};
    
    inline size_t poolIDFromSize(size_t a_reqSize)
    {
        // Requests size 16 or smaller are allocated at a 4 byte granularity.
        // Requests size 17 or larger are allocated at a 16 byte granularity.
        // With a s_poolCount of 14, requests size 177 or larger go in the default pool.
        
        // spreadsheet style =IF(B35<=16; FLOOR((B35-1)/4;1); MIN(FLOOR((B35-1)/16; 1) + 3; 14))
        size_t poolID = 0;
        if (a_reqSize > 0)
        {
            if (a_reqSize <= 16)
            {
                poolID = (a_reqSize - 1) >> 2;
            }
            else
            {
                poolID = std::min<size_t>(((a_reqSize - 1) >> 4) + 3, s_poolCount);
            }
        }
        return poolID;
    }
    
    DECL_MALLOC inline void* internalAlloc(size_t a_reqSize)
    {
        size_t poolID = poolIDFromSize(a_reqSize);
        nedalloc::nedpool* pool(0);// A pool pointer of 0 means the default pool.
        
        if (poolID < s_poolCount)
        {
            if (s_pools[poolID] == 0)
            {
                // Init pool if first use
                s_pools[poolID] = nedalloc::nedcreatepool(0, 8);
                nedalloc::nedpsetvalue(s_pools[poolID], s_poolFootprint); // All pools are stamped with a footprint
            }
            pool = s_pools[poolID];
        }
        
        return nedalloc::nedpmalloc(pool, a_reqSize);
    }
    
    DECL_MALLOC inline void* internalAllocAligned(size_t a_align, size_t a_reqSize)
    {
        size_t poolID = poolIDFromSize(a_reqSize);
        nedalloc::nedpool* pool(0);// A pool pointer of 0 means the default pool.
        
        if (poolID < s_poolCount) {
            if (s_poolsAligned[poolID] == 0)
            {
                // Init pool if first use
                s_poolsAligned[poolID] = nedalloc::nedcreatepool(0, 8);
                nedalloc::nedpsetvalue(s_poolsAligned[poolID], s_poolFootprint); // All pools are stamped with a footprint
            }
            pool = s_poolsAligned[poolID];
        }
        return nedalloc::nedpmemalign(pool, a_align, a_reqSize);
    }
    
    inline void internalFree(void* a_mem)
    {
        if (a_mem)
        {
            nedalloc::nedpool* pool(0);
            
            // nedalloc lets us get the pool pointer from the memory pointer
            void* footprint = nedalloc::nedgetvalue(&pool, a_mem);
            if (footprint == s_poolFootprint)
            {
                // If we allocated the pool, deallocate from this pool...
                nedalloc::nedpfree(pool, a_mem);
            }
            else
            {
                // ...otherwise let nedalloc handle it.
                nedalloc::nedfree(a_mem);
            }
        }
    }
}



DECL_MALLOC void* NedPoolImpl::allocBytes(size_t count, const char *file, int line, const char *func)
{
    void* ptr = _NedPoolIntern::internalAlloc(count);
#if USE_MEMORY_TRACKER
    MemoryTracker::get()._recordAlloc(ptr, count, 0, file, line, func);
#endif
    return ptr;
}

void NedPoolImpl::deallocBytes(void *ptr)
{
    if (!ptr)
    {
        return;
    }
    else
    {
#if USE_MEMORY_TRACKER
        MemoryTracker::get()._recordDealloc(ptr);
#endif
        _NedPoolIntern::internalFree(ptr);
    }
}

DECL_MALLOC void* NedPoolImpl::allocBytesAligned(size_t align, size_t count, const char *file, int line, const char *func)
{
    void* ptr = _NedPoolIntern::internalAllocAligned(align, count);
    return ptr;
}

void NedPoolImpl::deallocBytesAligned(size_t align, void *ptr)
{
    if (!ptr)
    {
        return;
    }
    else
    {
#if USE_MEMORY_TRACKER
        MemoryTracker::get()._recordDealloc(ptr);
#endif
        _NedPoolIntern::internalFree(ptr);
    }
}