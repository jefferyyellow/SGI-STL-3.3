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
 *
 * Copyright (c) 1997
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

#include <concept_checks.h>

#ifndef __SGI_STL_INTERNAL_DEQUE_H
#define __SGI_STL_INTERNAL_DEQUE_H

/* Class invariants:
 *  For any nonsingular iterator i:
 *    i.node is the address of an element in the map array.  The
 *      contents of i.node is a pointer to the beginning of a node.
 *    i.first == *(i.node) 
 *    i.last  == i.first + node_size
 *    i.cur is a pointer in the range [i.first, i.last).  NOTE:
 *      the implication of this is that i.cur is always a dereferenceable
 *      pointer, even if i is a past-the-end iterator.
 *  Start and Finish are always nonsingular iterators.  NOTE: this means
 *    that an empty deque must have one node, and that a deque
 *    with N elements, where N is the buffer size, must have two nodes.
 *  For every node other than start.node and finish.node, every element
 *    in the node is an initialized object.  If start.node == finish.node,
 *    then [start.cur, finish.cur) are initialized objects, and
 *    the elements outside that range are uninitialized storage.  Otherwise,
 *    [start.cur, start.last) and [finish.first, finish.cur) are initialized
 *    objects, and [start.first, start.cur) and [finish.cur, finish.last)
 *    are uninitialized storage.
 *  [map, map + map_size) is a valid, non-empty range.  
 *  [start.node, finish.node] is a valid range contained within 
 *    [map, map + map_size).  
 *  A pointer in the range [map, map + map_size) points to an allocated node
 *    if and only if the pointer is in the range [start.node, finish.node].
 */


/*
 * In previous versions of deque, there was an extra template 
 * parameter so users could control the node size.  This extension
 * turns out to violate the C++ standard (it can be detected using
 * template template parameters), and it has been removed.
 */
// 前一个版本的deque，有一个额外的模板参数，以便用户能控制节点大小，该扩展事实证明违反了C++标准，
// （可以使用模板参数检测），并且已经被删除了
__STL_BEGIN_NAMESPACE 

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif

// Note: this function is simply a kludge to work around several compilers'
//  bugs in handling constant expressions.
// 这个函数是方便不同编译器处理常量表达式的bug
// 小于512，一个内存节点512/__size个元素，超过512就是一个内存节点一个元素
inline size_t __deque_buf_size(size_t __size) {
  return __size < 512 ? size_t(512 / __size) : size_t(1);
}

template <class _Tp, class _Ref, class _Ptr>
struct _Deque_iterator {
  typedef _Deque_iterator<_Tp, _Tp&, _Tp*>             iterator;
  typedef _Deque_iterator<_Tp, const _Tp&, const _Tp*> const_iterator;
  static size_t _S_buffer_size() { return __deque_buf_size(sizeof(_Tp)); }

  typedef random_access_iterator_tag iterator_category;
  typedef _Tp value_type;
  typedef _Ptr pointer;
  typedef _Ref reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef _Tp** _Map_pointer;

  typedef _Deque_iterator _Self;
  // 当前元素指针
  _Tp* _M_cur;
  // 第一个元素指针
  _Tp* _M_first;
  // 最后一个元素指针
  _Tp* _M_last;
  // 内存节点指针
  _Map_pointer _M_node;

  _Deque_iterator(_Tp* __x, _Map_pointer __y) 
    : _M_cur(__x), _M_first(*__y),
      _M_last(*__y + _S_buffer_size()), _M_node(__y) {}
  _Deque_iterator() : _M_cur(0), _M_first(0), _M_last(0), _M_node(0) {}
  _Deque_iterator(const iterator& __x)
    : _M_cur(__x._M_cur), _M_first(__x._M_first), 
      _M_last(__x._M_last), _M_node(__x._M_node) {}

  reference operator*() const { return *_M_cur; }
#ifndef __SGI_STL_NO_ARROW_OPERATOR
  pointer operator->() const { return _M_cur; }
#endif /* __SGI_STL_NO_ARROW_OPERATOR */

  difference_type operator-(const _Self& __x) const {
	  // 每个内存节点_S_buffer_size()个元素
    return difference_type(_S_buffer_size()) * (_M_node - __x._M_node - 1) +
      (_M_cur - _M_first) + (__x._M_last - __x._M_cur);
  }

  _Self& operator++() {
    ++_M_cur;
	// 如果到了最末端，换下一个内存节点
    if (_M_cur == _M_last) {
		// 设置新的内存节点了
      _M_set_node(_M_node + 1);
	  // 同步_M_cur
      _M_cur = _M_first;
    }
    return *this; 
  }
  _Self operator++(int)  {
    _Self __tmp = *this;
    ++*this;
    return __tmp;
  }

  _Self& operator--() {
	  // 如果到最前端，换上一个内存节点
    if (_M_cur == _M_first) {
		// 设置新的内存节点了
      _M_set_node(_M_node - 1);
	  // 同步_M_cur
      _M_cur = _M_last;
    }
    --_M_cur;
    return *this;
  }
  _Self operator--(int) {
    _Self __tmp = *this;
    --*this;
    return __tmp;
  }

  _Self& operator+=(difference_type __n)
  {
	  // 计算在当前内存节点的偏移
    difference_type __offset = __n + (_M_cur - _M_first);
	// 如果还在内存节点内
    if (__offset >= 0 && __offset < difference_type(_S_buffer_size()))
      _M_cur += __n;
    else {
		// 超出节点的情况
		// 如果__offset > 0, __offset / _S_buffer_size()取整
		// 如果__offset < 0, -((-__offset - 1) / _S_buffer_size()) - 1
      difference_type __node_offset =
        __offset > 0 ? __offset / difference_type(_S_buffer_size())
                   : -difference_type((-__offset - 1) / _S_buffer_size()) - 1;
      _M_set_node(_M_node + __node_offset);
	  // 计算当前的元素指针
      _M_cur = _M_first + 
        (__offset - __node_offset * difference_type(_S_buffer_size()));
    }
    return *this;
  }

  _Self operator+(difference_type __n) const
  {
    _Self __tmp = *this;
    return __tmp += __n;
  }
  
  _Self& operator-=(difference_type __n) { return *this += -__n; }
 
  _Self operator-(difference_type __n) const {
    _Self __tmp = *this;
    return __tmp -= __n;
  }

  reference operator[](difference_type __n) const { return *(*this + __n); }

  bool operator==(const _Self& __x) const { return _M_cur == __x._M_cur; }
  bool operator!=(const _Self& __x) const { return !(*this == __x); }
  bool operator<(const _Self& __x) const {
    return (_M_node == __x._M_node) ? 
      (_M_cur < __x._M_cur) : (_M_node < __x._M_node);
  }
  bool operator>(const _Self& __x) const  { return __x < *this; }
  bool operator<=(const _Self& __x) const { return !(__x < *this); }
  bool operator>=(const _Self& __x) const { return !(*this < __x); }
  // 设置新的节点
  void _M_set_node(_Map_pointer __new_node) {
    _M_node = __new_node;
    _M_first = *__new_node;
    _M_last = _M_first + difference_type(_S_buffer_size());
  }
};

template <class _Tp, class _Ref, class _Ptr>
inline _Deque_iterator<_Tp, _Ref, _Ptr>
operator+(ptrdiff_t __n, const _Deque_iterator<_Tp, _Ref, _Ptr>& __x)
{
  return __x + __n;
}

#ifndef __STL_CLASS_PARTIAL_SPECIALIZATION

template <class _Tp, class _Ref, class _Ptr>
inline random_access_iterator_tag
iterator_category(const _Deque_iterator<_Tp,_Ref,_Ptr>&)
{
  return random_access_iterator_tag();
}

template <class _Tp, class _Ref, class _Ptr>
inline _Tp* value_type(const _Deque_iterator<_Tp,_Ref,_Ptr>&) { return 0; }

template <class _Tp, class _Ref, class _Ptr>
inline ptrdiff_t* distance_type(const _Deque_iterator<_Tp,_Ref,_Ptr>&) {
  return 0;
}

#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

// Deque base class.  It has two purposes.  First, its constructor
//  and destructor allocate (but don't initialize) storage.  This makes
//  exception safety easier.  Second, the base class encapsulates all of
//  the differences between SGI-style allocators and standard-conforming
//  allocators.
// Deque的基类，两个目的：第一：他构造和析构存储器（并不初始化）。这使得异常安全更加容易
// 第二：该基类封装了所有SGI风格的分配器和符合标准分配器之间的差异
#ifdef __STL_USE_STD_ALLOCATORS

