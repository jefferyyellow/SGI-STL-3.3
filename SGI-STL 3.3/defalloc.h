/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

// Inclusion of this file is DEPRECATED.  This is the original HP
// default allocator.  It is provided only for backward compatibility.
// This file WILL BE REMOVED in a future release.
//
// DO NOT USE THIS FILE unless you have an old container implementation
// that requires an allocator with the HP-style interface.  
//
// Standard-conforming allocators have a very different interface.  The
// standard default allocator is declared in the header <memory>.

// 注意注释的几个警告：
// 不建议包含该文件，这只是原始HP的风格的默认分配器，提供它只是为了向后兼容，
// 该文件在后续版本中将会被移除

// 不要使用该文件！！！除非你使用老的容器实现，该容器需要HP风格接口的分配器

// 符合标准的分配器具有完全不同的接口，在<memory>的前面声明了标准的默认分配器

#ifndef DEFALLOC_H
#define DEFALLOC_H

#include <new.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream.h>
#include <algobase.h>


template <class T>
inline T* allocate(ptrdiff_t size, T*) {
	// 卸载目前的内存分配异常处理函数
    set_new_handler(0);
	// 分配size个T的空间
    T* tmp = (T*)(::operator new((size_t)(size * sizeof(T))));
    if (tmp == 0) {
	cerr << "out of memory" << endl; 
	exit(1);
    }
    return tmp;
}


template <class T>
inline void deallocate(T* buffer) {
    ::operator delete(buffer);
}
// SGI-STL默认的分配器为alloc而不是allocator
template <class T>
class allocator {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    pointer allocate(size_type n) { 
	return ::allocate((difference_type)n, (pointer)0);
    }
    void deallocate(pointer p) { ::deallocate(p); }
    pointer address(reference x) { return (pointer)&x; }
    const_pointer const_address(const_reference x) { 
	return (const_pointer)&x; 
    }
    size_type init_page_size() { 
	return max(size_type(1), size_type(4096/sizeof(T))); 
    }
    size_type max_size() const { 
	return max(size_type(1), size_type(UINT_MAX/sizeof(T))); 
    }
};

// 特化版本
class allocator<void> {
public:
    typedef void* pointer;
};



#endif
