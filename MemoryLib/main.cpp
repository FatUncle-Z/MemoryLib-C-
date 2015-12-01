//
//  main.cpp
//  MemoryLib
//
//  Created by zhaojun on 15/11/30.
//  Copyright © 2015年 zhaojun. All rights reserved.
//

#include <iostream>
#include "core/MemoryNedPool.h"

int main(int argc, const char * argv[]) {
    int* ptr = (int*)NedPoolPolicy::allocateBytes(sizeof(int), __FILE__, __LINE__, __FUNCTION__);
    *ptr = 10;
    
    std::cout<<"ptr = "<<*ptr<<std::endl;
    NedPoolPolicy::deallocateBytes(ptr);
    return 0;
}