// Base class for ordinary allocators.
// 
template <class _Tp, class _Alloc, bool __is_static>
class _Deque_alloc_base {
public:
  typedef typename _Alloc_traits<_Tp,_Alloc>::allocator_type allocator_type;
  allocator_type get_allocator() const { return _M_node_allocator; }

  _Deque_alloc_base(const allocator_type& __a)
    : _M_node_allocator(__a), _M_map_allocator(__a),
      _M_map(0), _M_map_size(0)
  {}
  
protected:
  typedef typename _Alloc_traits<_Tp*, _Alloc>::allocator_type
          _Map_allocator_type;

  allocator_type      _M_node_allocator;
  _Map_allocator_type _M_map_allocator;
  // 分配内存节点
  _Tp* _M_allocate_node() {
    return _M_node_allocator.allocate(__deque_buf_size(sizeof(_Tp)));
  }
  // 释放内存节点
  void _M_deallocate_node(_Tp* __p) {
    _M_node_allocator.deallocate(__p, __deque_buf_size(sizeof(_Tp)));
  }
  // 分配内存节点指针数组内存
  _Tp** _M_allocate_map(size_t __n) 
    { return _M_map_allocator.allocate(__n); }
  // 释放内存节点指针数组内存
  void _M_deallocate_map(_Tp** __p, size_t __n) 
    { _M_map_allocator.deallocate(__p, __n); }
  // 内存节点指针数组
  _Tp** _M_map;
  // 内存节点大小
  size_t _M_map_size;
};

// Specialization for instanceless allocators.
// _Deque_alloc_base的特化版本，
template <class _Tp, class _Alloc>
class _Deque_alloc_base<_Tp, _Alloc, true>
{
public:
  typedef typename _Alloc_traits<_Tp,_Alloc>::allocator_type allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Deque_alloc_base(const allocator_type&) : _M_map(0), _M_map_size(0) {}
  
protected:
  typedef typename _Alloc_traits<_Tp, _Alloc>::_Alloc_type _Node_alloc_type;
  typedef typename _Alloc_traits<_Tp*, _Alloc>::_Alloc_type _Map_alloc_type;
  // 分配内存节点
  _Tp* _M_allocate_node() {
    return _Node_alloc_type::allocate(__deque_buf_size(sizeof(_Tp)));
  }
  // 释放内存节点
  void _M_deallocate_node(_Tp* __p) {
    _Node_alloc_type::deallocate(__p, __deque_buf_size(sizeof(_Tp)));
  }
  // 分配内存节点指针数组内存
  _Tp** _M_allocate_map(size_t __n) 
    { return _Map_alloc_type::allocate(__n); }
  // 释放内存节点指针数组内存
  void _M_deallocate_map(_Tp** __p, size_t __n) 
    { _Map_alloc_type::deallocate(__p, __n); }

  // 内存节点指针数组
  _Tp** _M_map;
  // 内存节点指针数组大小
  size_t _M_map_size;
};

template <class _Tp, class _Alloc>
class _Deque_base
  : public _Deque_alloc_base<_Tp,_Alloc,
                              _Alloc_traits<_Tp, _Alloc>::_S_instanceless>
{
public:
  typedef _Deque_alloc_base<_Tp,_Alloc,
                             _Alloc_traits<_Tp, _Alloc>::_S_instanceless>
          _Base;
  typedef typename _Base::allocator_type allocator_type;
  typedef _Deque_iterator<_Tp,_Tp&,_Tp*>             iterator;
  typedef _Deque_iterator<_Tp,const _Tp&,const _Tp*> const_iterator;

  // _M_initialize_map初始化内存节点指针数组
  _Deque_base(const allocator_type& __a, size_t __num_elements)
    : _Base(__a), _M_start(), _M_finish()
    { _M_initialize_map(__num_elements); }
  _Deque_base(const allocator_type& __a) 
    : _Base(__a), _M_start(), _M_finish() {}
  ~_Deque_base();    

protected:
  void _M_initialize_map(size_t);
  void _M_create_nodes(_Tp** __nstart, _Tp** __nfinish);
  void _M_destroy_nodes(_Tp** __nstart, _Tp** __nfinish);
  enum { _S_initial_map_size = 8 };

protected:
  iterator _M_start;
  iterator _M_finish;
};

#else /* __STL_USE_STD_ALLOCATORS */

template <class _Tp, class _Alloc>
class _Deque_base {
public:
  typedef _Deque_iterator<_Tp,_Tp&,_Tp*>             iterator;
  typedef _Deque_iterator<_Tp,const _Tp&,const _Tp*> const_iterator;

  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Deque_base(const allocator_type&, size_t __num_elements)
    : _M_map(0), _M_map_size(0),  _M_start(), _M_finish() {
    _M_initialize_map(__num_elements);
  }
  _Deque_base(const allocator_type&)
    : _M_map(0), _M_map_size(0),  _M_start(), _M_finish() {}
  ~_Deque_base();    

protected:
  void _M_initialize_map(size_t);
  void _M_create_nodes(_Tp** __nstart, _Tp** __nfinish);
  void _M_destroy_nodes(_Tp** __nstart, _Tp** __nfinish);
  enum { _S_initial_map_size = 8 };

protected:
  _Tp** _M_map;
  size_t _M_map_size;  
  iterator _M_start;
  iterator _M_finish;

  typedef simple_alloc<_Tp, _Alloc>  _Node_alloc_type;
  typedef simple_alloc<_Tp*, _Alloc> _Map_alloc_type;
  // 分配内存节点
  _Tp* _M_allocate_node()
    { return _Node_alloc_type::allocate(__deque_buf_size(sizeof(_Tp))); }
   // 分配内存节点指针数组内存
  void _M_deallocate_node(_Tp* __p)
    { _Node_alloc_type::deallocate(__p, __deque_buf_size(sizeof(_Tp))); }
   // 释放内存节点指针数组内存
  _Tp** _M_allocate_map(size_t __n) 
    { return _Map_alloc_type::allocate(__n); }
  void _M_deallocate_map(_Tp** __p, size_t __n) 
    { _Map_alloc_type::deallocate(__p, __n); }
};

#endif /* __STL_USE_STD_ALLOCATORS */

// Non-inline member functions from _Deque_base.

template <class _Tp, class _Alloc>
_Deque_base<_Tp,_Alloc>::~_Deque_base() {
  if (_M_map) {
	  // 释放内存节点
    _M_destroy_nodes(_M_start._M_node, _M_finish._M_node + 1);
	// 释放内存节点指针数组内存
    _M_deallocate_map(_M_map, _M_map_size);
  }
}

// 初始化内存节点指针数组和内存节点
template <class _Tp, class _Alloc>
void
_Deque_base<_Tp,_Alloc>::_M_initialize_map(size_t __num_elements)
{
	// 计算内存节点数目
  size_t __num_nodes = 
    __num_elements / __deque_buf_size(sizeof(_Tp)) + 1;
  // 计算内存节点数组的大小，最小_S_initial_map_size，最少前后预留一个内存节点指针空位
  _M_map_size = max((size_t) _S_initial_map_size, __num_nodes + 2);
  // 计算内存节点数组大小
  _M_map = _M_allocate_map(_M_map_size);
  // 取中间部分拿来用
  _Tp** __nstart = _M_map + (_M_map_size - __num_nodes) / 2;
  _Tp** __nfinish = __nstart + __num_nodes;
    
	// 创建内存节点
  __STL_TRY {
    _M_create_nodes(__nstart, __nfinish);
  }
  __STL_UNWIND((_M_deallocate_map(_M_map, _M_map_size), 
                _M_map = 0, _M_map_size = 0));
  // 初始化开始和结束两个迭代器
  _M_start._M_set_node(__nstart);
  _M_finish._M_set_node(__nfinish - 1);
  _M_start._M_cur = _M_start._M_first;
  _M_finish._M_cur = _M_finish._M_first +
               __num_elements % __deque_buf_size(sizeof(_Tp));
}

// 根据内存节点指针数组的前后地址，创建内存节点
template <class _Tp, class _Alloc>
void _Deque_base<_Tp,_Alloc>::_M_create_nodes(_Tp** __nstart, _Tp** __nfinish)
{
  _Tp** __cur;
  __STL_TRY {
    for (__cur = __nstart; __cur < __nfinish; ++__cur)
      *__cur = _M_allocate_node();
  }
  __STL_UNWIND(_M_destroy_nodes(__nstart, __cur));
}

// 根据内存节点指针数组的前后地址，销毁内存节点
template <class _Tp, class _Alloc>
void
_Deque_base<_Tp,_Alloc>::_M_destroy_nodes(_Tp** __nstart, _Tp** __nfinish)
{
  for (_Tp** __n = __nstart; __n < __nfinish; ++__n)
    _M_deallocate_node(*__n);
}

