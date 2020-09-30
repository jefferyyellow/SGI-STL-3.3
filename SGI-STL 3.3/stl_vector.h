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
 * Copyright (c) 1996
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

#ifndef __SGI_STL_INTERNAL_VECTOR_H
#define __SGI_STL_INTERNAL_VECTOR_H

#include <concept_checks.h>

__STL_BEGIN_NAMESPACE 

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif

// The vector base class serves two purposes.  First, its constructor
// and destructor allocate (but don't initialize) storage.  This makes
// exception safety easier.  Second, the base class encapsulates all of
// the differences between SGI-style allocators and standard-conforming
// allocators.
// 这个向量基类有两个作用：
// 第一，他构造和析构分配的内存，这使得异常安全更加容易
// 第二，基类封装了所有SGI风格和符合标准风格的分配器
#ifdef __STL_USE_STD_ALLOCATORS

// Base class for ordinary allocators.
// 普通分配器的基类
template <class _Tp, class _Allocator, bool _IsStatic>
class _Vector_alloc_base {
public:
  typedef typename _Alloc_traits<_Tp, _Allocator>::allocator_type
          allocator_type;
  allocator_type get_allocator() const { return _M_data_allocator; }

  _Vector_alloc_base(const allocator_type& __a)
    : _M_data_allocator(__a), _M_start(0), _M_finish(0), _M_end_of_storage(0) 
  {}
  
protected:
	// 分配器
  allocator_type _M_data_allocator;
  // 
  _Tp* _M_start;
  _Tp* _M_finish;
  _Tp* _M_end_of_storage;

  _Tp* _M_allocate(size_t __n)
    { return _M_data_allocator.allocate(__n); }
  void _M_deallocate(_Tp* __p, size_t __n)
    { if (__p) _M_data_allocator.deallocate(__p, __n); }
};

// Specialization for allocators that have the property that we don't
// actually have to store an allocator object.  
// 具有我们实际上不存储任何分配对象这种属性的存储器的特化版本
// 注意，这里没有声明分配器的类实例，而是一个调用的类分配方式
template <class _Tp, class _Allocator>
class _Vector_alloc_base<_Tp, _Allocator, true> {
public:
  typedef typename _Alloc_traits<_Tp, _Allocator>::allocator_type
          allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Vector_alloc_base(const allocator_type&)
    : _M_start(0), _M_finish(0), _M_end_of_storage(0) 
  {}
  
protected:
  _Tp* _M_start;
  _Tp* _M_finish;
  _Tp* _M_end_of_storage;

  typedef typename _Alloc_traits<_Tp, _Allocator>::_Alloc_type _Alloc_type;
  _Tp* _M_allocate(size_t __n)
    { return _Alloc_type::allocate(__n); }
  void _M_deallocate(_Tp* __p, size_t __n)
    { _Alloc_type::deallocate(__p, __n);}
};

template <class _Tp, class _Alloc>
struct _Vector_base
  : public _Vector_alloc_base<_Tp, _Alloc,
                              _Alloc_traits<_Tp, _Alloc>::_S_instanceless>
{
  typedef _Vector_alloc_base<_Tp, _Alloc, 
                             _Alloc_traits<_Tp, _Alloc>::_S_instanceless>
          _Base;
  typedef typename _Base::allocator_type allocator_type;

  _Vector_base(const allocator_type& __a) : _Base(__a) {}
  _Vector_base(size_t __n, const allocator_type& __a) : _Base(__a) {
    _M_start = _M_allocate(__n);
    _M_finish = _M_start;
    _M_end_of_storage = _M_start + __n;
  }

  ~_Vector_base() { _M_deallocate(_M_start, _M_end_of_storage - _M_start); }
};    

#else /* __STL_USE_STD_ALLOCATORS */

template <class _Tp, class _Alloc> 
class _Vector_base {
public:
  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Vector_base(const _Alloc&)
    : _M_start(0), _M_finish(0), _M_end_of_storage(0) {}
  _Vector_base(size_t __n, const _Alloc&)
    : _M_start(0), _M_finish(0), _M_end_of_storage(0) 
  {
    _M_start = _M_allocate(__n);
    _M_finish = _M_start;
    _M_end_of_storage = _M_start + __n;
  }

  ~_Vector_base() { _M_deallocate(_M_start, _M_end_of_storage - _M_start); }

protected:
  _Tp* _M_start;
  _Tp* _M_finish;
  _Tp* _M_end_of_storage;

  typedef simple_alloc<_Tp, _Alloc> _M_data_allocator;
  _Tp* _M_allocate(size_t __n)
    { return _M_data_allocator::allocate(__n); }
  void _M_deallocate(_Tp* __p, size_t __n) 
    { _M_data_allocator::deallocate(__p, __n); }
};

#endif /* __STL_USE_STD_ALLOCATORS */

template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp) >
class vector : protected _Vector_base<_Tp, _Alloc> 
{
  // requirements:

  __STL_CLASS_REQUIRES(_Tp, _Assignable);

private:
  typedef _Vector_base<_Tp, _Alloc> _Base;
public:
  typedef _Tp value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type* iterator;
  typedef const value_type* const_iterator;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef typename _Base::allocator_type allocator_type;
  allocator_type get_allocator() const { return _Base::get_allocator(); }

#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
  typedef reverse_iterator<const_iterator> const_reverse_iterator;
  typedef reverse_iterator<iterator> reverse_iterator;
#else /* __STL_CLASS_PARTIAL_SPECIALIZATION */
  typedef reverse_iterator<const_iterator, value_type, const_reference, 
                           difference_type>  const_reverse_iterator;
  typedef reverse_iterator<iterator, value_type, reference, difference_type>
          reverse_iterator;
#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

protected:
#ifdef __STL_HAS_NAMESPACES
  using _Base::_M_allocate;
  using _Base::_M_deallocate;
  using _Base::_M_start;
  using _Base::_M_finish;
  using _Base::_M_end_of_storage;
#endif /* __STL_HAS_NAMESPACES */

protected:
  void _M_insert_aux(iterator __position, const _Tp& __x);
  void _M_insert_aux(iterator __position);

public:
  iterator begin() { return _M_start; }
  const_iterator begin() const { return _M_start; }
  iterator end() { return _M_finish; }
  const_iterator end() const { return _M_finish; }

  reverse_iterator rbegin()
    { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const
    { return const_reverse_iterator(end()); }
  reverse_iterator rend()
    { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const
    { return const_reverse_iterator(begin()); }

  size_type size() const
    { return size_type(end() - begin()); }
  size_type max_size() const
    { return size_type(-1) / sizeof(_Tp); }
  size_type capacity() const
    { return size_type(_M_end_of_storage - begin()); }
  bool empty() const
    { return begin() == end(); }
  // 直接用中括号取值
  reference operator[](size_type __n) { return *(begin() + __n); }
  const_reference operator[](size_type __n) const { return *(begin() + __n); }

#ifdef __STL_THROW_RANGE_ERRORS
  // 对当前的__n的范围进行检查，如果超出大小就抛出越界异常，注意重载的[]没有范围检查
  void _M_range_check(size_type __n) const {
    if (__n >= this->size())
      __stl_throw_range_error("vector");
  }

  reference at(size_type __n)
    { _M_range_check(__n); return (*this)[__n]; }
  const_reference at(size_type __n) const
    { _M_range_check(__n); return (*this)[__n]; }
#endif /* __STL_THROW_RANGE_ERRORS */

  explicit vector(const allocator_type& __a = allocator_type())
    : _Base(__a) {}

  vector(size_type __n, const _Tp& __value,
         const allocator_type& __a = allocator_type()) 
    : _Base(__n, __a)
    { _M_finish = uninitialized_fill_n(_M_start, __n, __value); }

  explicit vector(size_type __n)
    : _Base(__n, allocator_type())
    { _M_finish = uninitialized_fill_n(_M_start, __n, _Tp()); }

  vector(const vector<_Tp, _Alloc>& __x) 
    : _Base(__x.size(), __x.get_allocator())
    { _M_finish = uninitialized_copy(__x.begin(), __x.end(), _M_start); }

#ifdef __STL_MEMBER_TEMPLATES
  // Check whether it's an integral type.  If so, it's not an iterator.
  // 检查_InputIterator是否整形，不同的方式不同的处理，因为向量支持整形[n]的方式
  template <class _InputIterator>
  vector(_InputIterator __first, _InputIterator __last,
         const allocator_type& __a = allocator_type()) : _Base(__a) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_initialize_aux(__first, __last, _Integral());
  }
 
  // 如果_InputIterator是整形
  template <class _Integer>
  void _M_initialize_aux(_Integer __n, _Integer __value, __true_type) {
    _M_start = _M_allocate(__n);
    _M_end_of_storage = _M_start + __n; 
    _M_finish = uninitialized_fill_n(_M_start, __n, __value);
  }

  // 不是整形
  template <class _InputIterator>
  void _M_initialize_aux(_InputIterator __first, _InputIterator __last,
                         __false_type) {
    _M_range_initialize(__first, __last, __ITERATOR_CATEGORY(__first));
  }

#else
  // 指针版本
  vector(const _Tp* __first, const _Tp* __last,
         const allocator_type& __a = allocator_type())
    : _Base(__last - __first, __a) 
    { _M_finish = uninitialized_copy(__first, __last, _M_start); }
#endif /* __STL_MEMBER_TEMPLATES */

  ~vector() { destroy(_M_start, _M_finish); }

  vector<_Tp, _Alloc>& operator=(const vector<_Tp, _Alloc>& __x);
  void reserve(size_type __n) {
    if (capacity() < __n) {
      const size_type __old_size = size();
	  // 分配新的并且把原来的拷贝过去
      iterator __tmp = _M_allocate_and_copy(__n, _M_start, _M_finish);
	  // 析构原来的
      destroy(_M_start, _M_finish);
	  // 释放原来的内存
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      _M_start = __tmp;
      _M_finish = __tmp + __old_size;
      _M_end_of_storage = _M_start + __n;
    }
  }

  // assign(), a generalized assignment member function.  Two
  // versions: one that takes a count, and one that takes a range.
  // The range version is a member template, so we dispatch on whether
  // or not the type is an integer.
  // assign函数，一个广义上的赋值成员函数。两个版本：一个是传入计数的版本，一个是传入范围的版本，
  // 传入范围的版本是一个类模板，所以我_InputIterator是整形进行区分
  void assign(size_type __n, const _Tp& __val) { _M_fill_assign(__n, __val); }
  void _M_fill_assign(size_type __n, const _Tp& __val);

#ifdef __STL_MEMBER_TEMPLATES
  // 各种版本的assign
  template <class _InputIterator>
  void assign(_InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_assign_dispatch(__first, __last, _Integral());
  }

  template <class _Integer>
  void _M_assign_dispatch(_Integer __n, _Integer __val, __true_type)
    { _M_fill_assign((size_type) __n, (_Tp) __val); }

  template <class _InputIter>
  void _M_assign_dispatch(_InputIter __first, _InputIter __last, __false_type)
    { _M_assign_aux(__first, __last, __ITERATOR_CATEGORY(__first)); }

  template <class _InputIterator>
  void _M_assign_aux(_InputIterator __first, _InputIterator __last,
                     input_iterator_tag);

  template <class _ForwardIterator>
  void _M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
                     forward_iterator_tag); 

#endif /* __STL_MEMBER_TEMPLATES */

  reference front() { return *begin(); }
  const_reference front() const { return *begin(); }
  reference back() { return *(end() - 1); }
  const_reference back() const { return *(end() - 1); }

  // 在尾部加入一个元素
  void push_back(const _Tp& __x) {
    if (_M_finish != _M_end_of_storage) {
      construct(_M_finish, __x);
      ++_M_finish;
    }
    else
      _M_insert_aux(end(), __x);
  }
  // 在尾部加入一个元素
  void push_back() {
    if (_M_finish != _M_end_of_storage) {
      construct(_M_finish);
      ++_M_finish;
    }
    else
      _M_insert_aux(end());
  }
  // 交换两个向量的变量
  void swap(vector<_Tp, _Alloc>& __x) {
    __STD::swap(_M_start, __x._M_start);
    __STD::swap(_M_finish, __x._M_finish);
    __STD::swap(_M_end_of_storage, __x._M_end_of_storage);
  }

  iterator insert(iterator __position, const _Tp& __x) {
    size_type __n = __position - begin();
	// 空间足够，并且是尾部插入
    if (_M_finish != _M_end_of_storage && __position == end()) {
      construct(_M_finish, __x);
      ++_M_finish;
    }
	// 走其他位置的插入函数
    else
      _M_insert_aux(__position, __x);
    return begin() + __n;
  }
  iterator insert(iterator __position) {
    size_type __n = __position - begin();
	// 空间足够，并且是尾部插入
    if (_M_finish != _M_end_of_storage && __position == end()) {
      construct(_M_finish);
      ++_M_finish;
    }
	// 走其他位置的插入函数
    else
      _M_insert_aux(__position);
    return begin() + __n;
  }
#ifdef __STL_MEMBER_TEMPLATES
  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InputIterator>
  void insert(iterator __pos, _InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_insert_dispatch(__pos, __first, __last, _Integral());
  }

  template <class _Integer>
  void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __val,
                          __true_type)
    { _M_fill_insert(__pos, (size_type) __n, (_Tp) __val); }

  template <class _InputIterator>
  void _M_insert_dispatch(iterator __pos,
                          _InputIterator __first, _InputIterator __last,
                          __false_type) {
    _M_range_insert(__pos, __first, __last, __ITERATOR_CATEGORY(__first));
  }
#else /* __STL_MEMBER_TEMPLATES */
  void insert(iterator __position,
              const_iterator __first, const_iterator __last);
#endif /* __STL_MEMBER_TEMPLATES */
  // 在指定位置插入n个x
  void insert (iterator __pos, size_type __n, const _Tp& __x)
    { _M_fill_insert(__pos, __n, __x); }

  void _M_fill_insert (iterator __pos, size_type __n, const _Tp& __x);

  void pop_back() {
    --_M_finish;
    destroy(_M_finish);
  }
  // 删除指定位置的元素
  iterator erase(iterator __position) {
    if (__position + 1 != end())
      copy(__position + 1, _M_finish, __position);
    --_M_finish;
    destroy(_M_finish);
    return __position;
  }
  // 删除迭代器之间的元素
  iterator erase(iterator __first, iterator __last) {
    iterator __i = copy(__last, _M_finish, __first);
    destroy(__i, _M_finish);
    _M_finish = _M_finish - (__last - __first);
    return __first;
  }
  // 重新调整大小
  void resize(size_type __new_size, const _Tp& __x) {
	  // 比原来的还小，删除后面的一截
    if (__new_size < size()) 
      erase(begin() + __new_size, end());
    else
		// 比原来的大，插入足够的元素
      insert(end(), __new_size - size(), __x);
  }
  void resize(size_type __new_size) { resize(__new_size, _Tp()); }
  // 清空所有的元素
  void clear() { erase(begin(), end()); }

protected:

#ifdef __STL_MEMBER_TEMPLATES
	// 分配新的内存，并把原来的拷贝到新的上面去
  template <class _ForwardIterator>
  iterator _M_allocate_and_copy(size_type __n, _ForwardIterator __first, 
                                               _ForwardIterator __last)
{
    iterator __result = _M_allocate(__n);
    __STL_TRY {
      uninitialized_copy(__first, __last, __result);
      return __result;
    }
    __STL_UNWIND(_M_deallocate(__result, __n));
  }
#else /* __STL_MEMBER_TEMPLATES */
	// 分配新的内存，并把原来的拷贝到新的上面去
  iterator _M_allocate_and_copy(size_type __n, const_iterator __first, 
                                               const_iterator __last)
  {
    iterator __result = _M_allocate(__n);
    __STL_TRY {
      uninitialized_copy(__first, __last, __result);
      return __result;
    }
    __STL_UNWIND(_M_deallocate(__result, __n));
  }
#endif /* __STL_MEMBER_TEMPLATES */


#ifdef __STL_MEMBER_TEMPLATES
  template <class _InputIterator>
  void _M_range_initialize(_InputIterator __first,  
                           _InputIterator __last, input_iterator_tag)
  {
    for ( ; __first != __last; ++__first)
      push_back(*__first);
  }

  // This function is only called by the constructor. 
  // 该函数只能被构造函数调用
  template <class _ForwardIterator>
  void _M_range_initialize(_ForwardIterator __first,
                           _ForwardIterator __last, forward_iterator_tag)
  {
    size_type __n = 0;
	// 计算元素数目
    distance(__first, __last, __n);
	// 分配内存
    _M_start = _M_allocate(__n);
    _M_end_of_storage = _M_start + __n;
	// 拷贝
    _M_finish = uninitialized_copy(__first, __last, _M_start);
  }

  template <class _InputIterator>
  void _M_range_insert(iterator __pos,
                       _InputIterator __first, _InputIterator __last,
                       input_iterator_tag);

  template <class _ForwardIterator>
  void _M_range_insert(iterator __pos,
                       _ForwardIterator __first, _ForwardIterator __last,
                       forward_iterator_tag);

#endif /* __STL_MEMBER_TEMPLATES */
};

template <class _Tp, class _Alloc>
inline bool 
operator==(const vector<_Tp, _Alloc>& __x, const vector<_Tp, _Alloc>& __y)
{
  return __x.size() == __y.size() &&
         equal(__x.begin(), __x.end(), __y.begin());
}

template <class _Tp, class _Alloc>
inline bool 
operator<(const vector<_Tp, _Alloc>& __x, const vector<_Tp, _Alloc>& __y)
{
  return lexicographical_compare(__x.begin(), __x.end(), 
                                 __y.begin(), __y.end());
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Tp, class _Alloc>
inline void swap(vector<_Tp, _Alloc>& __x, vector<_Tp, _Alloc>& __y)
{
  __x.swap(__y);
}

template <class _Tp, class _Alloc>
inline bool
operator!=(const vector<_Tp, _Alloc>& __x, const vector<_Tp, _Alloc>& __y) {
  return !(__x == __y);
}

template <class _Tp, class _Alloc>
inline bool
operator>(const vector<_Tp, _Alloc>& __x, const vector<_Tp, _Alloc>& __y) {
  return __y < __x;
}

template <class _Tp, class _Alloc>
inline bool
operator<=(const vector<_Tp, _Alloc>& __x, const vector<_Tp, _Alloc>& __y) {
  return !(__y < __x);
}

template <class _Tp, class _Alloc>
inline bool
operator>=(const vector<_Tp, _Alloc>& __x, const vector<_Tp, _Alloc>& __y) {
  return !(__x < __y);
}

#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

template <class _Tp, class _Alloc>
vector<_Tp,_Alloc>& 
vector<_Tp,_Alloc>::operator=(const vector<_Tp, _Alloc>& __x)
{
  if (&__x != this) {
    const size_type __xlen = __x.size();
	// 如果传入的大小超过了现在的容量，
    if (__xlen > capacity()) {
      iterator __tmp = _M_allocate_and_copy(__xlen, __x.begin(), __x.end());
      destroy(_M_start, _M_finish);
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      _M_start = __tmp;
      _M_end_of_storage = _M_start + __xlen;
    }
    else if (size() >= __xlen) {
		// 将传入的拷贝过去，将后面的析构掉
      iterator __i = copy(__x.begin(), __x.end(), begin());
      destroy(__i, _M_finish);
    }
    else {
		// 在原来有元素的上面拷贝
      copy(__x.begin(), __x.begin() + size(), _M_start);
	  // 原来没有元素的上面拷贝
      uninitialized_copy(__x.begin() + size(), __x.end(), _M_finish);
    }
    _M_finish = _M_start + __xlen;
  }
  return *this;
}

// assign n个__val
template <class _Tp, class _Alloc>
void vector<_Tp, _Alloc>::_M_fill_assign(size_t __n, const value_type& __val) 
{
	// 超出容量了,重新构建一个临时的，然后交换
  if (__n > capacity()) {
    vector<_Tp, _Alloc> __tmp(__n, __val, get_allocator());
    __tmp.swap(*this);
  }
  else if (__n > size()) {
	  // 填充原来有值的部分
    fill(begin(), end(), __val);
	// 填充原来没有值的部分
    _M_finish = uninitialized_fill_n(_M_finish, __n - size(), __val);
  }
  else
	  // 填充对应的长度后，将后面的删除掉
    erase(fill_n(begin(), __n, __val), end());
}

#ifdef __STL_MEMBER_TEMPLATES
// input_iterator_tag的版本
template <class _Tp, class _Alloc> template <class _InputIter>
void vector<_Tp, _Alloc>::_M_assign_aux(_InputIter __first, _InputIter __last,
                                        input_iterator_tag) {
  iterator __cur = begin();
  // 如果共同长度的元素，直接拷贝
  for ( ; __first != __last && __cur != end(); ++__cur, ++__first)
    *__cur = *__first;
  // 如果已经传入的长度小，那就把原来向量的后面那一部分删除掉
  if (__first == __last)
    erase(__cur, end());
  else
	  // 如果原来的向量长度小，将多出的部分插入到后面
    insert(end(), __first, __last);
}
// forward_iterator_tag的版本
template <class _Tp, class _Alloc> template <class _ForwardIter>
void
vector<_Tp, _Alloc>::_M_assign_aux(_ForwardIter __first, _ForwardIter __last,
                                   forward_iterator_tag) {
  size_type __len = 0;
  distance(__first, __last, __len);
  // 如果传入的长度大于容量，那就分配拷贝过去
  if (__len > capacity()) {
    iterator __tmp = _M_allocate_and_copy(__len, __first, __last);
    destroy(_M_start, _M_finish);
    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
    _M_start = __tmp;
    _M_end_of_storage = _M_finish = _M_start + __len;
  }
  // 如果大小大于传入的长度
  else if (size() >= __len) {
    iterator __new_finish = copy(__first, __last, _M_start);
	// 将后面的一截删除掉
    destroy(__new_finish, _M_finish);
    _M_finish = __new_finish;
  }
  else {
    _ForwardIter __mid = __first;
    advance(__mid, size());
	// 拷贝原来有值的部分
    copy(__first, __mid, _M_start);
	// 拷贝原来没有值的部分
    _M_finish = uninitialized_copy(__mid, __last, _M_finish);
  }
}

#endif /* __STL_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc>
void 
vector<_Tp, _Alloc>::_M_insert_aux(iterator __position, const _Tp& __x)
{
	// 还有空间
	// 这个if语句里面有两个疑问
	// 第一，为什么要先吧最后一个拷过去，在_M_finish还没有后移的情况下,不能用copy_backward(__position, _M_finish, _M_finish + 1);吗？
	// 第二，为啥先做出拷贝__x_copy，为啥不直接*__position = __x;为啥要多一个步骤?
  if (_M_finish != _M_end_of_storage) {
	  // 先拷贝最后一个
    construct(_M_finish, *(_M_finish - 1));
	// 增加空间
    ++_M_finish;
	// 将x做出一个拷贝
    _Tp __x_copy = __x;
	// 向后拷贝元素
    copy_backward(__position, _M_finish - 2, _M_finish - 1);
	// 拷贝到指定位置
    *__position = __x_copy;
  }
  else {
    const size_type __old_size = size();
	// 如果原大小为0，就为1，要不然扩展到原来的两倍
    const size_type __len = __old_size != 0 ? 2 * __old_size : 1;
    iterator __new_start = _M_allocate(__len);
    iterator __new_finish = __new_start;
    __STL_TRY {
		// 将__position位置的前半部分拷贝到新的地方，
      __new_finish = uninitialized_copy(_M_start, __position, __new_start);
	  // 用传入的值在指定位置构造元素
      construct(__new_finish, __x);
      ++__new_finish;
	  // 将后半部分拷贝过去
      __new_finish = uninitialized_copy(__position, _M_finish, __new_finish);
    }
	// 如果出现异常就还原操作
    __STL_UNWIND((destroy(__new_start,__new_finish), 
                  _M_deallocate(__new_start,__len)));
	// 销毁原来的
    destroy(begin(), end());
	// 释放内存
    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
    _M_start = __new_start;
    _M_finish = __new_finish;
    _M_end_of_storage = __new_start + __len;
  }
}

template <class _Tp, class _Alloc>
void 
vector<_Tp, _Alloc>::_M_insert_aux(iterator __position)
{
	// 还有空间，
	// 这里同样有为啥先将最后一个拷贝的疑问
	// 还有这个函数和上面基本一样，为啥不直接调用上面的函数_M_insert_aux(__position, _Tp());
  if (_M_finish != _M_end_of_storage) {
    construct(_M_finish, *(_M_finish - 1));
    ++_M_finish;
    copy_backward(__position, _M_finish - 2, _M_finish - 1);
    *__position = _Tp();
  }
  else {
    const size_type __old_size = size();
    const size_type __len = __old_size != 0 ? 2 * __old_size : 1;
    iterator __new_start = _M_allocate(__len);
    iterator __new_finish = __new_start;
    __STL_TRY {
      __new_finish = uninitialized_copy(_M_start, __position, __new_start);
      construct(__new_finish);
      ++__new_finish;
      __new_finish = uninitialized_copy(__position, _M_finish, __new_finish);
    }
    __STL_UNWIND((destroy(__new_start,__new_finish), 
                  _M_deallocate(__new_start,__len)));
    destroy(begin(), end());
    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
    _M_start = __new_start;
    _M_finish = __new_finish;
    _M_end_of_storage = __new_start + __len;
  }
}

// 在指定位置插入n个__x
template <class _Tp, class _Alloc>
void vector<_Tp, _Alloc>::_M_fill_insert(iterator __position, size_type __n, 
                                         const _Tp& __x)
{
  if (__n != 0) {
	  // 是否有多余的空间
    if (size_type(_M_end_of_storage - _M_finish) >= __n) {
      _Tp __x_copy = __x;
	  // 原来向量中pos到最后的元素数目
      const size_type __elems_after = _M_finish - __position;
      iterator __old_finish = _M_finish;
      if (__elems_after > __n) {
		  // 以前没有构造的那一部分，拷贝过去构造好
        uninitialized_copy(_M_finish - __n, _M_finish, _M_finish);
        _M_finish += __n;
		// 原来已经构造的，使用copy_backward往后移
        copy_backward(__position, __old_finish - __n, __old_finish);
		// 将__x_copy填充进去
        fill(__position, __position + __n, __x_copy);
      }
      else {
		  // 原来没有元素并且需要填充的地方，先填充好
        uninitialized_fill_n(_M_finish, __n - __elems_after, __x_copy);
		// 计数出原来元素拷贝的开始地址
        _M_finish += __n - __elems_after;
		// 将原来的元素后移__n个
        uninitialized_copy(__position, __old_finish, _M_finish);
        _M_finish += __elems_after;
		// 从pos处开始填充
        fill(__position, __old_finish, __x_copy);
      }
    }
    else {
      const size_type __old_size = size();
	  // 计算需要分配的数目
      const size_type __len = __old_size + max(__old_size, __n);
	  // 分配内存
      iterator __new_start = _M_allocate(__len);
      iterator __new_finish = __new_start;
      __STL_TRY {
		  // 拷贝pos前面部分
        __new_finish = uninitialized_copy(_M_start, __position, __new_start);
		// 填充__n个__x
        __new_finish = uninitialized_fill_n(__new_finish, __n, __x);
		// 拷贝后半部分
        __new_finish
          = uninitialized_copy(__position, _M_finish, __new_finish);
      }
      __STL_UNWIND((destroy(__new_start,__new_finish), 
                    _M_deallocate(__new_start,__len)));
      destroy(_M_start, _M_finish);
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      _M_start = __new_start;
      _M_finish = __new_finish;
      _M_end_of_storage = __new_start + __len;
    }
  }
}

#ifdef __STL_MEMBER_TEMPLATES
// input_iterator_tag版本
template <class _Tp, class _Alloc> template <class _InputIterator>
void 
vector<_Tp, _Alloc>::_M_range_insert(iterator __pos, 
                                     _InputIterator __first, 
                                     _InputIterator __last,
                                     input_iterator_tag)
{
	// 一个一个的插入进去这个效率可能会很低，因为会导致多次移动
  for ( ; __first != __last; ++__first) {
    __pos = insert(__pos, *__first);
    ++__pos;
  }
}

// forward_iterator_tag版本
template <class _Tp, class _Alloc> template <class _ForwardIterator>
void 
vector<_Tp, _Alloc>::_M_range_insert(iterator __position,
                                     _ForwardIterator __first,
                                     _ForwardIterator __last,
                                     forward_iterator_tag)
{
  if (__first != __last) {
    size_type __n = 0;
    distance(__first, __last, __n);
	// 是否有多余的空间
    if (size_type(_M_end_of_storage - _M_finish) >= __n) {
		 // 原来向量中pos到最后的元素数目
      const size_type __elems_after = _M_finish - __position;
      iterator __old_finish = _M_finish;
	  // 原来向量中pos到最后的元素数目比__n还大，表示插入进来的都在原来构造好的内存上
      if (__elems_after > __n) {
		  // 以前没有构造的那一部分，拷贝过去构造好
        uninitialized_copy(_M_finish - __n, _M_finish, _M_finish);
        _M_finish += __n;
		// 原来已经构造的，使用copy_backward往后移
        copy_backward(__position, __old_finish - __n, __old_finish);
		// 拷贝传入的
        copy(__first, __last, __position);
      }
      else {
        _ForwardIterator __mid = __first;
        advance(__mid, __elems_after);
		// 原来没有元素并且需要填充的地方，先将传入的拷贝一部分进去
        uninitialized_copy(__mid, __last, _M_finish);
        _M_finish += __n - __elems_after;
		// 将原来的元素后移__n个
        uninitialized_copy(__position, __old_finish, _M_finish);
        _M_finish += __elems_after;
		// 拷贝剩下的
        copy(__first, __mid, __position);
      }
    }
    else {
      const size_type __old_size = size();
	  // 计算需要分配的数目
      const size_type __len = __old_size + max(__old_size, __n);
	  // 分配内存
      iterator __new_start = _M_allocate(__len);
      iterator __new_finish = __new_start;
      __STL_TRY {
		// 拷贝pos前面部分
        __new_finish = uninitialized_copy(_M_start, __position, __new_start);
		// 拷贝传入的部分
        __new_finish = uninitialized_copy(__first, __last, __new_finish);
		// 拷贝后面的部分
        __new_finish
          = uninitialized_copy(__position, _M_finish, __new_finish);
      }
      __STL_UNWIND((destroy(__new_start,__new_finish), 
                    _M_deallocate(__new_start,__len)));
	  // 析构原来的
      destroy(_M_start, _M_finish);
	  // 释放原来的内存
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      _M_start = __new_start;
      _M_finish = __new_finish;
      _M_end_of_storage = __new_start + __len;
    }
  }
}

#else /* __STL_MEMBER_TEMPLATES */
// 算法和前面一样
template <class _Tp, class _Alloc>
void 
vector<_Tp, _Alloc>::insert(iterator __position, 
                            const_iterator __first, 
                            const_iterator __last)
{
  if (__first != __last) {
    size_type __n = 0;
    distance(__first, __last, __n);
	// 是否有多余的空间
    if (size_type(_M_end_of_storage - _M_finish) >= __n) {
		// 原来向量中pos到最后的元素数目
      const size_type __elems_after = _M_finish - __position;
      iterator __old_finish = _M_finish;
	  // 原来向量中pos到最后的元素数目比__n还大，表示插入进来的都在原来构造好的内存上
      if (__elems_after > __n) {
		  // 以前没有构造的那一部分，拷贝过去构造好
        uninitialized_copy(_M_finish - __n, _M_finish, _M_finish);
        _M_finish += __n;
		// 原来已经构造的，使用copy_backward往后移
        copy_backward(__position, __old_finish - __n, __old_finish);
		// 拷贝传入的
        copy(__first, __last, __position);
      }
      else {
		  // 原来没有元素并且需要填充的地方，先将传入的拷贝一部分进去
        uninitialized_copy(__first + __elems_after, __last, _M_finish);
        _M_finish += __n - __elems_after;
		// 将原来的元素后移__n个
        uninitialized_copy(__position, __old_finish, _M_finish);
        _M_finish += __elems_after;
		// 拷贝剩下的
        copy(__first, __first + __elems_after, __position);
      }
    }
    else {
      const size_type __old_size = size();
	  // 计算需要分配的数目
      const size_type __len = __old_size + max(__old_size, __n);
      iterator __new_start = _M_allocate(__len);
      iterator __new_finish = __new_start;
      __STL_TRY {
		  // 拷贝pos前面部分
        __new_finish = uninitialized_copy(_M_start, __position, __new_start);
		// 拷贝传入的部分
        __new_finish = uninitialized_copy(__first, __last, __new_finish);
		// 拷贝后面的部分
        __new_finish
          = uninitialized_copy(__position, _M_finish, __new_finish);
      }
      __STL_UNWIND((destroy(__new_start,__new_finish),
                    _M_deallocate(__new_start,__len)));
	  // 析构原来的
      destroy(_M_start, _M_finish);
	  // 释放原来的内存
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      _M_start = __new_start;
      _M_finish = __new_finish;
      _M_end_of_storage = __new_start + __len;
    }
  }
}

#endif /* __STL_MEMBER_TEMPLATES */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

__STL_END_NAMESPACE 

#endif /* __SGI_STL_INTERNAL_VECTOR_H */

// Local Variables:
// mode:C++
// End:
