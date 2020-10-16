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

#ifndef __SGI_STL_INTERNAL_HEAP_H
#define __SGI_STL_INTERNAL_HEAP_H

__STL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1209
#endif

// Heap-manipulation functions: push_heap, pop_heap, make_heap, sort_heap.

template <class _RandomAccessIterator, class _Distance, class _Tp>
void 
__push_heap(_RandomAccessIterator __first,
            _Distance __holeIndex, _Distance __topIndex, _Tp __value)
{
  // 压入元素的父节点索引
  _Distance __parent = (__holeIndex - 1) / 2;
  // 如果没有到达根节点，并且压入的值大于父节点
  while (__holeIndex > __topIndex && *(__first + __parent) < __value) {
	  // 将父节点移动到子节点
    *(__first + __holeIndex) = *(__first + __parent);
	// 继续进行新一轮的比较和移动
    __holeIndex = __parent;
    __parent = (__holeIndex - 1) / 2;
  }
  // 新的值找到对应的位置
  *(__first + __holeIndex) = __value;
}

template <class _RandomAccessIterator, class _Distance, class _Tp>
inline void 
__push_heap_aux(_RandomAccessIterator __first,
                _RandomAccessIterator __last, _Distance*, _Tp*)
{
  __push_heap(__first, _Distance((__last - __first) - 1), _Distance(0), 
              _Tp(*(__last - 1)));
}

template <class _RandomAccessIterator>
inline void 
push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  __push_heap_aux(__first, __last,
                  __DISTANCE_TYPE(__first), __VALUE_TYPE(__first));
}

template <class _RandomAccessIterator, class _Distance, class _Tp, 
          class _Compare>
void
__push_heap(_RandomAccessIterator __first, _Distance __holeIndex,
            _Distance __topIndex, _Tp __value, _Compare __comp)
{
	// 压入元素的父节点索引
  _Distance __parent = (__holeIndex - 1) / 2;
  // 如果没有到达根节点，并且压入的值大于父节点
  while (__holeIndex > __topIndex && __comp(*(__first + __parent), __value)) {
	  // 将父节点移动到子节点
    *(__first + __holeIndex) = *(__first + __parent);
	// 继续进行新一轮的比较和移动
    __holeIndex = __parent;
    __parent = (__holeIndex - 1) / 2;
  }
  // 新的值找到对应的位置
  *(__first + __holeIndex) = __value;
}

template <class _RandomAccessIterator, class _Compare,
          class _Distance, class _Tp>
inline void 
__push_heap_aux(_RandomAccessIterator __first,
                _RandomAccessIterator __last, _Compare __comp,
                _Distance*, _Tp*) 
{
  __push_heap(__first, _Distance((__last - __first) - 1), _Distance(0), 
              _Tp(*(__last - 1)), __comp);
}

template <class _RandomAccessIterator, class _Compare>
inline void 
push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
          _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __push_heap_aux(__first, __last, __comp,
                  __DISTANCE_TYPE(__first), __VALUE_TYPE(__first));
}

template <class _RandomAccessIterator, class _Distance, class _Tp>
void 
__adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex,
              _Distance __len, _Tp __value)
{
  _Distance __topIndex = __holeIndex;
  _Distance __secondChild = 2 * __holeIndex + 2;
  // 右边的子节点索引小于长度
  while (__secondChild < __len) {
	  // 如果左边的子节点比较大，得到左边子节点的索引，或者直接用右边子节点的索引
    if (*(__first + __secondChild) < *(__first + (__secondChild - 1)))
      __secondChild--;
	// 将比较大的赋值给空洞
    *(__first + __holeIndex) = *(__first + __secondChild);
	// 循环处理到叶子节点
    __holeIndex = __secondChild;
    __secondChild = 2 * (__secondChild + 1);
  }
  // 如果计算出的右边索引和长度一样，说明没有右边的子节点
  if (__secondChild == __len) {
	  // 使用左子节点
    *(__first + __holeIndex) = *(__first + (__secondChild - 1));
    __holeIndex = __secondChild - 1;
  }
  // 很可能换了一个节点，换了节点后，该节点可能比他的父节点大，所以需要使用__push_heap找到合适的位置
  __push_heap(__first, __holeIndex, __topIndex, __value);
}

template <class _RandomAccessIterator, class _Tp, class _Distance>
inline void 
__pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
           _RandomAccessIterator __result, _Tp __value, _Distance*)
{
  *__result = *__first;
  __adjust_heap(__first, _Distance(0), _Distance(__last - __first), __value);
}

template <class _RandomAccessIterator, class _Tp>
inline void 
__pop_heap_aux(_RandomAccessIterator __first, _RandomAccessIterator __last,
               _Tp*)
{
  __pop_heap(__first, __last - 1, __last - 1, 
             _Tp(*(__last - 1)), __DISTANCE_TYPE(__first));
}

template <class _RandomAccessIterator>
inline void pop_heap(_RandomAccessIterator __first, 
                     _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  __pop_heap_aux(__first, __last, __VALUE_TYPE(__first));
}
// __holeIndex位置找到合适的子节点来填充
template <class _RandomAccessIterator, class _Distance,
          class _Tp, class _Compare>
void
__adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex,
              _Distance __len, _Tp __value, _Compare __comp)
{
  _Distance __topIndex = __holeIndex;
  _Distance __secondChild = 2 * __holeIndex + 2;
  // 右边的子节点索引小于长度
  while (__secondChild < __len) {
	  // 如果左边的子节点比较大，得到左边子节点的索引，或者直接用右边子节点的索引
    if (__comp(*(__first + __secondChild), *(__first + (__secondChild - 1))))
      __secondChild--;
	// 将比较大的赋值给空洞
    *(__first + __holeIndex) = *(__first + __secondChild);
	// 循环处理到叶子节点
    __holeIndex = __secondChild;
    __secondChild = 2 * (__secondChild + 1);
  }
  // 如果计算出的右边索引和长度一样，说明没有右边的子节点
  if (__secondChild == __len) {
	  // 使用左子节点
    *(__first + __holeIndex) = *(__first + (__secondChild - 1));
    __holeIndex = __secondChild - 1;
  }
  // 很可能换了一个节点，换了节点后，该节点可能比他的父节点大，所以需要使用__push_heap找到合适的位置
  __push_heap(__first, __holeIndex, __topIndex, __value, __comp);
}

template <class _RandomAccessIterator, class _Tp, class _Compare, 
          class _Distance>
inline void 
__pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
           _RandomAccessIterator __result, _Tp __value, _Compare __comp,
           _Distance*)
{
	// __value为最后一个的值
	// 将第一个调整到最后一个
  *__result = *__first;
  // 洞的索引是第一个
  __adjust_heap(__first, _Distance(0), _Distance(__last - __first), 
                __value, __comp);
}

template <class _RandomAccessIterator, class _Tp, class _Compare>
inline void 
__pop_heap_aux(_RandomAccessIterator __first,
               _RandomAccessIterator __last, _Tp*, _Compare __comp)
{
  __pop_heap(__first, __last - 1, __last - 1, _Tp(*(__last - 1)), __comp,
             __DISTANCE_TYPE(__first));
}

template <class _RandomAccessIterator, class _Compare>
inline void 
pop_heap(_RandomAccessIterator __first,
         _RandomAccessIterator __last, _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __pop_heap_aux(__first, __last, __VALUE_TYPE(__first), __comp);
}

template <class _RandomAccessIterator, class _Tp, class _Distance>
void 
__make_heap(_RandomAccessIterator __first,
            _RandomAccessIterator __last, _Tp*, _Distance*)
{
  if (__last - __first < 2) return;
  _Distance __len = __last - __first;
  _Distance __parent = (__len - 2)/2;
    
  while (true) {
    __adjust_heap(__first, __parent, __len, _Tp(*(__first + __parent)));
    if (__parent == 0) return;
    __parent--;
  }
}

template <class _RandomAccessIterator>
inline void 
make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  __make_heap(__first, __last,
              __VALUE_TYPE(__first), __DISTANCE_TYPE(__first));
}

template <class _RandomAccessIterator, class _Compare,
          class _Tp, class _Distance>
void
__make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
            _Compare __comp, _Tp*, _Distance*)
{
  if (__last - __first < 2) return;
  _Distance __len = __last - __first;
  // 找到第一个需要重排的子树头部
  _Distance __parent = (__len - 2)/2;
  while (true) {
	  // 调整子树成为最大树
    __adjust_heap(__first, __parent, __len, _Tp(*(__first + __parent)),
                  __comp);
    if (__parent == 0) return;
    __parent--;
  }
}

template <class _RandomAccessIterator, class _Compare>
inline void 
make_heap(_RandomAccessIterator __first, 
          _RandomAccessIterator __last, _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __make_heap(__first, __last, __comp,
              __VALUE_TYPE(__first), __DISTANCE_TYPE(__first));
}
// pop操作会把最大的值放在尾端，如果一直操作，整个向量就会变成一个从小到大的向量，但是注意：该向量就不是一个合法的最大堆了
template <class _RandomAccessIterator>
void sort_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  while (__last - __first > 1)
    pop_heap(__first, __last--);
}

// pop操作会把最大的值放在尾端，如果一直操作，整个向量就会变成一个从小到大的向量，但是注意：该向量就不是一个合法的最大堆了
template <class _RandomAccessIterator, class _Compare>
void 
sort_heap(_RandomAccessIterator __first,
          _RandomAccessIterator __last, _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  while (__last - __first > 1)
    pop_heap(__first, __last--, __comp);
}

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1209
#endif

__STL_END_NAMESPACE

#endif /* __SGI_STL_INTERNAL_HEAP_H */

// Local Variables:
// mode:C++
// End:
