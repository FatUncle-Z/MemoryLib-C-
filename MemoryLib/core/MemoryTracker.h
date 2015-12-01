//
//  MemoryTracker.h
//  MemoryLib
//
//  Created by zhaojun on 15/12/1.
//  Copyright © 2015年 zhaojun. All rights reserved.
//

#ifndef MemoryTracker_h
#define MemoryTracker_h

#include <unordered_map>
#include <vector>
#include <mutex>
#include <string>

class MemoryTracker
{
protected:
    std::recursive_mutex mRMutex;
    
    // Allocation record
    struct Alloc
    {
        size_t         bytes;
        unsigned int   pool;
        std::string    filename;
        size_t         line;
        std::string    function;
        
        Alloc():bytes(0), line(0){}
        Alloc(size_t sz, unsigned int p, const char* file, size_t ln, const char* func):bytes(sz), pool(p), line(ln)
        {
            if(file)
            {
                filename = file;
            }
            
            if (func)
            {
                function = func;
            }
        }
    };
    
    std::string mLeakFileName;
    bool mDumpToStdOut;
    typedef std::unordered_map<void*, Alloc> AllocationMap;
    AllocationMap mAllocations;
    
    size_t mTotalAllocations;
    typedef std::vector<size_t> AllocationsByPool;
    AllocationsByPool mAllocationsByPool;
    bool mRecordEnable;
    
    void reportLeaks();
    
    MemoryTracker():mLeakFileName("Leaks.log")
    , mDumpToStdOut(true)
    , mTotalAllocations(0)
    , mRecordEnable(true)
    {
        
    }
    
public:
    void setReportFileName(const std::string& name)
    {
        mLeakFileName = name;
    }
    
    const std::string& getReportFileName() const
    {
        return mLeakFileName;
    }
    
    void setReportToStdOut(bool rep)
    {
        mDumpToStdOut = rep;
    }
    
    bool getReportToStdOut()
    {
        return mDumpToStdOut;
    }
    
    
    size_t getTotalMemoryAllocated() const;
    
    size_t getMemoryAllocatedForPool(unsigned int pool) const;
    
    void _recordAlloc(void* ptr, size_t sz, unsigned int pool = 0,
                      const char* file = 0, size_t ln = 0, const char* func = 0);
    
    void _recordDealloc(void* ptr);
    
    void setRecordEnable(bool recordEnable)
    {
        mRecordEnable = recordEnable;
    }
    
    bool getRecordEnable() const
    {
        return mRecordEnable;
    }
    
    ~MemoryTracker()
    {
        reportLeaks();
        mRecordEnable = false;
    }
    
    
    static MemoryTracker& get();
};

#endif /* MemoryTracker_h */
