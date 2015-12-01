//
//  MemoryMacro.h
//  MemoryLib
//
//  Created by zhaojun on 15/12/1.
//  Copyright © 2015年 zhaojun. All rights reserved.
//

#ifndef MemoryMacro_h
#define MemoryMacro_h

#include <iostream>

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef int int32;
typedef short int16;
typedef signed char int8;
// define uint64 type
#if defined( _MSC_VER )
typedef unsigned __int64 uint64;
typedef __int64 int64;
#else
typedef unsigned long long uint64;
typedef long long int64;
#endif

#if defined( _MSC_VER )
#  include<windows.h>
#  define DECL_MALLOC __declspec(restrict) __declspec(noalias)
#  define OutputCString(str) ::OutputDebugStringA(str)
#  define OutputWString(str) ::OutputDebugStringW(str)
#else
#  define DECL_MALLOC __attribute__ ((malloc))
#  define OutputCString(str) std::cerr<<str
#  define OutputWString(str) std::cerr<<str
#endif

#endif /* MemoryMacro_h */