template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp) >
class deque : protected _Deque_base<_Tp, _Alloc> {

  // requirements:

  __STL_CLASS_REQUIRES(_Tp, _Assignable);

  typedef _Deque_base<_Tp, _Alloc> _Base;
public:                         // Basic types
  typedef _Tp value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef typename _Base::allocator_type allocator_type;
  allocator_type get_allocator() const { return _Base::get_allocator(); }

public:                         // Iterators
  typedef typename _Base::iterator       iterator;
  typedef typename _Base::const_iterator const_iterator;

#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
  typedef reverse_iterator<const_iterator> const_reverse_iterator;
  typedef reverse_iterator<iterator> reverse_iterator;
#else /* __STL_CLASS_PARTIAL_SPECIALIZATION */
  typedef reverse_iterator<const_iterator, value_type, const_reference, 
                           difference_type>  
          const_reverse_iterator;
  typedef reverse_iterator<iterator, value_type, reference, difference_type>
          reverse_iterator; 
#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

protected:                      // Internal typedefs
  typedef pointer* _Map_pointer;
  // 一个内存节点的元素数目
  static size_t _S_buffer_size() { return __deque_buf_size(sizeof(_Tp)); }

protected:
#ifdef __STL_USE_NAMESPACES
  using _Base::_M_initialize_map;
  using _Base::_M_create_nodes;
  using _Base::_M_destroy_nodes;
  using _Base::_M_allocate_node;
  using _Base::_M_deallocate_node;
  using _Base::_M_allocate_map;
  using _Base::_M_deallocate_map;

  using _Base::_M_map;
  using _Base::_M_map_size;
  using _Base::_M_start;
  using _Base::_M_finish;
#endif /* __STL_USE_NAMESPACES */

public:                         // Basic accessors
  iterator begin() { return _M_start; }
  iterator end() { return _M_finish; }
  const_iterator begin() const { return _M_start; }
  const_iterator end() const { return _M_finish; }

  reverse_iterator rbegin() { return reverse_iterator(_M_finish); }
  reverse_iterator rend() { return reverse_iterator(_M_start); }
  const_reverse_iterator rbegin() const 
    { return const_reverse_iterator(_M_finish); }
  const_reverse_iterator rend() const 
    { return const_reverse_iterator(_M_start); }

  reference operator[](size_type __n)
    { return _M_start[difference_type(__n)]; }
  const_reference operator[](size_type __n) const 
    { return _M_start[difference_type(__n)]; }

#ifdef __STL_THROW_RANGE_ERRORS
  void _M_range_check(size_type __n) const {
    if (__n >= this->size())
      __stl_throw_range_error("deque");
  }

  reference at(size_type __n)
    { _M_range_check(__n); return (*this)[__n]; }
  const_reference at(size_type __n) const
    { _M_range_check(__n); return (*this)[__n]; }
#endif /* __STL_THROW_RANGE_ERRORS */

  reference front() { return *_M_start; }
  reference back() {
    iterator __tmp = _M_finish;
    --__tmp;
    return *__tmp;
  }
  const_reference front() const { return *_M_start; }
  const_reference back() const {
    const_iterator __tmp = _M_finish;
    --__tmp;
    return *__tmp;
  }
 // 查看迭代器重载的减号操作符，计算deque的大小
  size_type size() const { return _M_finish - _M_start; }
  size_type max_size() const { return size_type(-1); }
  // 查看迭代器重载的==操作符
  bool empty() const { return _M_finish == _M_start; }

public:                         // Constructor, destructor.
  explicit deque(const allocator_type& __a = allocator_type()) 
    : _Base(__a, 0) {}
  deque(const deque& __x) : _Base(__x.get_allocator(), __x.size()) 
    { uninitialized_copy(__x.begin(), __x.end(), _M_start); }
  deque(size_type __n, const value_type& __value,
        const allocator_type& __a = allocator_type()) : _Base(__a, __n)
    { _M_fill_initialize(__value); }
  explicit deque(size_type __n) : _Base(allocator_type(), __n)
    { _M_fill_initialize(value_type()); }

#ifdef __STL_MEMBER_TEMPLATES

  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InputIterator>
  deque(_InputIterator __first, _InputIterator __last,
        const allocator_type& __a = allocator_type()) : _Base(__a) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_initialize_dispatch(__first, __last, _Integral());
  }

  template <class _Integer>
  void _M_initialize_dispatch(_Integer __n, _Integer __x, __true_type) {
    _M_initialize_map(__n);
    _M_fill_initialize(__x);
  }

  template <class _InputIter>
  void _M_initialize_dispatch(_InputIter __first, _InputIter __last,
                              __false_type) {
    _M_range_initialize(__first, __last, __ITERATOR_CATEGORY(__first));
  }

#else /* __STL_MEMBER_TEMPLATES */

  deque(const value_type* __first, const value_type* __last,
        const allocator_type& __a = allocator_type()) 
    : _Base(__a, __last - __first)
    { uninitialized_copy(__first, __last, _M_start); }
  deque(const_iterator __first, const_iterator __last,
        const allocator_type& __a = allocator_type()) 
    : _Base(__a, __last - __first)
    { uninitialized_copy(__first, __last, _M_start); }

#endif /* __STL_MEMBER_TEMPLATES */

  ~deque() { destroy(_M_start, _M_finish); }

  deque& operator= (const deque& __x) {
    const size_type __len = size();
    if (&__x != this) {
      if (__len >= __x.size())
		  // 拷贝内容，把多余的删除掉
        erase(copy(__x.begin(), __x.end(), _M_start), _M_finish);
      else {
        const_iterator __mid = __x.begin() + difference_type(__len);
		// 拷贝内容
        copy(__x.begin(), __mid, _M_start);
		// 将没有放下的插入在后面
        insert(_M_finish, __mid, __x.end());
      }
    }
    return *this;
  }        

  void swap(deque& __x) {
    __STD::swap(_M_start, __x._M_start);
    __STD::swap(_M_finish, __x._M_finish);
    __STD::swap(_M_map, __x._M_map);
    __STD::swap(_M_map_size, __x._M_map_size);
  }

public: 
  // assign(), a generalized assignment member function.  Two
  // versions: one that takes a count, and one that takes a range.
  // The range version is a member template, so we dispatch on whether
  // or not the type is an integer.

  void _M_fill_assign(size_type __n, const _Tp& __val) {
    if (__n > size()) {
		// 将原来的元素赋值
      fill(begin(), end(), __val);
	  // 插入多余的
      insert(end(), __n - size(), __val);
    }
    else {
		// 删除多余的
      erase(begin() + __n, end());
	  // 剩下的赋值
      fill(begin(), end(), __val);
    }
  }

  void assign(size_type __n, const _Tp& __val) {
    _M_fill_assign(__n, __val);
  }

#ifdef __STL_MEMBER_TEMPLATES

  template <class _InputIterator>
  void assign(_InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_assign_dispatch(__first, __last, _Integral());
  }

private:                        // helper functions for assign() 

  template <class _Integer>
  void _M_assign_dispatch(_Integer __n, _Integer __val, __true_type)
    { _M_fill_assign((size_type) __n, (_Tp) __val); }

  template <class _InputIterator>
  void _M_assign_dispatch(_InputIterator __first, _InputIterator __last,
                          __false_type) {
    _M_assign_aux(__first, __last, __ITERATOR_CATEGORY(__first));
  }

  template <class _InputIterator>
  void _M_assign_aux(_InputIterator __first, _InputIterator __last,
                     input_iterator_tag);

  template <class _ForwardIterator>
  void _M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
                     forward_iterator_tag) {
    size_type __len = 0;
	// 计算大小
    distance(__first, __last, __len);
	// 比现在的大小大
    if (__len > size()) {
      _ForwardIterator __mid = __first;
	  // 找到拷贝结束的迭代器
      advance(__mid, size());
	  // 拷贝过去
      copy(__first, __mid, begin());
	  // 插入剩下的
      insert(end(), __mid, __last);
    }
    else
		// 拷贝过去后删除多余的
      erase(copy(__first, __last, begin()), end());
  }

#endif /* __STL_MEMBER_TEMPLATES */

