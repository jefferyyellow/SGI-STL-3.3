/*
 * Copyright (c) 1996-1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_STL_INTERNAL_ALLOC_H
#define __SGI_STL_INTERNAL_ALLOC_H

#ifdef __SUNPRO_CC
#  define __PRIVATE public
   // Extra access restrictions prevent us from really making some things
   // private.
#else
#  define __PRIVATE private
#endif

#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
#  define __USE_MALLOC
#endif


// This implements some standard node allocators.  These are
// NOT the same as the allocators in the C++ draft standard or in
// in the original STL.  They do not encapsulate different pointer
// types; indeed we assume that there is only one pointer type.
// The allocation primitives are intended to allocate individual objects,
// not larger arenas as with the original STL allocators.

// 这实现了一些标准的节点分配器，这是和C++标准或者原始的STL分配器不同。
// 他们没有封装不同的指针类型，事实上我们假定只有一种指针类型。
// 该分配的原意是分配单个独立的实例，不像原始STL分配器那样用于更大的场所

#ifndef __THROW_BAD_ALLOC
#  if defined(__STL_NO_BAD_ALLOC) || !defined(__STL_USE_EXCEPTIONS)
#    include <stdio.h>
#    include <stdlib.h>
#    define __THROW_BAD_ALLOC fprintf(stderr, "out of memory\n"); exit(1)
#  else /* Standard conforming out-of-memory handling */
#    include <new>
#    define __THROW_BAD_ALLOC throw std::bad_alloc()
#  endif
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifndef __RESTRICT
#  define __RESTRICT
#endif

#ifdef __STL_THREADS
# include <stl_threads.h>
# define __NODE_ALLOCATOR_THREADS true
# ifdef __STL_SGI_THREADS
  // We test whether threads are in use before locking.
  // Perhaps this should be moved into stl_threads.h, but that
  // probably makes it harder to avoid the procedure call when
  // it isn't needed.
    extern "C" {
      extern int __us_rsthread_malloc;
    }
	// The above is copied from malloc.h.  Including <malloc.h>
	// would be cleaner but fails with certain levels of standard
	// conformance.
#   define __NODE_ALLOCATOR_LOCK if (threads && __us_rsthread_malloc) \
                { _S_node_allocator_lock._M_acquire_lock(); }
#   define __NODE_ALLOCATOR_UNLOCK if (threads && __us_rsthread_malloc) \
                { _S_node_allocator_lock._M_release_lock(); }
# else /* !__STL_SGI_THREADS */
#   define __NODE_ALLOCATOR_LOCK \
        { if (threads) _S_node_allocator_lock._M_acquire_lock(); }
#   define __NODE_ALLOCATOR_UNLOCK \
        { if (threads) _S_node_allocator_lock._M_release_lock(); }
# endif
#else
//  Thread-unsafe
#   define __NODE_ALLOCATOR_LOCK
#   define __NODE_ALLOCATOR_UNLOCK
#   define __NODE_ALLOCATOR_THREADS false
#endif

__STL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#endif

// Malloc-based allocator.  Typically slower than default alloc below.
// Typically thread-safe and more storage efficient.
// 基于malloc的分配器，通常比默认的分配器慢，通常是线程安全并且存储效率更高
#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
# ifdef __DECLARE_GLOBALS_HERE
    void (* __malloc_alloc_oom_handler)() = 0;
    // g++ 2.7.2 does not handle static template data members.
# else
    extern void (* __malloc_alloc_oom_handler)();
# endif
#endif

template <int __inst>
class __malloc_alloc_template {

private:

  static void* _S_oom_malloc(size_t);
  static void* _S_oom_realloc(void*, size_t);

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
  static void (* __malloc_alloc_oom_handler)();
#endif

public:

  static void* allocate(size_t __n)
  {
    void* __result = malloc(__n);
    if (0 == __result) __result = _S_oom_malloc(__n);
    return __result;
  }

  static void deallocate(void* __p, size_t /* __n */)
  {
    free(__p);
  }

