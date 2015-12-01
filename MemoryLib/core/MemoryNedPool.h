//
//  MemoryNedPool.h
//  MemoryLib
//
//  Created by zhaojun on 15/11/30.
//  Copyright © 2015年 zhaojun. All rights reserved.
//

#ifndef MemoryNedPool_h
#define MemoryNedPool_h

#include <istream>
#include <limits>

#include "MemoryMacro.h"

class NedPoolImpl {
public:
    static DECL_MALLOC void* allocBytes(size_t count,
                                        const char* file, int line, const char* func);
    static void deallocBytes(void* ptr);
    static DECL_MALLOC void* allocBytesAligned(size_t align, size_t count,
                                               const char* file, int line, const char* func);
    static void deallocBytesAligned(size_t align, void* ptr);
};

/** An allocation policy for use with AllocatedObject and
 STLAllocator. This is the class that actually does the allocation
 and deallocation of physical memory, and is what you will want to
 provide a custom version of if you wish to change how memory is allocated.
 @par
 This allocation policy uses nedmalloc
 (http://nedprod.com/programs/portable/nedmalloc/index.html).
 */
class NedPoolPolicy
{
public:
    static inline DECL_MALLOC void* allocateBytes(size_t count,
                                                  const char* file = 0, int line = 0, const char* func = 0)
    {
        return NedPoolImpl::allocBytes(count, file, line, func);
    }
    static inline void deallocateBytes(void* ptr)
    {
        NedPoolImpl::deallocBytes(ptr);
    }
    /// Get the maximum size of a single allocation
    static inline size_t getMaxAllocationSize()
    {
        return std::numeric_limits<size_t>::max();
    }
    
private:
    NedPoolPolicy()
    {
    
    }
};

/** An allocation policy for use with AllocatedObject and
 STLAllocator, which aligns memory at a given boundary (which should be
 a power of 2). This is the class that actually does the allocation
 and deallocation of physical memory, and is what you will want to
 provide a custom version of if you wish to change how memory is allocated.
 @par
 This allocation policy uses nedmalloc
 (http://nedprod.com/programs/portable/nedmalloc/index.html).
 @note
 template parameter Alignment equal to zero means use default
 platform dependent alignment.
 */
template <size_t Alignment = 0>
class NedPoolAlignedPolicy
{
public:
    // compile-time check alignment is available.
    typedef int IsValidAlignment
    [Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];
    
    static inline DECL_MALLOC void* allocateBytes(size_t count,
                                                  const char* file = 0, int line = 0, const char* func = 0)
    {
        return NedPoolImpl::allocBytesAligned(Alignment, count, file, line, func);
    }
    
    static inline void deallocateBytes(void* ptr)
    {
        NedPoolImpl::deallocBytesAligned(Alignment, ptr);
    }
    
    /// Get the maximum size of a single allocation
    static inline size_t getMaxAllocationSize()
    {
        return std::numeric_limits<size_t>::max();
    }
private:
    // no instantiation allowed
    NedPoolAlignedPolicy()
    { }
};

#endif /* MemoryNedPool_h */