public:                         // push_* and pop_*
  
  void push_back(const value_type& __t) {
	  // 如果当前内存节点后部还有空位
    if (_M_finish._M_cur != _M_finish._M_last - 1) {
		// 构造
      construct(_M_finish._M_cur, __t);
	  // 迭代器后移
      ++_M_finish._M_cur;
    }
    else
      _M_push_back_aux(__t);
  }

  void push_back() {
	  // 如果当前内存节点后部还有空位
    if (_M_finish._M_cur != _M_finish._M_last - 1) {
		// 构造
      construct(_M_finish._M_cur);
	  // 迭代器后移
      ++_M_finish._M_cur;
    }
    else
      _M_push_back_aux();
  }

  void push_front(const value_type& __t) {
	  // 如果当前内存节点前部还有空位
    if (_M_start._M_cur != _M_start._M_first) {
      construct(_M_start._M_cur - 1, __t);
      --_M_start._M_cur;
    }
    else
      _M_push_front_aux(__t);
  }

  void push_front() {
	  // 如果当前内存节点前部还有空位
    if (_M_start._M_cur != _M_start._M_first) {
      construct(_M_start._M_cur - 1);
      --_M_start._M_cur;
    }
    else
      _M_push_front_aux();
  }

  // 
  void pop_back() {
	  // 最后面的内存节点还有其他元素
    if (_M_finish._M_cur != _M_finish._M_first) {
      --_M_finish._M_cur;
	  // 析构
      destroy(_M_finish._M_cur);
    }
    else
      _M_pop_back_aux();
  }

  void pop_front() {
	  // 最前面的内存节点还有其他元素
    if (_M_start._M_cur != _M_start._M_last - 1) {
		// 析构
      destroy(_M_start._M_cur);
      ++_M_start._M_cur;
    }
    else 
      _M_pop_front_aux();
  }

public:                         // Insert

  iterator insert(iterator position, const value_type& __x) {
	  // 如果最前面，那就直接调用push_front
    if (position._M_cur == _M_start._M_cur) {
      push_front(__x);
	  // 插入的位置应该就是_M_start
      return _M_start;
    }
	// 如果是最后面，那就直接调用push_back
    else if (position._M_cur == _M_finish._M_cur) {
      push_back(__x);
	  // 插入的位置应该是_M_finish的前一个
      iterator __tmp = _M_finish;
      --__tmp;
      return __tmp;
    }
    else {
      return _M_insert_aux(position, __x);
    }
  }
  // 在指定位置插入一个默认值
  iterator insert(iterator __position)
    { return insert(__position, value_type()); }
  // 在指定位置插入__n个__x
  void insert(iterator __pos, size_type __n, const value_type& __x)
    { _M_fill_insert(__pos, __n, __x); }

  void _M_fill_insert(iterator __pos, size_type __n, const value_type& __x); 

#ifdef __STL_MEMBER_TEMPLATES  

  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InputIterator>
  void insert(iterator __pos, _InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_insert_dispatch(__pos, __first, __last, _Integral());
  }
  // _InputIterator的整型的特化版本 
  template <class _Integer>
  void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __x,
                          __true_type) {
    _M_fill_insert(__pos, (size_type) __n, (value_type) __x);
  }

  template <class _InputIterator>
  void _M_insert_dispatch(iterator __pos,
                          _InputIterator __first, _InputIterator __last,
                          __false_type) {
    insert(__pos, __first, __last, __ITERATOR_CATEGORY(__first));
  }

#else /* __STL_MEMBER_TEMPLATES */

  void insert(iterator __pos,
              const value_type* __first, const value_type* __last);
  void insert(iterator __pos,
              const_iterator __first, const_iterator __last);

#endif /* __STL_MEMBER_TEMPLATES */

  void resize(size_type __new_size, const value_type& __x) {
    const size_type __len = size();
	// 新的大小比原来的小，删除多出来的
    if (__new_size < __len) 
      erase(_M_start + __new_size, _M_finish);
    else
		// 新的大小比较大，在后面插入多出来的
      insert(_M_finish, __new_size - __len, __x);
  }

  void resize(size_type new_size) { resize(new_size, value_type()); }

public:                         // Erase
  iterator erase(iterator __pos) {
    iterator __next = __pos;
    ++__next;
    difference_type __index = __pos - _M_start;
	// 如果删除的位置在前半部分
    if (size_type(__index) < (this->size() >> 1)) {
		// 那就将Pos前面的往后挪一个位置
      copy_backward(_M_start, __pos, __next);
	  // 把最前面的一个弹出来
      pop_front();
    }
	// 如果删除的位置在后半部分
    else {
		// 拷贝Pos后面的部分往前挪一个位置
      copy(__next, _M_finish, __pos);
	  // 将最后的一个弹出来
      pop_back();
    }
    return _M_start + __index;
  }

  iterator erase(iterator __first, iterator __last);
  void clear(); 

protected:                        // Internal construction/destruction

  void _M_fill_initialize(const value_type& __value);

#ifdef __STL_MEMBER_TEMPLATES  

  template <class _InputIterator>
  void _M_range_initialize(_InputIterator __first, _InputIterator __last,
                        input_iterator_tag);

  template <class _ForwardIterator>
  void _M_range_initialize(_ForwardIterator __first, _ForwardIterator __last,
                        forward_iterator_tag);

#endif /* __STL_MEMBER_TEMPLATES */

protected:                        // Internal push_* and pop_*

  void _M_push_back_aux(const value_type&);
  void _M_push_back_aux();
  void _M_push_front_aux(const value_type&);
  void _M_push_front_aux();
  void _M_pop_back_aux();
  void _M_pop_front_aux();

protected:                        // Internal insert functions

#ifdef __STL_MEMBER_TEMPLATES  

  template <class _InputIterator>
  void insert(iterator __pos, _InputIterator __first, _InputIterator __last,
              input_iterator_tag);

  template <class _ForwardIterator>
  void insert(iterator __pos,
              _ForwardIterator __first, _ForwardIterator __last,
              forward_iterator_tag);

#endif /* __STL_MEMBER_TEMPLATES */

  iterator _M_insert_aux(iterator __pos, const value_type& __x);
  iterator _M_insert_aux(iterator __pos);
  void _M_insert_aux(iterator __pos, size_type __n, const value_type& __x);

#ifdef __STL_MEMBER_TEMPLATES  

  template <class _ForwardIterator>
  void _M_insert_aux(iterator __pos, 
                     _ForwardIterator __first, _ForwardIterator __last,
                     size_type __n);

#else /* __STL_MEMBER_TEMPLATES */
  
  void _M_insert_aux(iterator __pos,
                     const value_type* __first, const value_type* __last,
                     size_type __n);

  void _M_insert_aux(iterator __pos, 
                     const_iterator __first, const_iterator __last,
                     size_type __n);
 
#endif /* __STL_MEMBER_TEMPLATES */
  // 在前面reserve __n个元素
  iterator _M_reserve_elements_at_front(size_type __n) {
    size_type __vacancies = _M_start._M_cur - _M_start._M_first;
    if (__n > __vacancies) 
      _M_new_elements_at_front(__n - __vacancies);
    return _M_start - difference_type(__n);
  }
  // 在后边reserve __n个元素
  iterator _M_reserve_elements_at_back(size_type __n) {
    size_type __vacancies = (_M_finish._M_last - _M_finish._M_cur) - 1;
    if (__n > __vacancies)
      _M_new_elements_at_back(__n - __vacancies);
    return _M_finish + difference_type(__n);
  }

  void _M_new_elements_at_front(size_type __new_elements);
  void _M_new_elements_at_back(size_type __new_elements);

protected:                      // Allocation of _M_map and nodes

  // Makes sure the _M_map has space for new nodes.  Does not actually
  //  add the nodes.  Can invalidate _M_map pointers.  (And consequently, 
  //  deque iterators.)
  // 确保_M_map有空间放下新的nodes,不需要真的加节点，会导致_M_map指针失效

	// 如果空间不够，重新分配map，并且新加的放在前面
  void _M_reserve_map_at_back (size_type __nodes_to_add = 1) {
    if (__nodes_to_add + 1 > _M_map_size - (_M_finish._M_node - _M_map))
      _M_reallocate_map(__nodes_to_add, false);
  }
  // 如果空间不够，重新分配map，并且新加的放在后面
  void _M_reserve_map_at_front (size_type __nodes_to_add = 1) {
    if (__nodes_to_add > size_type(_M_start._M_node - _M_map))
      _M_reallocate_map(__nodes_to_add, true);
  }

  void _M_reallocate_map(size_type __nodes_to_add, bool __add_at_front);
};

// Non-inline member functions

#ifdef __STL_MEMBER_TEMPLATES

template <class _Tp, class _Alloc>
template <class _InputIter>
void deque<_Tp, _Alloc>
  ::_M_assign_aux(_InputIter __first, _InputIter __last, input_iterator_tag)
{
  iterator __cur = begin();
  // 
  for ( ; __first != __last && __cur != end(); ++__cur, ++__first)
    *__cur = *__first;
  // 如果赋值的元素少，将后面的都删除掉
  if (__first == __last)
    erase(__cur, end());
  else
	  // 如果赋值的元素多，将剩下的插入在后面
    insert(end(), __first, __last);
}

#endif /* __STL_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc>
void deque<_Tp, _Alloc>::_M_fill_insert(iterator __pos,
                                        size_type __n, const value_type& __x)
{
	// 如果插入在最前面
  if (__pos._M_cur == _M_start._M_cur) {
	  // 在前面预留出__n个元素的位置
    iterator __new_start = _M_reserve_elements_at_front(__n);
    __STL_TRY {
      uninitialized_fill(__new_start, _M_start, __x);
      _M_start = __new_start;
    }
    __STL_UNWIND(_M_destroy_nodes(__new_start._M_node, _M_start._M_node));
  }
  // 如果插入在最后面
  else if (__pos._M_cur == _M_finish._M_cur) {
	   // 在后面预留出__n个元素的位置
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    __STL_TRY {
      uninitialized_fill(_M_finish, __new_finish, __x);
      _M_finish = __new_finish;
    }
    __STL_UNWIND(_M_destroy_nodes(_M_finish._M_node + 1, 
                                  __new_finish._M_node + 1));    
  }
  // 插入中间
  else 
    _M_insert_aux(__pos, __n, __x);
}

#ifndef __STL_MEMBER_TEMPLATES  

template <class _Tp, class _Alloc>
void deque<_Tp, _Alloc>::insert(iterator __pos,
                                const value_type* __first,
                                const value_type* __last) {
  size_type __n = __last - __first;
  // 在最前面插入
  if (__pos._M_cur == _M_start._M_cur) {
	  // 在最前面预留__n的空间
    iterator __new_start = _M_reserve_elements_at_front(__n);
    __STL_TRY {
		// 将值拷进去
      uninitialized_copy(__first, __last, __new_start);
      _M_start = __new_start;
    }
    __STL_UNWIND(_M_destroy_nodes(__new_start._M_node, _M_start._M_node));
  }
  // 在最后面插入
  else if (__pos._M_cur == _M_finish._M_cur) {
	  // 在最后面预留__n的空间
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    __STL_TRY {
		// 将值拷进去
      uninitialized_copy(__first, __last, _M_finish);
      _M_finish = __new_finish;
    }
    __STL_UNWIND(_M_destroy_nodes(_M_finish._M_node + 1, 
                                  __new_finish._M_node + 1));
  }
  // 中间插入
  else
    _M_insert_aux(__pos, __first, __last, __n);
}

template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::insert(iterator __pos,
                               const_iterator __first, const_iterator __last)
{
  size_type __n = __last - __first;
  // 在最前面插入
  if (__pos._M_cur == _M_start._M_cur) {
	   // 在最前面预留__n的空间
    iterator __new_start = _M_reserve_elements_at_front(__n);
    __STL_TRY {
		// 将值拷进去
      uninitialized_copy(__first, __last, __new_start);
      _M_start = __new_start;
    }
    __STL_UNWIND(_M_destroy_nodes(__new_start._M_node, _M_start._M_node));
  }
  // 在最后面插入
  else if (__pos._M_cur == _M_finish._M_cur) {
	  // 在最后面预留__n的空间
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    __STL_TRY {
		// 将值拷进去
      uninitialized_copy(__first, __last, _M_finish);
      _M_finish = __new_finish;
    }
    __STL_UNWIND(_M_destroy_nodes(_M_finish._M_node + 1, 
                 __new_finish._M_node + 1));
  }
  // 中间插入
  else
    _M_insert_aux(__pos, __first, __last, __n);
}

#endif /* __STL_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc>
typename deque<_Tp,_Alloc>::iterator 
deque<_Tp,_Alloc>::erase(iterator __first, iterator __last)
{
	// 如果是清空整个deque，直接调用clear函数
  if (__first == _M_start && __last == _M_finish) {
    clear();
    return _M_finish;
  }
  else {
    difference_type __n = __last - __first;
    difference_type __elems_before = __first - _M_start;
	// 删除的大部分在前半部分
    if (__elems_before < difference_type((this->size() - __n) / 2)) {
		// 将元素往后移
      copy_backward(_M_start, __first, __last);
      iterator __new_start = _M_start + __n;
	  // 析构
      destroy(_M_start, __new_start);
	  // 释放内存
      _M_destroy_nodes(__new_start._M_node, _M_start._M_node);
      _M_start = __new_start;
    }
	// 删除的大部分在后半部分
    else {
		// 将元素前移
      copy(__last, _M_finish, __first);
      iterator __new_finish = _M_finish - __n;
	  // 析构
      destroy(__new_finish, _M_finish);
	  // 释放内存
      _M_destroy_nodes(__new_finish._M_node + 1, _M_finish._M_node + 1);
      _M_finish = __new_finish;
    }
    return _M_start + __elems_before;
  }
}

template <class _Tp, class _Alloc> 
void deque<_Tp,_Alloc>::clear()
{
	// 遍历每一个内存节点，注意:_M_start._M_node + 1
  for (_Map_pointer __node = _M_start._M_node + 1;
       __node < _M_finish._M_node;
       ++__node) {
	  // 析构每一个内存节点里面的元素
    destroy(*__node, *__node + _S_buffer_size());
    _M_deallocate_node(*__node);
  }
  // 如果不在一个内存节点，析构最前面和最后的内存节点的元素
  if (_M_start._M_node != _M_finish._M_node) {
    destroy(_M_start._M_cur, _M_start._M_last);
    destroy(_M_finish._M_first, _M_finish._M_cur);
	// 释放最后的内存节点
    _M_deallocate_node(_M_finish._M_first);
  }
  else
	  // 如果是同一个内存节点，析构就好，不释放内存节点
    destroy(_M_start._M_cur, _M_finish._M_cur);
  // 保留一个内存节点，可以参看一下_M_initialize_map传入参数0时，也会分配一个内存节点
  _M_finish = _M_start;
}

// Precondition: _M_start and _M_finish have already been initialized,
// but none of the deque's elements have yet been constructed.
template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_fill_initialize(const value_type& __value) {
  _Map_pointer __cur;
  __STL_TRY {
	  // 遍历内存节点，fill元素
    for (__cur = _M_start._M_node; __cur < _M_finish._M_node; ++__cur)
      uninitialized_fill(*__cur, *__cur + _S_buffer_size(), __value);
	// 构造最后一个内存节点，这个节点元素不一定满，单独构造
    uninitialized_fill(_M_finish._M_first, _M_finish._M_cur, __value);
  }
  __STL_UNWIND(destroy(_M_start, iterator(*__cur, __cur)));
}

#ifdef __STL_MEMBER_TEMPLATES  
// _InputIterator的版本
template <class _Tp, class _Alloc> template <class _InputIterator>
void deque<_Tp,_Alloc>::_M_range_initialize(_InputIterator __first,
                                            _InputIterator __last,
                                            input_iterator_tag)
{
	// 初始化内存
  _M_initialize_map(0);
  __STL_TRY {
	  // 挨个压入后面
    for ( ; __first != __last; ++__first)
      push_back(*__first);
  }
  __STL_UNWIND(clear());
}

template <class _Tp, class _Alloc> template <class _ForwardIterator>
void deque<_Tp,_Alloc>::_M_range_initialize(_ForwardIterator __first,
                                            _ForwardIterator __last,
                                            forward_iterator_tag)
{
	// 计算元素数量
  size_type __n = 0;
  distance(__first, __last, __n);
  // 初始化内存
  _M_initialize_map(__n);

  _Map_pointer __cur_node;
  __STL_TRY {
	  // 遍历内存节点，进行赋值
    for (__cur_node = _M_start._M_node; 
         __cur_node < _M_finish._M_node; 
         ++__cur_node) {
      _ForwardIterator __mid = __first;
	  // 找到拷贝到一个内存节点结束的迭代器
      advance(__mid, _S_buffer_size());
	  // 赋值
      uninitialized_copy(__first, __mid, *__cur_node);
      __first = __mid;
    }
	// 拷贝最后多出来的那一部分
    uninitialized_copy(__first, __last, _M_finish._M_first);
  }
  __STL_UNWIND(destroy(_M_start, iterator(*__cur_node, __cur_node)));
}

#endif /* __STL_MEMBER_TEMPLATES */

// Called only if _M_finish._M_cur == _M_finish._M_last - 1.
// 只有在M_finish._M_cur == _M_finish._M_last - 1才会调用
template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_push_back_aux(const value_type& __t)
{
  value_type __t_copy = __t;
  // 在后面预留map
  _M_reserve_map_at_back();
  // 分配内存节点
  *(_M_finish._M_node + 1) = _M_allocate_node();
  __STL_TRY {
	  // 拷贝构造
    construct(_M_finish._M_cur, __t_copy);
	// 设置节点和当前的位置
    _M_finish._M_set_node(_M_finish._M_node + 1);
    _M_finish._M_cur = _M_finish._M_first;
  }
  __STL_UNWIND(_M_deallocate_node(*(_M_finish._M_node + 1)));
}

// Called only if _M_finish._M_cur == _M_finish._M_last - 1.
// 只有在_M_finish._M_cur == _M_finish._M_last - 1才会调用
template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_push_back_aux()
{
	// 在后面预留map
  _M_reserve_map_at_back();
  // 分配内存节点
  *(_M_finish._M_node + 1) = _M_allocate_node();
  __STL_TRY {
	  // 构造
    construct(_M_finish._M_cur);
	// 设置节点和当前的位置
    _M_finish._M_set_node(_M_finish._M_node + 1);
    _M_finish._M_cur = _M_finish._M_first;
  }
  __STL_UNWIND(_M_deallocate_node(*(_M_finish._M_node + 1)));
}

// Called only if _M_start._M_cur == _M_start._M_first.
// 只有在_M_start._M_cur == _M_start._M_first才会调用
template <class _Tp, class _Alloc>
void  deque<_Tp,_Alloc>::_M_push_front_aux(const value_type& __t)
{
  value_type __t_copy = __t;
  // 在前面预留map
  _M_reserve_map_at_front();
  // 分配内存节点
  *(_M_start._M_node - 1) = _M_allocate_node();
  __STL_TRY {
	  // 设置节点和当前的位置
    _M_start._M_set_node(_M_start._M_node - 1);
    _M_start._M_cur = _M_start._M_last - 1;
	// 构造
    construct(_M_start._M_cur, __t_copy);
  }
  __STL_UNWIND((++_M_start, _M_deallocate_node(*(_M_start._M_node - 1))));
} 

// Called only if _M_start._M_cur == _M_start._M_first.
// 只有在_M_start._M_cur == _M_start._M_first才会调用
template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_push_front_aux()
{
	// 在前面预留map
  _M_reserve_map_at_front();
	// 分配内存节点
  *(_M_start._M_node - 1) = _M_allocate_node();
  __STL_TRY {
	 // 设置节点和当前的位置
    _M_start._M_set_node(_M_start._M_node - 1);
    _M_start._M_cur = _M_start._M_last - 1;
	// 构造
    construct(_M_start._M_cur);
  }
  __STL_UNWIND((++_M_start, _M_deallocate_node(*(_M_start._M_node - 1))));
} 

// Called only if _M_finish._M_cur == _M_finish._M_first.
// 只有在_M_finish._M_cur == _M_finish._M_first才会调用
template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_pop_back_aux()
{
	// 释放内存节点
  _M_deallocate_node(_M_finish._M_first);
   // 设置节点和当前的位置
  _M_finish._M_set_node(_M_finish._M_node - 1);
  _M_finish._M_cur = _M_finish._M_last - 1;
  // 析构
  destroy(_M_finish._M_cur);
}

// Called only if _M_start._M_cur == _M_start._M_last - 1.  Note that 
// if the deque has at least one element (a precondition for this member 
// function), and if _M_start._M_cur == _M_start._M_last, then the deque 
// must have at least two nodes.
// 只有在_M_start._M_cur == _M_start._M_last - 1调用
// 注意：如果deque至少有一个元素（这个成员函数的前提条件），
// 如果_M_start._M_cur == _M_start._M_last，则该deque至少有两个nodes
template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_pop_front_aux()
{
	// 析构
  destroy(_M_start._M_cur);
  // 删除内存节点
  _M_deallocate_node(_M_start._M_first);
  // 设置节点和当前的位置
  _M_start._M_set_node(_M_start._M_node + 1);
  _M_start._M_cur = _M_start._M_first;
}      

#ifdef __STL_MEMBER_TEMPLATES  

template <class _Tp, class _Alloc> template <class _InputIterator>
void deque<_Tp,_Alloc>::insert(iterator __pos,
                               _InputIterator __first, _InputIterator __last,
                               input_iterator_tag)
{
  copy(__first, __last, inserter(*this, __pos));
}

template <class _Tp, class _Alloc> template <class _ForwardIterator>
void
deque<_Tp,_Alloc>::insert(iterator __pos,
                          _ForwardIterator __first, _ForwardIterator __last,
                          forward_iterator_tag) {
  size_type __n = 0;
  distance(__first, __last, __n);
  // 插入在最前面
  if (__pos._M_cur == _M_start._M_cur) {
	  // 在最前面预留空间
    iterator __new_start = _M_reserve_elements_at_front(__n);
    __STL_TRY {
		// 拷贝进去
      uninitialized_copy(__first, __last, __new_start);
	  // 开始节点前移
      _M_start = __new_start;
    }
    __STL_UNWIND(_M_destroy_nodes(__new_start._M_node, _M_start._M_node));
  }
  // 插入在最后面
  else if (__pos._M_cur == _M_finish._M_cur) {
	  // 在后面预留空间
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    __STL_TRY {
		// 拷贝进去
      uninitialized_copy(__first, __last, _M_finish);
	  // 结束的节点后移
      _M_finish = __new_finish;
    }
    __STL_UNWIND(_M_destroy_nodes(_M_finish._M_node + 1, 
                                  __new_finish._M_node + 1));
  }
  // 插入在中间
  else
    _M_insert_aux(__pos, __first, __last, __n);
}

#endif /* __STL_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc>
typename deque<_Tp, _Alloc>::iterator
deque<_Tp,_Alloc>::_M_insert_aux(iterator __pos, const value_type& __x)
{
  difference_type __index = __pos - _M_start;
  value_type __x_copy = __x;
  // 插入的位置在前半部分
  if (size_type(__index) < this->size() / 2) {
	  // 在最前面构造一个和首节点一样的元素
    push_front(front());
    iterator __front1 = _M_start;
    ++__front1;
    iterator __front2 = __front1;
    ++__front2;
    __pos = _M_start + __index;
    iterator __pos1 = __pos;
    ++__pos1;
	// 原来的元素往前移一个位置
    copy(__front2, __pos1, __front1);
  }
  // 插入的位置在后半部分
  else {
	// 在最后面构造一个和尾节点一样的元素
    push_back(back());
    iterator __back1 = _M_finish;
    --__back1;
    iterator __back2 = __back1;
    --__back2;
    __pos = _M_start + __index;
	// 原来的元素往后移一个位置
    copy_backward(__pos, __back2, __back1);
  }
  // 空出来的位置上赋值
  *__pos = __x_copy;
  return __pos;
}

template <class _Tp, class _Alloc>
typename deque<_Tp,_Alloc>::iterator 
deque<_Tp,_Alloc>::_M_insert_aux(iterator __pos)
{
  difference_type __index = __pos - _M_start;
  // 插入的位置在前半部分
  if (__index < size() / 2) {
	  // 在最前面构造一个和首节点一样的元素
    push_front(front());
    iterator __front1 = _M_start;
    ++__front1;
    iterator __front2 = __front1;
    ++__front2;
    __pos = _M_start + __index;
    iterator __pos1 = __pos;
    ++__pos1;
	// 原来的元素往前移一个位置
    copy(__front2, __pos1, __front1);
  }
  // 插入的位置在后半部分
  else {
	// 在最后面构造一个和尾节点一样的元素
    push_back(back());
    iterator __back1 = _M_finish;
    --__back1;
    iterator __back2 = __back1;
    --__back2;
    __pos = _M_start + __index;
	// 原来的元素往后移一个位置
    copy_backward(__pos, __back2, __back1);
  }
  // 空出来的位置上赋值
  *__pos = value_type();
  return __pos;
}

// 在__pos的前面插入__n个__x
template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_insert_aux(iterator __pos,
                                      size_type __n,
                                      const value_type& __x)
{
  const difference_type __elems_before = __pos - _M_start;
  size_type __length = this->size();
  value_type __x_copy = __x;
  // 插入位置在前半部分
  if (__elems_before < difference_type(__length / 2)) {
	  // 在前面预留出了__n个元素
    iterator __new_start = _M_reserve_elements_at_front(__n);
    iterator __old_start = _M_start;
    __pos = _M_start + __elems_before;
    __STL_TRY {
		// 如果第一个元素到插入位置的数目大于等于__n
      if (__elems_before >= difference_type(__n)) {
		  // 原来的最前面的__n个元素移动到未初始化的内存上面去，使用uninitialized_copy（没有初始化的拷贝）
        iterator __start_n = _M_start + difference_type(__n);
        uninitialized_copy(_M_start, __start_n, __new_start);
        _M_start = __new_start;
		// __elems_before - __n的那部分调用copy拷贝
        copy(__start_n, __pos, __old_start);
		// 后面的调用fill填充__n个__x
        fill(__pos - difference_type(__n), __pos, __x_copy);
      }
	  // 如果第一个元素到插入位置的数目小于__n
      else {
		  // 整个原来的元素放在前面没有初始化的内存上面去，
        __uninitialized_copy_fill(_M_start, __pos, __new_start, 
                                  _M_start, __x_copy);
        _M_start = __new_start;
		// 调用fill填充__n个__x
        fill(__old_start, __pos, __x_copy);
      }
    }
    __STL_UNWIND(_M_destroy_nodes(__new_start._M_node, _M_start._M_node));
  }
  // 插入位置在后半部分
  else {
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    iterator __old_finish = _M_finish;
    const difference_type __elems_after = 
      difference_type(__length) - __elems_before;
    __pos = _M_finish - __elems_after;
    __STL_TRY {
		// 如果插入位置到第后一个元素的数目大于__n
      if (__elems_after > difference_type(__n)) {
        iterator __finish_n = _M_finish - difference_type(__n);
		// 原来的最后面的__n个元素移动到未初始化的内存上面去，使用uninitialized_copy（没有初始化的拷贝）
        uninitialized_copy(__finish_n, _M_finish, _M_finish);
        _M_finish = __new_finish;
		// __elems_before - __n的那部分调用copy拷贝
        copy_backward(__pos, __finish_n, __old_finish);
		// 调用fill填充__n个__x
        fill(__pos, __pos + difference_type(__n), __x_copy);
      }
      else {
		   // 整个原来的元素放在后面没有初始化的内存上面去，
        __uninitialized_fill_copy(_M_finish, __pos + difference_type(__n),
                                  __x_copy, __pos, _M_finish);
        _M_finish = __new_finish;
		// 调用fill填充__n个__x
        fill(__pos, __old_finish, __x_copy);
      }
    }
    __STL_UNWIND(_M_destroy_nodes(_M_finish._M_node + 1, 
                                  __new_finish._M_node + 1));
  }
}

#ifdef __STL_MEMBER_TEMPLATES  

template <class _Tp, class _Alloc> template <class _ForwardIterator>
void deque<_Tp,_Alloc>::_M_insert_aux(iterator __pos,
                                      _ForwardIterator __first,
                                      _ForwardIterator __last,
                                      size_type __n)
{
  const difference_type __elemsbefore = __pos - _M_start;
  size_type __length = size();
  // 插入位置在前半部分
  if (__elemsbefore < __length / 2) {
	  // 在前面预留出了__n个元素
    iterator __new_start = _M_reserve_elements_at_front(__n);
    iterator __old_start = _M_start;
    __pos = _M_start + __elemsbefore;
    __STL_TRY {
		// 如果第一个元素到插入位置的数目大于等于__n
      if (__elemsbefore >= difference_type(__n)) {
        iterator __start_n = _M_start + difference_type(__n); 
		// 原来的最前面的__n个元素移动到未初始化的内存上面去，使用uninitialized_copy（没有初始化的拷贝）
        uninitialized_copy(_M_start, __start_n, __new_start);
        _M_start = __new_start;
		// __elems_before - __n的那部分调用copy拷贝
        copy(__start_n, __pos, __old_start);
		// 将值拷贝进去
        copy(__first, __last, __pos - difference_type(__n));
      }
      else {
        _ForwardIterator __mid = __first;
        advance(__mid, difference_type(__n) - __elemsbefore);
		// 将原来Pos前面的元素移动到未初始化内存上面去，然后把输入参数中__n - __elemsbefore也拷贝到未初始化内存上面去
        __uninitialized_copy_copy(_M_start, __pos, __first, __mid,
                                  __new_start);
        _M_start = __new_start;
		// 拷贝剩余的元素
        copy(__mid, __last, __old_start);
      }
    }
    __STL_UNWIND(_M_destroy_nodes(__new_start._M_node, _M_start._M_node));
  }
  // 插入位置在后半部分
  else {
	  // 在后面预留出了__n个元素
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    iterator __old_finish = _M_finish;
    const difference_type __elemsafter = 
      difference_type(__length) - __elemsbefore;
    __pos = _M_finish - __elemsafter;
    __STL_TRY {
		// 如果插入位置到第后一个元素的数目大于__n
      if (__elemsafter > difference_type(__n)) {
        iterator __finish_n = _M_finish - difference_type(__n);
		// 原来的最后面的__n个元素移动到未初始化的内存上面去，使用uninitialized_copy（没有初始化的拷贝）
        uninitialized_copy(__finish_n, _M_finish, _M_finish);
        _M_finish = __new_finish;
		// __elems_before - __n的那部分调用copy_backward拷贝
        copy_backward(__pos, __finish_n, __old_finish);
		// 将值拷贝进去
        copy(__first, __last, __pos);
      }
      else {
        _ForwardIterator __mid = __first;
        advance(__mid, __elemsafter);
		// 将把输入参数[__first, __last)中最后的__n - __elemsbefore个元素拷贝到未初始化内存上面去，然后将原来Pos后面的元素移动到未初始化内存上面去，
        __uninitialized_copy_copy(__mid, __last, __pos, _M_finish, _M_finish);
        _M_finish = __new_finish;
		// 拷贝剩下的元素
        copy(__first, __mid, __pos);
      }
    }
    __STL_UNWIND(_M_destroy_nodes(_M_finish._M_node + 1, 
                                  __new_finish._M_node + 1));
  }
}

#else /* __STL_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_insert_aux(iterator __pos,
                                      const value_type* __first,
                                      const value_type* __last,
                                      size_type __n)
{
  const difference_type __elemsbefore = __pos - _M_start;
  size_type __length = size();
  // 插入位置在前半部分
  if (__elemsbefore < __length / 2) {
	// 在前面预留出了__n个元素
    iterator __new_start = _M_reserve_elements_at_front(__n);
    iterator __old_start = _M_start;
    __pos = _M_start + __elemsbefore;
    __STL_TRY {
		// 如果第一个元素到插入位置的数目大于等于__n
      if (__elemsbefore >= difference_type(__n)) {
        iterator __start_n = _M_start + difference_type(__n);
		// 原来的最前面的__n个元素移动到未初始化的内存上面去，使用uninitialized_copy（没有初始化的拷贝）
        uninitialized_copy(_M_start, __start_n, __new_start);
        _M_start = __new_start;
		// __elems_before - __n的那部分调用copy拷贝
        copy(__start_n, __pos, __old_start);
		// 将值拷贝进去
        copy(__first, __last, __pos - difference_type(__n));
      }
      else {
        const value_type* __mid = 
          __first + (difference_type(__n) - __elemsbefore);
		// 将原来Pos前面的元素移动到未初始化内存上面去，然后把输入参数中__n - __elemsbefore也拷贝到未初始化内存上面去
        __uninitialized_copy_copy(_M_start, __pos, __first, __mid,
                                  __new_start);
        _M_start = __new_start;
		// 拷贝剩余的元素
        copy(__mid, __last, __old_start);
      }
    }
    __STL_UNWIND(_M_destroy_nodes(__new_start._M_node, _M_start._M_node));
  }
  else {
	// 在后面预留出了__n个元素
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    iterator __old_finish = _M_finish;
    const difference_type __elemsafter = 
      difference_type(__length) - __elemsbefore;
    __pos = _M_finish - __elemsafter;
    __STL_TRY {
		// 如果插入位置到第后一个元素的数目大于__n
      if (__elemsafter > difference_type(__n)) {
        iterator __finish_n = _M_finish - difference_type(__n);
		// 原来的最后面的__n个元素移动到未初始化的内存上面去，使用uninitialized_copy（没有初始化的拷贝）
        uninitialized_copy(__finish_n, _M_finish, _M_finish);
        _M_finish = __new_finish;
		// __elems_before - __n的那部分调用copy_backward拷贝
        copy_backward(__pos, __finish_n, __old_finish);
		// 将值拷贝进去
        copy(__first, __last, __pos);
      }
      else {
        const value_type* __mid = __first + __elemsafter;
		// 将把输入参数[__first, __last)中最后的__n - __elemsbefore个元素拷贝到未初始化内存上面去，然后将原来Pos后面的元素移动到未初始化内存上面去，
        __uninitialized_copy_copy(__mid, __last, __pos, _M_finish, _M_finish);
        _M_finish = __new_finish;
		// 拷贝剩下的元素
        copy(__first, __mid, __pos);
      }
    }
    __STL_UNWIND(_M_destroy_nodes(_M_finish._M_node + 1, 
                                  __new_finish._M_node + 1));
  }
}

template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_insert_aux(iterator __pos,
                                      const_iterator __first,
                                      const_iterator __last,
                                      size_type __n)
{
  const difference_type __elemsbefore = __pos - _M_start;
  size_type __length = size();
    // 插入位置在前半部分
  if (__elemsbefore < __length / 2) {
	// 在前面预留出了__n个元素
    iterator __new_start = _M_reserve_elements_at_front(__n);
    iterator __old_start = _M_start;
    __pos = _M_start + __elemsbefore;
    __STL_TRY {
		// 如果插入位置到第一个元素的数目大于等于__n
      if (__elemsbefore >= __n) {
        iterator __start_n = _M_start + __n;
		// 原来的最前面的__n个元素移动到未初始化的内存上面去，使用uninitialized_copy（没有初始化的拷贝）
        uninitialized_copy(_M_start, __start_n, __new_start);
        _M_start = __new_start;
		// __elems_before - __n的那部分调用copy拷贝
        copy(__start_n, __pos, __old_start);
		// 将值拷贝进去
        copy(__first, __last, __pos - difference_type(__n));
      }
      else {
        const_iterator __mid = __first + (__n - __elemsbefore);
		// 将原来Pos前面的元素移动到未初始化内存上面去，然后把输入参数中__n - __elemsbefore也拷贝到未初始化内存上面去
        __uninitialized_copy_copy(_M_start, __pos, __first, __mid,
                                  __new_start);
        _M_start = __new_start;
		// 拷贝剩余的元素
        copy(__mid, __last, __old_start);
      }
    }
    __STL_UNWIND(_M_destroy_nodes(__new_start._M_node, _M_start._M_node));
  }
  else {
	  // 在后面预留出了__n个元素
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    iterator __old_finish = _M_finish;
    const difference_type __elemsafter = __length - __elemsbefore;
    __pos = _M_finish - __elemsafter;
    __STL_TRY {
		// 如果插入位置到第后一个元素的数目大于__n
      if (__elemsafter > __n) {
        iterator __finish_n = _M_finish - difference_type(__n);
		// 原来的最后面的__n个元素移动到未初始化的内存上面去，使用uninitialized_copy（没有初始化的拷贝）
        uninitialized_copy(__finish_n, _M_finish, _M_finish);
        _M_finish = __new_finish;
		// __elems_before - __n的那部分调用copy_backward拷贝
        copy_backward(__pos, __finish_n, __old_finish);
		// 将值拷贝进去
        copy(__first, __last, __pos);
      }
      else {
        const_iterator __mid = __first + __elemsafter;
		// 将把输入参数[__first, __last)中最后的__n - __elemsbefore个元素拷贝到未初始化内存上面去，然后将原来Pos后面的元素移动到未初始化内存上面去，
        __uninitialized_copy_copy(__mid, __last, __pos, _M_finish, _M_finish);
        _M_finish = __new_finish;
		// 拷贝剩下的元素
        copy(__first, __mid, __pos);
      }
    }
    __STL_UNWIND(_M_destroy_nodes(_M_finish._M_node + 1, 
                 __new_finish._M_node + 1));
  }
}

#endif /* __STL_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_new_elements_at_front(size_type __new_elems)
{
	// 计算增加的内存节点
  size_type __new_nodes
      = (__new_elems + _S_buffer_size() - 1) / _S_buffer_size();
  // 在前面预留map空间
  _M_reserve_map_at_front(__new_nodes);
  size_type __i;
  __STL_TRY {
	// 分配内存节点
    for (__i = 1; __i <= __new_nodes; ++__i)
      *(_M_start._M_node - __i) = _M_allocate_node();
  }
#       ifdef __STL_USE_EXCEPTIONS
  catch(...) {
    for (size_type __j = 1; __j < __i; ++__j)
      _M_deallocate_node(*(_M_start._M_node - __j));      
    throw;
  }
#       endif /* __STL_USE_EXCEPTIONS */
}

template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_new_elements_at_back(size_type __new_elems)
{
	// 计算增加的内存节点
  size_type __new_nodes
      = (__new_elems + _S_buffer_size() - 1) / _S_buffer_size();
  // 在后面预留map空间
  _M_reserve_map_at_back(__new_nodes);
  size_type __i;
  __STL_TRY {
	// 分配内存节点
    for (__i = 1; __i <= __new_nodes; ++__i)
      *(_M_finish._M_node + __i) = _M_allocate_node();
  }
#       ifdef __STL_USE_EXCEPTIONS
  catch(...) {
    for (size_type __j = 1; __j < __i; ++__j)
      _M_deallocate_node(*(_M_finish._M_node + __j));      
    throw;
  }
#       endif /* __STL_USE_EXCEPTIONS */
}

// 重新分配map,__add_at_front:增加的节点是否在前面
template <class _Tp, class _Alloc>
void deque<_Tp,_Alloc>::_M_reallocate_map(size_type __nodes_to_add,
                                          bool __add_at_front)
{
	// 计算老的节点数
  size_type __old_num_nodes = _M_finish._M_node - _M_start._M_node + 1;
  // 计算新的节点数
  size_type __new_num_nodes = __old_num_nodes + __nodes_to_add;

  _Map_pointer __new_nstart;
  // 还有空间，那就把用到的调整到中间位置
  if (_M_map_size > 2 * __new_num_nodes) {
    __new_nstart = _M_map + (_M_map_size - __new_num_nodes) / 2 
                     + (__add_at_front ? __nodes_to_add : 0);
	// 往前拷贝
    if (__new_nstart < _M_start._M_node)
      copy(_M_start._M_node, _M_finish._M_node + 1, __new_nstart);
	// 往后拷贝
    else
      copy_backward(_M_start._M_node, _M_finish._M_node + 1, 
                    __new_nstart + __old_num_nodes);
  }
  else {
	// 扩展map的策略
    size_type __new_map_size = 
      _M_map_size + max(_M_map_size, __nodes_to_add) + 2;
	// 分配新的map
    _Map_pointer __new_map = _M_allocate_map(__new_map_size);
	// 开始位置调整到中间
    __new_nstart = __new_map + (__new_map_size - __new_num_nodes) / 2
                         + (__add_at_front ? __nodes_to_add : 0);
	// 拷贝到新的内容里面
    copy(_M_start._M_node, _M_finish._M_node + 1, __new_nstart);
	// 释放内存
    _M_deallocate_map(_M_map, _M_map_size);
	// 设置新的map和map size
    _M_map = __new_map;
    _M_map_size = __new_map_size;
  }

  _M_start._M_set_node(__new_nstart);
  _M_finish._M_set_node(__new_nstart + __old_num_nodes - 1);
}


// Nonmember functions.

template <class _Tp, class _Alloc>
inline bool operator==(const deque<_Tp, _Alloc>& __x,
                       const deque<_Tp, _Alloc>& __y) {
  return __x.size() == __y.size() &&
         equal(__x.begin(), __x.end(), __y.begin());
}

template <class _Tp, class _Alloc>
inline bool operator<(const deque<_Tp, _Alloc>& __x,
                      const deque<_Tp, _Alloc>& __y) {
  return lexicographical_compare(__x.begin(), __x.end(), 
                                 __y.begin(), __y.end());
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Tp, class _Alloc>
inline bool operator!=(const deque<_Tp, _Alloc>& __x,
                       const deque<_Tp, _Alloc>& __y) {
  return !(__x == __y);
}

template <class _Tp, class _Alloc>
inline bool operator>(const deque<_Tp, _Alloc>& __x,
                      const deque<_Tp, _Alloc>& __y) {
  return __y < __x;
}

template <class _Tp, class _Alloc>
inline bool operator<=(const deque<_Tp, _Alloc>& __x,
                       const deque<_Tp, _Alloc>& __y) {
  return !(__y < __x);
}
template <class _Tp, class _Alloc>
inline bool operator>=(const deque<_Tp, _Alloc>& __x,
                       const deque<_Tp, _Alloc>& __y) {
  return !(__x < __y);
}

template <class _Tp, class _Alloc>
inline void swap(deque<_Tp,_Alloc>& __x, deque<_Tp,_Alloc>& __y) {
  __x.swap(__y);
}

#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif
          
__STL_END_NAMESPACE 
  
#endif /* __SGI_STL_INTERNAL_DEQUE_H */

// Local Variables:
// mode:C++
// End:
