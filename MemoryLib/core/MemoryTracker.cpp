//
//  MemoryTracker.cpp
//  MemoryLib
//
//  Created by zhaojun on 15/12/1.
//  Copyright © 2015年 zhaojun. All rights reserved.
//
#include <assert.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include "MemoryTracker.h"
#include "MemoryMacro.h"

MemoryTracker& MemoryTracker::get()
{
    static MemoryTracker tracker;
    return tracker;
}

void MemoryTracker::_recordAlloc(void *ptr, size_t sz, unsigned int pool,
                                 const char* file, size_t ln, const char* func)
{
    if (mRecordEnable)
    {
        mRMutex.lock();
        assert(mAllocations.find(ptr) == mAllocations.end() && "Double allocation with same address - "
               "this probably means you have a mismatched allocation / deallocation style, "
               "check if you're are using ALLOC_T / FREE and NEW_T / DELETE_T consistently");
        mAllocations[ptr] = Alloc(sz, pool, file, ln, file);
        if (pool >= mAllocationsByPool.size())
        {
            mAllocationsByPool.resize(pool+1, 0);
        }
        mAllocationsByPool[pool] += sz;
        mTotalAllocations += sz;
        mRMutex.unlock();
    }
}

void MemoryTracker::_recordDealloc(void *ptr)
{
    if (mRecordEnable)
    {
        if (!ptr)
        {
            return;
        }
        mRMutex.lock();
        AllocationMap::iterator iter = mAllocations.find(ptr);
        assert(iter != mAllocations.end() && "Unable to locate allocation unit - "
               "this probably means you have a mismatched allocation / deallocation style, "
               "check if you're are using ALLOC_T / FREE and NEW_T / DELETE_T consistently");
        mAllocationsByPool[iter->second.pool] -= iter->second.pool;
        mTotalAllocations -= iter->second.bytes;
        mAllocations.erase(iter);
        mRMutex.unlock();
    }
}

size_t MemoryTracker::getMemoryAllocatedForPool(unsigned int pool) const
{
    return mAllocationsByPool[pool];
}

size_t MemoryTracker::getTotalMemoryAllocated() const
{
    return mTotalAllocations;
}

void MemoryTracker::reportLeaks()
{
    if (mRecordEnable)
    {
        std::stringstream os;
        if (mAllocations.empty())
        {
            os << "Memory : No Memory leaks"<<std::endl;
        }
        else
        {
            os << "Memory : Detected memory leaks !!!"<<std::endl;
            os << "Memory : ("<<mAllocations.size()<<")Allocation(s) with total "<<mTotalAllocations<<"bytes"<<std::endl;
            os << "Memory : Dumping allocations -> "<<std::endl;
            
            for (AllocationMap::const_iterator iter = mAllocations.begin(); iter != mAllocations.end(); ++iter)
            {
                const Alloc& alloc = iter->second;
                if (!alloc.filename.empty())
                {
                    os << alloc.filename;
                }
                else
                {
                    os << "(unknown source):";
                }
                
                os <<"("<<alloc.line<<"):{"<<alloc.bytes<<" bytes"<<" function: "<<alloc.function<<std::endl;
            }
            os << std::endl;
        }
        if (mDumpToStdOut)
        {
            OutputCString(os.str().c_str());
        }
        
        std::ofstream of;
        of.open(mLeakFileName.c_str());
        of << os.str();
        of.close();
    }
}