  static void* reallocate(void* __p, size_t /* old_sz */, size_t __new_sz)
  {
    void* __result = realloc(__p, __new_sz);
    if (0 == __result) __result = _S_oom_realloc(__p, __new_sz);
    return __result;
  }
  // 设置内存不足处理
  static void (* __set_malloc_handler(void (*__f)()))()
  {
    void (* __old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = __f;
    return(__old);
  }

};

// malloc_alloc out-of-memory handling
// malloc_alloc 内存不足处理
#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
template <int __inst>
void (* __malloc_alloc_template<__inst>::__malloc_alloc_oom_handler)() = 0;
#endif

template <int __inst>
void*
__malloc_alloc_template<__inst>::_S_oom_malloc(size_t __n)
{
    void (* __my_malloc_handler)();
    void* __result;

    for (;;) {
		// 内存不足的处理是否存在，如果不存在，直接__THROW_BAD_ALLOC
        __my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == __my_malloc_handler) { __THROW_BAD_ALLOC; }
		// 循环调用内存不足的处理，直到分配内存成功
        (*__my_malloc_handler)();
        __result = malloc(__n);
        if (__result) return(__result);
    }
}

template <int __inst>
void* __malloc_alloc_template<__inst>::_S_oom_realloc(void* __p, size_t __n)
{
    void (* __my_malloc_handler)();
    void* __result;

    for (;;) {
		// 内存不足的处理是否存在，如果不存在，直接__THROW_BAD_ALLOC
        __my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == __my_malloc_handler) { __THROW_BAD_ALLOC; }
		// 循环调用内存不足的处理，直到分配内存成功
        (*__my_malloc_handler)();
        __result = realloc(__p, __n);
        if (__result) return(__result);
    }
}

typedef __malloc_alloc_template<0> malloc_alloc;
// 为了符合STL规范，做了封装
template<class _Tp, class _Alloc>
class simple_alloc {

public:
    static _Tp* allocate(size_t __n)
      { return 0 == __n ? 0 : (_Tp*) _Alloc::allocate(__n * sizeof (_Tp)); }
    static _Tp* allocate(void)
      { return (_Tp*) _Alloc::allocate(sizeof (_Tp)); }
    static void deallocate(_Tp* __p, size_t __n)
      { if (0 != __n) _Alloc::deallocate(__p, __n * sizeof (_Tp)); }
    static void deallocate(_Tp* __p)
      { _Alloc::deallocate(__p, sizeof (_Tp)); }
};

// Allocator adaptor to check size arguments for debugging.
// Reports errors using assert.  Checking can be disabled with
// NDEBUG, but it's far better to just use the underlying allocator
// instead when no checking is desired.
// There is some evidence that this can confuse Purify.
// 分配器适配器为了调试检查大小参数，使用断言来报告错误。可以使用NDEBUG来禁用检查，
// 当不需要检查时最好的方式是只适用基础分配器。
// 整体的搞法就是在分配大小的基础上+_S_extra个字节，在最开始的_S_extra里面保存分配的大小
template <class _Alloc>
class debug_alloc {

private:
	// 保存大小的存储空间大小，注意：这必须足够大来保持对齐
  enum {_S_extra = 8};  // Size of space used to store size.  Note
                        // that this must be large enough to preserve
                        // alignment.

public:

  static void* allocate(size_t __n)
  {
    char* __result = (char*)_Alloc::allocate(__n + (int) _S_extra);
    *(size_t*)__result = __n;
    return __result + (int) _S_extra;
  }

  static void deallocate(void* __p, size_t __n)
  {
    char* __real_p = (char*)__p - (int) _S_extra;
    assert(*(size_t*)__real_p == __n);
    _Alloc::deallocate(__real_p, __n + (int) _S_extra);
  }

  static void* reallocate(void* __p, size_t __old_sz, size_t __new_sz)
  {
    char* __real_p = (char*)__p - (int) _S_extra;
    assert(*(size_t*)__real_p == __old_sz);
    char* __result = (char*)
      _Alloc::reallocate(__real_p, __old_sz + (int) _S_extra,
                                   __new_sz + (int) _S_extra);
    *(size_t*)__result = __new_sz;
    return __result + (int) _S_extra;
  }

};


# ifdef __USE_MALLOC

typedef malloc_alloc alloc;
typedef malloc_alloc single_client_alloc;

# else


// Default node allocator.
// With a reasonable compiler, this should be roughly as fast as the
// original STL class-specific allocators, but with less fragmentation.
// Default_alloc_template parameters are experimental and MAY
// DISAPPEAR in the future.  Clients should just use alloc for now.
// 默认的节点分配器
// 使用合适的编译器，它可以和原始STL类指定的分配器有差不多的速度，但是碎片更少。
// Default_alloc_template的参数都是实验性并且在未来可能消失，用户现在应该只使用alloc
//
// Important implementation properties:
// 1. If the client request an object of size > _MAX_BYTES, the resulting
//    object will be obtained directly from malloc.
// 2. In all other cases, we allocate an object of size exactly
//    _S_round_up(requested_size).  Thus the client has enough size
//    information that we can return the object to the proper free list
//    without permanently losing part of the object.
// 重要的实现属性：
// 1.如果用户要求一个实例的大小大于_MAX_BYTES，结果的实例会直接从malloc中获得
// 2.在其他的情况下，我们分配一个大小等于_S_round_up(requested_size)的对象。
// 因此用户有足够的大小信息，我们可以把实例返回到合适的空闲列表中去，并且永久丢失部分对象
// 
// The first template parameter specifies whether more than one thread
// may use this allocator.  It is safe to allocate an object from
// one instance of a default_alloc and deallocate it with another
// one.  This effectively transfers its ownership to the second one.
// This may have undesirable effects on reference locality.
// The second parameter is unreferenced and serves only to allow the
// creation of multiple default_alloc instances.
// Node that containers built on different allocator instances have
// different types, limiting the utility of this approach.

#if defined(__SUNPRO_CC) || defined(__GNUC__)
// breaks if we make these template class members:
  enum {_ALIGN = 8};
  enum {_MAX_BYTES = 128};
  enum {_NFREELISTS = 16}; // _MAX_BYTES/_ALIGN
#endif

template <bool threads, int inst>
class __default_alloc_template {

private:
  // Really we should use static const int x = N
  // instead of enum { x = N }, but few compilers accept the former.
#if ! (defined(__SUNPRO_CC) || defined(__GNUC__))
    enum {_ALIGN = 8};			// 小型内存块的对齐大小
    enum {_MAX_BYTES = 128};	// 小型内存块的最大大小
    enum {_NFREELISTS = 16}; // _MAX_BYTES/_ALIGN	// 空闲列表的数目
# endif
	// 将大小为_ALIGN对齐（8字节）
  static size_t
  _S_round_up(size_t __bytes) 
    { return (((__bytes) + (size_t) _ALIGN-1) & ~((size_t) _ALIGN - 1)); }

__PRIVATE:
  // 空闲列表的节点
  union _Obj {
        union _Obj* _M_free_list_link;
        char _M_client_data[1];    /* The client sees this.        */
  };
private:
	// 空闲列表
# if defined(__SUNPRO_CC) || defined(__GNUC__) || defined(__HP_aCC)
    static _Obj* __STL_VOLATILE _S_free_list[]; 
        // Specifying a size results in duplicate def for 4.1
# else
    static _Obj* __STL_VOLATILE _S_free_list[_NFREELISTS]; 
# endif
	// 获取空闲列表索引
  static  size_t _S_freelist_index(size_t __bytes) {
        return (((__bytes) + (size_t)_ALIGN-1)/(size_t)_ALIGN - 1);
  }

  // Returns an object of size __n, and optionally adds to size __n free list.
  static void* _S_refill(size_t __n);
  // Allocates a chunk for nobjs of size size.  nobjs may be reduced
  // if it is inconvenient to allocate the requested number.
  static char* _S_chunk_alloc(size_t __size, int& __nobjs);

  // Chunk allocation state.
  static char* _S_start_free;
  static char* _S_end_free;
  static size_t _S_heap_size;

# ifdef __STL_THREADS
    static _STL_mutex_lock _S_node_allocator_lock;
# endif

    // It would be nice to use _STL_auto_lock here.  But we
    // don't need the NULL check.  And we do need a test whether
    // threads have actually been started.
    class _Lock;
    friend class _Lock;
    class _Lock {
        public:
            _Lock() { __NODE_ALLOCATOR_LOCK; }
            ~_Lock() { __NODE_ALLOCATOR_UNLOCK; }
    };

public:

  /* __n must be > 0      */
	// 大小必须大于0
  static void* allocate(size_t __n)
  {
    void* __ret = 0;

	// 如果大于_MAX_BYTES直接调用直接分配
    if (__n > (size_t) _MAX_BYTES) {
      __ret = malloc_alloc::allocate(__n);
    }
    else {
		// 找到空闲列表
      _Obj* __STL_VOLATILE* __my_free_list
          = _S_free_list + _S_freelist_index(__n);
      // Acquire the lock here with a constructor call.
      // This ensures that it is released in exit or during stack
      // unwinding.
#     ifndef _NOTHREADS
      /*REFERENCED*/
      _Lock __lock_instance;
#     endif
	  // 如果空闲列表没有，就重新分配，如果空闲列表有，直接用空闲列表中的
      _Obj* __RESTRICT __result = *__my_free_list;
      if (__result == 0)
        __ret = _S_refill(_S_round_up(__n));
      else {
        *__my_free_list = __result -> _M_free_list_link;
        __ret = __result;
      }
    }

    return __ret;
  };

  /* __p may not be 0 */
  static void deallocate(void* __p, size_t __n)
  {
	  // 大小大于_MAX_BYTES直接释放
    if (__n > (size_t) _MAX_BYTES)
      malloc_alloc::deallocate(__p, __n);
    else {
		// 找到空闲列表
      _Obj* __STL_VOLATILE*  __my_free_list
          = _S_free_list + _S_freelist_index(__n);
      _Obj* __q = (_Obj*)__p;

      // acquire lock
#       ifndef _NOTHREADS
      /*REFERENCED*/
      _Lock __lock_instance;
#       endif /* _NOTHREADS */
	  // 插入空闲列表头部
      __q -> _M_free_list_link = *__my_free_list;
      *__my_free_list = __q;
      // lock is released here
    }
  }

  static void* reallocate(void* __p, size_t __old_sz, size_t __new_sz);

} ;

typedef __default_alloc_template<__NODE_ALLOCATOR_THREADS, 0> alloc;
typedef __default_alloc_template<false, 0> single_client_alloc;

template <bool __threads, int __inst>
inline bool operator==(const __default_alloc_template<__threads, __inst>&,
                       const __default_alloc_template<__threads, __inst>&)
{
  return true;
}

# ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <bool __threads, int __inst>
inline bool operator!=(const __default_alloc_template<__threads, __inst>&,
                       const __default_alloc_template<__threads, __inst>&)
{
  return false;
}
# endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */



/* We allocate memory in large chunks in order to avoid fragmenting     */
/* the malloc heap too much.                                            */
/* We assume that size is properly aligned.                             */
/* We hold the allocation lock.                                         */
// 我们分配大块内存以防止malloc堆的碎片化
// 我们假定大小是正确对齐的
// 我们持有分配锁
template <bool __threads, int __inst>
char*
__default_alloc_template<__threads, __inst>::_S_chunk_alloc(size_t __size, 
                                                            int& __nobjs)
{
    char* __result;
    size_t __total_bytes = __size * __nobjs;
    size_t __bytes_left = _S_end_free - _S_start_free;
	// 如果剩下的比想要的还多，直接剩下的分配一块
    if (__bytes_left >= __total_bytes) {
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    } 
	// 如果剩下的大于等于1个想要的大小，那就能分配几个大小算几个吧
	else if (__bytes_left >= __size) {
		// 看剩下的还能分配几个
        __nobjs = (int)(__bytes_left/__size);
        __total_bytes = __size * __nobjs;
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    } 
	// 一个都分配不了的情况
	else {
		// 计算分配的大小
        size_t __bytes_to_get = 
	  2 * __total_bytes + _S_round_up(_S_heap_size >> 4);
        // Try to make use of the left-over piece.
		// 将原来剩下的挂到空闲列表中去
        if (__bytes_left > 0) {
            _Obj* __STL_VOLATILE* __my_free_list =
                        _S_free_list + _S_freelist_index(__bytes_left);

            ((_Obj*)_S_start_free) -> _M_free_list_link = *__my_free_list;
            *__my_free_list = (_Obj*)_S_start_free;
        }
		// 直接分配
        _S_start_free = (char*)malloc(__bytes_to_get);
        if (0 == _S_start_free) {
            size_t __i;
            _Obj* __STL_VOLATILE* __my_free_list;
	    _Obj* __p;
            // Try to make do with what we have.  That can't
            // hurt.  We do not try smaller requests, since that tends
            // to result in disaster on multi-process machines.
			// 看看比他大的空闲列表中是否可以找到合适的
            for (__i = __size;
                 __i <= (size_t) _MAX_BYTES;
                 __i += (size_t) _ALIGN) {
                __my_free_list = _S_free_list + _S_freelist_index(__i);
                __p = *__my_free_list;
                if (0 != __p) {
                    *__my_free_list = __p -> _M_free_list_link;
                    _S_start_free = (char*)__p;
                    _S_end_free = _S_start_free + __i;
                    return(_S_chunk_alloc(__size, __nobjs));
                    // Any leftover piece will eventually make it to the
                    // right free list.
                }
            }
	    _S_end_free = 0;	// In case of exception.
			// 调用第一级分配器的分配函数
            _S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);
            // This should either throw an
            // exception or remedy the situation.  Thus we assume it
            // succeeded.
        }
		// 更新堆大小和目前空闲的大小
        _S_heap_size += __bytes_to_get;
        _S_end_free = _S_start_free + __bytes_to_get;
        return(_S_chunk_alloc(__size, __nobjs));
    }
}


/* Returns an object of size __n, and optionally adds to size __n free list.*/
/* We assume that __n is properly aligned.                                */
/* We hold the allocation lock.                                         */
template <bool __threads, int __inst>
void*
__default_alloc_template<__threads, __inst>::_S_refill(size_t __n)
{
    int __nobjs = 20;
    char* __chunk = _S_chunk_alloc(__n, __nobjs);
    _Obj* __STL_VOLATILE* __my_free_list;
    _Obj* __result;
    _Obj* __current_obj;
    _Obj* __next_obj;
    int __i;
	// 如果只分配到一个，直接返回
    if (1 == __nobjs) return(__chunk);
	// 得到空闲索引
    __my_free_list = _S_free_list + _S_freelist_index(__n);

    /* Build free list in chunk */
      __result = (_Obj*)__chunk;
	  // 后面一个加入空闲列表
      *__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
      for (__i = 1; ; __i++) {
        __current_obj = __next_obj;
        __next_obj = (_Obj*)((char*)__next_obj + __n);
		// 最后一个了
        if (__nobjs - 1 == __i) {
            __current_obj -> _M_free_list_link = 0;
            break;
        } else {
			// 空闲结点都串起来
            __current_obj -> _M_free_list_link = __next_obj;
        }
      }
    return(__result);
}

template <bool threads, int inst>
void*
__default_alloc_template<threads, inst>::reallocate(void* __p,
                                                    size_t __old_sz,
                                                    size_t __new_sz)
{
    void* __result;
    size_t __copy_sz;
	// 如果原始大小和新大小都大于_MAX_BYTES，直接调用realloc
    if (__old_sz > (size_t) _MAX_BYTES && __new_sz > (size_t) _MAX_BYTES) {
        return(realloc(__p, __new_sz));
    }

	// 如果老的和新的规整以后是同样大小，那原来的内存就足够使用了，返回原来的指针
    if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) return(__p);
    __result = allocate(__new_sz);
	// 找到拷贝长度，然后拷贝
    __copy_sz = __new_sz > __old_sz? __old_sz : __new_sz;
    memcpy(__result, __p, __copy_sz);
	// 释放原来的内存
    deallocate(__p, __old_sz);
    return(__result);
}

#ifdef __STL_THREADS
    template <bool __threads, int __inst>
    _STL_mutex_lock
    __default_alloc_template<__threads, __inst>::_S_node_allocator_lock
        __STL_MUTEX_INITIALIZER;
#endif


template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_start_free = 0;

template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_end_free = 0;

template <bool __threads, int __inst>
size_t __default_alloc_template<__threads, __inst>::_S_heap_size = 0;

template <bool __threads, int __inst>
typename __default_alloc_template<__threads, __inst>::_Obj* __STL_VOLATILE
__default_alloc_template<__threads, __inst> ::_S_free_list[
# if defined(__SUNPRO_CC) || defined(__GNUC__) || defined(__HP_aCC)
    _NFREELISTS
# else
    __default_alloc_template<__threads, __inst>::_NFREELISTS
# endif
] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
// The 16 zeros are necessary to make version 4.1 of the SunPro
// compiler happy.  Otherwise it appears to allocate too little
// space for the array.

#endif /* ! __USE_MALLOC */

// This implements allocators as specified in the C++ standard.  
//
// Note that standard-conforming allocators use many language features
// that are not yet widely implemented.  In particular, they rely on
// member templates, partial specialization, partial ordering of function
// templates, the typename keyword, and the use of the template keyword
// to refer to a template member of a dependent type.

#ifdef __STL_USE_STD_ALLOCATORS

template <class _Tp>
class allocator {
  typedef alloc _Alloc;          // The underlying allocator.
public:
  typedef size_t     size_type;
  typedef ptrdiff_t  difference_type;
  typedef _Tp*       pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp&       reference;
  typedef const _Tp& const_reference;
  typedef _Tp        value_type;

  template <class _Tp1> struct rebind {
    typedef allocator<_Tp1> other;
  };

  allocator() __STL_NOTHROW {}
  allocator(const allocator&) __STL_NOTHROW {}
  template <class _Tp1> allocator(const allocator<_Tp1>&) __STL_NOTHROW {}
  ~allocator() __STL_NOTHROW {}

  pointer address(reference __x) const { return &__x; }
  const_pointer address(const_reference __x) const { return &__x; }

  // __n is permitted to be 0.  The C++ standard says nothing about what
  // the return value is when __n == 0.
  _Tp* allocate(size_type __n, const void* = 0) {
    return __n != 0 ? static_cast<_Tp*>(_Alloc::allocate(__n * sizeof(_Tp))) 
                    : 0;
  }

  // __p is not permitted to be a null pointer.
  void deallocate(pointer __p, size_type __n)
    { _Alloc::deallocate(__p, __n * sizeof(_Tp)); }

  size_type max_size() const __STL_NOTHROW 
    { return size_t(-1) / sizeof(_Tp); }

  void construct(pointer __p, const _Tp& __val) { new(__p) _Tp(__val); }
  void destroy(pointer __p) { __p->~_Tp(); }
};

template<>
class allocator<void> {
public:
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;
  typedef void*       pointer;
  typedef const void* const_pointer;
  typedef void        value_type;

  template <class _Tp1> struct rebind {
    typedef allocator<_Tp1> other;
  };
};


template <class _T1, class _T2>
inline bool operator==(const allocator<_T1>&, const allocator<_T2>&) 
{
  return true;
}

template <class _T1, class _T2>
inline bool operator!=(const allocator<_T1>&, const allocator<_T2>&)
{
  return false;
}

// Allocator adaptor to turn an SGI-style allocator (e.g. alloc, malloc_alloc)
// into a standard-conforming allocator.   Note that this adaptor does
// *not* assume that all objects of the underlying alloc class are
// identical, nor does it assume that all of the underlying alloc's
// member functions are static member functions.  Note, also, that 
// __allocator<_Tp, alloc> is essentially the same thing as allocator<_Tp>.

template <class _Tp, class _Alloc>
struct __allocator {
  _Alloc __underlying_alloc;

  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef _Tp*       pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp&       reference;
  typedef const _Tp& const_reference;
  typedef _Tp        value_type;

  template <class _Tp1> struct rebind {
    typedef __allocator<_Tp1, _Alloc> other;
  };

  __allocator() __STL_NOTHROW {}
  __allocator(const __allocator& __a) __STL_NOTHROW
    : __underlying_alloc(__a.__underlying_alloc) {}
  template <class _Tp1> 
  __allocator(const __allocator<_Tp1, _Alloc>& __a) __STL_NOTHROW
    : __underlying_alloc(__a.__underlying_alloc) {}
  ~__allocator() __STL_NOTHROW {}

  pointer address(reference __x) const { return &__x; }
  const_pointer address(const_reference __x) const { return &__x; }

  // __n is permitted to be 0.
  _Tp* allocate(size_type __n, const void* = 0) {
    return __n != 0 
        ? static_cast<_Tp*>(__underlying_alloc.allocate(__n * sizeof(_Tp))) 
        : 0;
  }

  // __p is not permitted to be a null pointer.
  void deallocate(pointer __p, size_type __n)
    { __underlying_alloc.deallocate(__p, __n * sizeof(_Tp)); }

  size_type max_size() const __STL_NOTHROW 
    { return size_t(-1) / sizeof(_Tp); }

  void construct(pointer __p, const _Tp& __val) { new(__p) _Tp(__val); }
  void destroy(pointer __p) { __p->~_Tp(); }
};

template <class _Alloc>
class __allocator<void, _Alloc> {
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;
  typedef void*       pointer;
  typedef const void* const_pointer;
  typedef void        value_type;

  template <class _Tp1> struct rebind {
    typedef __allocator<_Tp1, _Alloc> other;
  };
};

template <class _Tp, class _Alloc>
inline bool operator==(const __allocator<_Tp, _Alloc>& __a1,
                       const __allocator<_Tp, _Alloc>& __a2)
{
  return __a1.__underlying_alloc == __a2.__underlying_alloc;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <class _Tp, class _Alloc>
inline bool operator!=(const __allocator<_Tp, _Alloc>& __a1,
                       const __allocator<_Tp, _Alloc>& __a2)
{
  return __a1.__underlying_alloc != __a2.__underlying_alloc;
}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

// Comparison operators for all of the predifined SGI-style allocators.
// This ensures that __allocator<malloc_alloc> (for example) will
// work correctly.

template <int inst>
inline bool operator==(const __malloc_alloc_template<inst>&,
                       const __malloc_alloc_template<inst>&)
{
  return true;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <int __inst>
inline bool operator!=(const __malloc_alloc_template<__inst>&,
                       const __malloc_alloc_template<__inst>&)
{
  return false;
}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */


template <class _Alloc>
inline bool operator==(const debug_alloc<_Alloc>&,
                       const debug_alloc<_Alloc>&) {
  return true;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <class _Alloc>
inline bool operator!=(const debug_alloc<_Alloc>&,
                       const debug_alloc<_Alloc>&) {
  return false;
}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

// Another allocator adaptor: _Alloc_traits.  This serves two
// purposes.  First, make it possible to write containers that can use
// either SGI-style allocators or standard-conforming allocator.
// Second, provide a mechanism so that containers can query whether or
// not the allocator has distinct instances.  If not, the container
// can avoid wasting a word of memory to store an empty object.

// This adaptor uses partial specialization.  The general case of
// _Alloc_traits<_Tp, _Alloc> assumes that _Alloc is a
// standard-conforming allocator, possibly with non-equal instances
// and non-static members.  (It still behaves correctly even if _Alloc
// has static member and if all instances are equal.  Refinements
// affect performance, not correctness.)

// There are always two members: allocator_type, which is a standard-
// conforming allocator type for allocating objects of type _Tp, and
// _S_instanceless, a static const member of type bool.  If
// _S_instanceless is true, this means that there is no difference
// between any two instances of type allocator_type.  Furthermore, if
// _S_instanceless is true, then _Alloc_traits has one additional
// member: _Alloc_type.  This type encapsulates allocation and
// deallocation of objects of type _Tp through a static interface; it
// has two member functions, whose signatures are
//    static _Tp* allocate(size_t)
//    static void deallocate(_Tp*, size_t)

// The fully general version.

template <class _Tp, class _Allocator>
struct _Alloc_traits
{
  static const bool _S_instanceless = false;
  typedef typename _Allocator::__STL_TEMPLATE rebind<_Tp>::other 
          allocator_type;
};

template <class _Tp, class _Allocator>
const bool _Alloc_traits<_Tp, _Allocator>::_S_instanceless;

// The version for the default allocator.

template <class _Tp, class _Tp1>
struct _Alloc_traits<_Tp, allocator<_Tp1> >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, alloc> _Alloc_type;
  typedef allocator<_Tp> allocator_type;
};

// Versions for the predefined SGI-style allocators.

template <class _Tp, int __inst>
struct _Alloc_traits<_Tp, __malloc_alloc_template<__inst> >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __malloc_alloc_template<__inst> > _Alloc_type;
  typedef __allocator<_Tp, __malloc_alloc_template<__inst> > allocator_type;
};

template <class _Tp, bool __threads, int __inst>
struct _Alloc_traits<_Tp, __default_alloc_template<__threads, __inst> >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __default_alloc_template<__threads, __inst> > 
          _Alloc_type;
  typedef __allocator<_Tp, __default_alloc_template<__threads, __inst> > 
          allocator_type;
};

template <class _Tp, class _Alloc>
struct _Alloc_traits<_Tp, debug_alloc<_Alloc> >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, debug_alloc<_Alloc> > _Alloc_type;
  typedef __allocator<_Tp, debug_alloc<_Alloc> > allocator_type;
};

// Versions for the __allocator adaptor used with the predefined
// SGI-style allocators.

template <class _Tp, class _Tp1, int __inst>
struct _Alloc_traits<_Tp, 
                     __allocator<_Tp1, __malloc_alloc_template<__inst> > >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __malloc_alloc_template<__inst> > _Alloc_type;
  typedef __allocator<_Tp, __malloc_alloc_template<__inst> > allocator_type;
};

template <class _Tp, class _Tp1, bool __thr, int __inst>
struct _Alloc_traits<_Tp, 
                      __allocator<_Tp1, 
                                  __default_alloc_template<__thr, __inst> > >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __default_alloc_template<__thr,__inst> > 
          _Alloc_type;
  typedef __allocator<_Tp, __default_alloc_template<__thr,__inst> > 
          allocator_type;
};

template <class _Tp, class _Tp1, class _Alloc>
struct _Alloc_traits<_Tp, __allocator<_Tp1, debug_alloc<_Alloc> > >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, debug_alloc<_Alloc> > _Alloc_type;
  typedef __allocator<_Tp, debug_alloc<_Alloc> > allocator_type;
};


#endif /* __STL_USE_STD_ALLOCATORS */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#endif

__STL_END_NAMESPACE

#undef __PRIVATE

#endif /* __SGI_STL_INTERNAL_ALLOC_H */

// Local Variables:
// mode:C++
// End:
