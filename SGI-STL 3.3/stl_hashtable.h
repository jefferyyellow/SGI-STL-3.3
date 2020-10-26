/*
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
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

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_STL_INTERNAL_HASHTABLE_H
#define __SGI_STL_INTERNAL_HASHTABLE_H

// Hashtable class, used to implement the hashed associative containers
// hash_set, hash_map, hash_multiset, and hash_multimap.
// Hashtable�࣬����ʵ��hashed��������hash_set, hash_map, hash_multiset, and hash_multimap
#include <stl_algobase.h>
#include <stl_alloc.h>
#include <stl_construct.h>
#include <stl_tempbuf.h>
#include <stl_algo.h>
#include <stl_uninitialized.h>
#include <stl_function.h>
#include <stl_vector.h>
#include <stl_hash_fun.h>

__STL_BEGIN_NAMESPACE

// ��ϣ��Ľڵ�,��������Ľṹ
template <class _Val>
struct _Hashtable_node
{
  _Hashtable_node* _M_next;
  _Val _M_val;
};  

template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc = alloc>
class hashtable;

template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
struct _Hashtable_iterator;

template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
struct _Hashtable_const_iterator;

template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
struct _Hashtable_iterator {
  typedef hashtable<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc>
          _Hashtable;
  typedef _Hashtable_iterator<_Val, _Key, _HashFcn, 
                              _ExtractKey, _EqualKey, _Alloc>
          iterator;
  typedef _Hashtable_const_iterator<_Val, _Key, _HashFcn, 
                                    _ExtractKey, _EqualKey, _Alloc>
          const_iterator;
  typedef _Hashtable_node<_Val> _Node;

  typedef forward_iterator_tag iterator_category;
  typedef _Val value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef _Val& reference;
  typedef _Val* pointer;

  // ��ǰ�ڵ�
  _Node* _M_cur;
  // hash��
  _Hashtable* _M_ht;

  _Hashtable_iterator(_Node* __n, _Hashtable* __tab) 
    : _M_cur(__n), _M_ht(__tab) {}
  _Hashtable_iterator() {}
  reference operator*() const { return _M_cur->_M_val; }
#ifndef __SGI_STL_NO_ARROW_OPERATOR
  pointer operator->() const { return &(operator*()); }
#endif /* __SGI_STL_NO_ARROW_OPERATOR */
  iterator& operator++();
  iterator operator++(int);
  bool operator==(const iterator& __it) const
    { return _M_cur == __it._M_cur; }
  bool operator!=(const iterator& __it) const
    { return _M_cur != __it._M_cur; }
};


template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
struct _Hashtable_const_iterator {
  typedef hashtable<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc>
          _Hashtable;
  typedef _Hashtable_iterator<_Val,_Key,_HashFcn, 
                              _ExtractKey,_EqualKey,_Alloc>
          iterator;
  typedef _Hashtable_const_iterator<_Val, _Key, _HashFcn, 
                                    _ExtractKey, _EqualKey, _Alloc>
          const_iterator;
  typedef _Hashtable_node<_Val> _Node;

  typedef forward_iterator_tag iterator_category;
  typedef _Val value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef const _Val& reference;
  typedef const _Val* pointer;

  const _Node* _M_cur;
  const _Hashtable* _M_ht;

  _Hashtable_const_iterator(const _Node* __n, const _Hashtable* __tab)
    : _M_cur(__n), _M_ht(__tab) {}
  _Hashtable_const_iterator() {}
  _Hashtable_const_iterator(const iterator& __it) 
    : _M_cur(__it._M_cur), _M_ht(__it._M_ht) {}
  reference operator*() const { return _M_cur->_M_val; }
#ifndef __SGI_STL_NO_ARROW_OPERATOR
  pointer operator->() const { return &(operator*()); }
#endif /* __SGI_STL_NO_ARROW_OPERATOR */
  const_iterator& operator++();
  const_iterator operator++(int);
  bool operator==(const const_iterator& __it) const 
    { return _M_cur == __it._M_cur; }
  bool operator!=(const const_iterator& __it) const 
    { return _M_cur != __it._M_cur; }
};

// Note: assumes long is at least 32 bits.
enum { __stl_num_primes = 28 };
// ��Щ�����������������ù�ϣ�����ֲ����ȣ����Ҷ��Ǵ��³������Ĺ�ϵ,���һ��Ӧ����2^32-1����ر��������
static const unsigned long __stl_prime_list[__stl_num_primes] =
{
  53ul,         97ul,         193ul,       389ul,       769ul,
  1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
  49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
  1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
  50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
  1610612741ul, 3221225473ul, 4294967291ul
};

// �õ�__stl_prime_list�б��д���__n����С����
inline unsigned long __stl_next_prime(unsigned long __n)
{
  const unsigned long* __first = __stl_prime_list;
  const unsigned long* __last = __stl_prime_list + (int)__stl_num_primes;
  const unsigned long* pos = lower_bound(__first, __last, __n);
  return pos == __last ? *(__last - 1) : *pos;
}

// Forward declaration of operator==.

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
class hashtable;

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
bool operator==(const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht1,
                const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht2);


// Hashtables handle allocators a bit differently than other containers
//  do.  If we're using standard-conforming allocators, then a hashtable
//  unconditionally has a member variable to hold its allocator, even if
//  it so happens that all instances of the allocator type are identical.
// This is because, for hashtables, this extra storage is negligible.  
//  Additionally, a base class wouldn't serve any other purposes; it 
//  wouldn't, for example, simplify the exception-handling code.
// Hashtables��������������������һ�㲻ͬ���������ʹ�÷��ϱ�׼��������
// ��һ��hashtable������ӵ��һ����Ա���������������������ʹ����
// ���������͵�����ʵ��������ͬ�ġ�������Ϊ����hashtables��˵��������
// �Ĵ洢���Ժ��Բ��ơ����⣬һ�����಻�������κ�����Ŀ�ģ����磬������
// ���쳣������롣

template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
class hashtable {
public:
  typedef _Key key_type;
  typedef _Val value_type;
  typedef _HashFcn hasher;
  typedef _EqualKey key_equal;

  typedef size_t            size_type;
  typedef ptrdiff_t         difference_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type&       reference;
  typedef const value_type& const_reference;

  hasher hash_funct() const { return _M_hash; }
  key_equal key_eq() const { return _M_equals; }

private:
  typedef _Hashtable_node<_Val> _Node;

#ifdef __STL_USE_STD_ALLOCATORS
public:
  typedef typename _Alloc_traits<_Val,_Alloc>::allocator_type allocator_type;
  allocator_type get_allocator() const { return _M_node_allocator; }
private:
  typename _Alloc_traits<_Node, _Alloc>::allocator_type _M_node_allocator;
  _Node* _M_get_node() { return _M_node_allocator.allocate(1); }
  void _M_put_node(_Node* __p) { _M_node_allocator.deallocate(__p, 1); }
# define __HASH_ALLOC_INIT(__a) _M_node_allocator(__a), 
#else /* __STL_USE_STD_ALLOCATORS */
public:
  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }
private:
  typedef simple_alloc<_Node, _Alloc> _M_node_allocator_type;
  _Node* _M_get_node() { return _M_node_allocator_type::allocate(1); }
  void _M_put_node(_Node* __p) { _M_node_allocator_type::deallocate(__p, 1); }
# define __HASH_ALLOC_INIT(__a)
#endif /* __STL_USE_STD_ALLOCATORS */

private:
  // ��ϣ����
  hasher                _M_hash;
  key_equal             _M_equals;
  _ExtractKey           _M_get_key;
  // Ͱλ
  vector<_Node*,_Alloc> _M_buckets;
  // Ԫ����Ŀ
  size_type             _M_num_elements;

public:
  typedef _Hashtable_iterator<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc>
          iterator;
  typedef _Hashtable_const_iterator<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,
                                    _Alloc>
          const_iterator;

  friend struct
  _Hashtable_iterator<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc>;
  friend struct
  _Hashtable_const_iterator<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc>;

public:
  hashtable(size_type __n,
            const _HashFcn&    __hf,
            const _EqualKey&   __eql,
            const _ExtractKey& __ext,
            const allocator_type& __a = allocator_type())
    : __HASH_ALLOC_INIT(__a)
      _M_hash(__hf),
      _M_equals(__eql),
      _M_get_key(__ext),
      _M_buckets(__a),
      _M_num_elements(0)
  {
    _M_initialize_buckets(__n);
  }

  hashtable(size_type __n,
            const _HashFcn&    __hf,
            const _EqualKey&   __eql,
            const allocator_type& __a = allocator_type())
    : __HASH_ALLOC_INIT(__a)
      _M_hash(__hf),
      _M_equals(__eql),
      _M_get_key(_ExtractKey()),
      _M_buckets(__a),
      _M_num_elements(0)
  {
    _M_initialize_buckets(__n);
  }

  hashtable(const hashtable& __ht)
    : __HASH_ALLOC_INIT(__ht.get_allocator())
      _M_hash(__ht._M_hash),
      _M_equals(__ht._M_equals),
      _M_get_key(__ht._M_get_key),
      _M_buckets(__ht.get_allocator()),
      _M_num_elements(0)
  {
    _M_copy_from(__ht);
  }

#undef __HASH_ALLOC_INIT

  hashtable& operator= (const hashtable& __ht)
  {
    if (&__ht != this) {
		// ����գ��ٿ���
      clear();
      _M_hash = __ht._M_hash;
      _M_equals = __ht._M_equals;
      _M_get_key = __ht._M_get_key;
      _M_copy_from(__ht);
    }
    return *this;
  }
  // ��սڵ�
  ~hashtable() { clear(); }

  size_type size() const { return _M_num_elements; }
  size_type max_size() const { return size_type(-1); }
  bool empty() const { return size() == 0; }
  // ��������hashtable
  void swap(hashtable& __ht)
  {
    __STD::swap(_M_hash, __ht._M_hash);
    __STD::swap(_M_equals, __ht._M_equals);
    __STD::swap(_M_get_key, __ht._M_get_key);
    _M_buckets.swap(__ht._M_buckets);
    __STD::swap(_M_num_elements, __ht._M_num_elements);
  }
  // �ҵ���һ���ǿյ�Ͱλ�ڵ�
  iterator begin()
  { 
    for (size_type __n = 0; __n < _M_buckets.size(); ++__n)
      if (_M_buckets[__n])
        return iterator(_M_buckets[__n], this);
    return end();
  }

  iterator end() { return iterator(0, this); }

  // �ҵ���һ���ǿյ�Ͱλ�ڵ�
  const_iterator begin() const
  {
    for (size_type __n = 0; __n < _M_buckets.size(); ++__n)
      if (_M_buckets[__n])
        return const_iterator(_M_buckets[__n], this);
    return end();
  }

  const_iterator end() const { return const_iterator(0, this); }

#ifdef __STL_MEMBER_TEMPLATES
  template <class _Vl, class _Ky, class _HF, class _Ex, class _Eq, class _Al>
  friend bool operator== (const hashtable<_Vl, _Ky, _HF, _Ex, _Eq, _Al>&,
                          const hashtable<_Vl, _Ky, _HF, _Ex, _Eq, _Al>&);
#else /* __STL_MEMBER_TEMPLATES */
  friend bool __STD_QUALIFIER
  operator== __STL_NULL_TMPL_ARGS (const hashtable&, const hashtable&);
#endif /* __STL_MEMBER_TEMPLATES */

public:
	// Ͱλ��Ŀ
  size_type bucket_count() const { return _M_buckets.size(); }
  // ����Ͱλ��Ŀ��ֱ��ȡͰλ������������һ��ֵ
  size_type max_bucket_count() const
    { return __stl_prime_list[(int)__stl_num_primes - 1]; } 

  // ָ��Ͱλ������Ͱ�����Ԫ����Ŀ
  size_type elems_in_bucket(size_type __bucket) const
  {
    size_type __result = 0;
	// ����Ͱλ����
    for (_Node* __cur = _M_buckets[__bucket]; __cur; __cur = __cur->_M_next)
      __result += 1;
    return __result;
  }

  // Ψһ����
  pair<iterator, bool> insert_unique(const value_type& __obj)
  {
	// �������ô�С
    resize(_M_num_elements + 1);
    return insert_unique_noresize(__obj);
  }

  // �����ҵ���ȵĽڵ㣬�ҽ��ڵ�һ����Ƚڵ���棬���û�У�ֱ�ӷ���Ͱλ��ǰ��
  iterator insert_equal(const value_type& __obj)
  {
    resize(_M_num_elements + 1);
    return insert_equal_noresize(__obj);
  }

  pair<iterator, bool> insert_unique_noresize(const value_type& __obj);
  iterator insert_equal_noresize(const value_type& __obj);
 
#ifdef __STL_MEMBER_TEMPLATES
  // Ψһ����
  template <class _InputIterator>
  void insert_unique(_InputIterator __f, _InputIterator __l)
  {
    insert_unique(__f, __l, __ITERATOR_CATEGORY(__f));
  }
  // ��Ȳ���
  template <class _InputIterator>
  void insert_equal(_InputIterator __f, _InputIterator __l)
  {
    insert_equal(__f, __l, __ITERATOR_CATEGORY(__f));
  }

  // Ψһ����
  template <class _InputIterator>
  void insert_unique(_InputIterator __f, _InputIterator __l,
                     input_iterator_tag)
  {
    for ( ; __f != __l; ++__f)
      insert_unique(*__f);
  }
  // ��Ȳ���
  template <class _InputIterator>
  void insert_equal(_InputIterator __f, _InputIterator __l,
                    input_iterator_tag)
  {
    for ( ; __f != __l; ++__f)
      insert_equal(*__f);
  }
  // Ψһ����
  template <class _ForwardIterator>
  void insert_unique(_ForwardIterator __f, _ForwardIterator __l,
                     forward_iterator_tag)
  {
    size_type __n = 0;
    distance(__f, __l, __n);
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_unique_noresize(*__f);
  }
  // ��Ȳ���
  template <class _ForwardIterator>
  void insert_equal(_ForwardIterator __f, _ForwardIterator __l,
                    forward_iterator_tag)
  {
    size_type __n = 0;
    distance(__f, __l, __n);
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_equal_noresize(*__f);
  }

#else /* __STL_MEMBER_TEMPLATES */
  // Ψһ����
  void insert_unique(const value_type* __f, const value_type* __l)
  {
    size_type __n = __l - __f;
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_unique_noresize(*__f);
  }
  // ��Ȳ���
  void insert_equal(const value_type* __f, const value_type* __l)
  {
    size_type __n = __l - __f;
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_equal_noresize(*__f);
  }
  // Ψһ����
  void insert_unique(const_iterator __f, const_iterator __l)
  {
    size_type __n = 0;
    distance(__f, __l, __n);
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_unique_noresize(*__f);
  }
  // ��Ȳ���
  void insert_equal(const_iterator __f, const_iterator __l)
  {
    size_type __n = 0;
    distance(__f, __l, __n);
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_equal_noresize(*__f);
  }
#endif /*__STL_MEMBER_TEMPLATES */

  reference find_or_insert(const value_type& __obj);

  // �ҵ���Ӧ���ĵ�һ���ڵ�
  iterator find(const key_type& __key) 
  {
	  // �ҵ���Ӧ��Ͱλ
    size_type __n = _M_bkt_num_key(__key);
	// �����ڵ�
    _Node* __first;
    for ( __first = _M_buckets[__n];
          __first && !_M_equals(_M_get_key(__first->_M_val), __key);
          __first = __first->_M_next)
      {}
    return iterator(__first, this);
  } 
  // �ҵ���Ӧ���ĵ�һ���ڵ�
  const_iterator find(const key_type& __key) const
  {
	  // �ҵ���Ӧ��Ͱλ
    size_type __n = _M_bkt_num_key(__key);
	// �����ڵ�
    const _Node* __first;
    for ( __first = _M_buckets[__n];
          __first && !_M_equals(_M_get_key(__first->_M_val), __key);
          __first = __first->_M_next)
      {}
    return const_iterator(__first, this);
  } 
  // �����Ӧ����ֵ�ĸ���
  size_type count(const key_type& __key) const
  {
    const size_type __n = _M_bkt_num_key(__key);
    size_type __result = 0;

    for (const _Node* __cur = _M_buckets[__n]; __cur; __cur = __cur->_M_next)
      if (_M_equals(_M_get_key(__cur->_M_val), __key))
        ++__result;
    return __result;
  }

  pair<iterator, iterator> 
  equal_range(const key_type& __key);

  pair<const_iterator, const_iterator> 
  equal_range(const key_type& __key) const;

  size_type erase(const key_type& __key);
  void erase(const iterator& __it);
  void erase(iterator __first, iterator __last);

  void erase(const const_iterator& __it);
  void erase(const_iterator __first, const_iterator __last);

  void resize(size_type __num_elements_hint);
  void clear();

private:
	// __n��Ӧ��ϣͰλ�Ĵ�С
  size_type _M_next_size(size_type __n) const
    { return __stl_next_prime(__n); }
  // ��ʼ��__n��Ͱλ�Ĺ�ϣ��
  void _M_initialize_buckets(size_type __n)
  {
    const size_type __n_buckets = _M_next_size(__n);
    _M_buckets.reserve(__n_buckets);
    _M_buckets.insert(_M_buckets.end(), __n_buckets, (_Node*) 0);
    _M_num_elements = 0;
  }

  // ͨ����ֵ�õ�Ͱλ����
  size_type _M_bkt_num_key(const key_type& __key) const
  {
    return _M_bkt_num_key(__key, _M_buckets.size());
  }

  // ͨ��ֵʵ���õ�Ͱλ����
  size_type _M_bkt_num(const value_type& __obj) const
  {
    return _M_bkt_num_key(_M_get_key(__obj));
  }

  // ͨ����ֵ��Ͱλ��С�õ�Ͱλ����
  size_type _M_bkt_num_key(const key_type& __key, size_t __n) const
  {
    return _M_hash(__key) % __n;
  }

  // ͨ��ֵʵ����Ͱλ��С�õ�Ͱλ����
  size_type _M_bkt_num(const value_type& __obj, size_t __n) const
  {
    return _M_bkt_num_key(_M_get_key(__obj), __n);
  }

  // ����һ���µ�hash�ڵ�
  _Node* _M_new_node(const value_type& __obj)
  {
	  // ����ڵ�
    _Node* __n = _M_get_node();
    __n->_M_next = 0;
    __STL_TRY {
	  // �����ڵ�
      construct(&__n->_M_val, __obj);
      return __n;
    }
    __STL_UNWIND(_M_put_node(__n));
  }
  
  // ���ٽڵ�
  void _M_delete_node(_Node* __n)
  {
	// �����ڵ�
    destroy(&__n->_M_val);
	// �ͷ��ڴ�
    _M_put_node(__n);
  }

  void _M_erase_bucket(const size_type __n, _Node* __first, _Node* __last);
  void _M_erase_bucket(const size_type __n, _Node* __last);

  void _M_copy_from(const hashtable& __ht);

};

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::operator++()
{
  const _Node* __old = _M_cur;
  // �ҵ���ǰͰλ����һ���ڵ�
  _M_cur = _M_cur->_M_next;
  // ����ڵ�Ϊ��
  if (!_M_cur) {
	  // ����һ���ǿ�Ͱλ�ĵ�һ���ڵ�
    size_type __bucket = _M_ht->_M_bkt_num(__old->_M_val);
    while (!_M_cur && ++__bucket < _M_ht->_M_buckets.size())
      _M_cur = _M_ht->_M_buckets[__bucket];
  }
  return *this;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
inline _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::operator++(int)
{
  iterator __tmp = *this;
  ++*this;
  return __tmp;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
_Hashtable_const_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&
_Hashtable_const_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::operator++()
{
  const _Node* __old = _M_cur;
  // �ҵ���ǰͰλ����һ���ڵ�
  _M_cur = _M_cur->_M_next;
  // ����ڵ�Ϊ��
  if (!_M_cur) {
    size_type __bucket = _M_ht->_M_bkt_num(__old->_M_val);
	// ����һ���ǿ�Ͱλ�ĵ�һ���ڵ�
    while (!_M_cur && ++__bucket < _M_ht->_M_buckets.size())
      _M_cur = _M_ht->_M_buckets[__bucket];
  }
  return *this;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
inline _Hashtable_const_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>
_Hashtable_const_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::operator++(int)
{
  const_iterator __tmp = *this;
  ++*this;
  return __tmp;
}

#ifndef __STL_CLASS_PARTIAL_SPECIALIZATION

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
inline forward_iterator_tag
iterator_category(const _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&)
{
  return forward_iterator_tag();
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
inline _Val* 
value_type(const _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&)
{
  return (_Val*) 0;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
inline hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::difference_type*
distance_type(const _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&)
{
  return (hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::difference_type*) 0;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
inline forward_iterator_tag
iterator_category(const _Hashtable_const_iterator<_Val,_Key,_HF,
                                                  _ExK,_EqK,_All>&)
{
  return forward_iterator_tag();
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
inline _Val* 
value_type(const _Hashtable_const_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&)
{
  return (_Val*) 0;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
inline hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::difference_type*
distance_type(const _Hashtable_const_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&)
{
  return (hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::difference_type*) 0;
}

#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

// ����hash table�ıȽ�
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
bool operator==(const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht1,
                const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht2)
{
  typedef typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::_Node _Node;
  // Ͱλ��С��һ�£�ֱ�ӷ���false
  if (__ht1._M_buckets.size() != __ht2._M_buckets.size())
    return false;
  // ����Ͱλ
  for (int __n = 0; __n < __ht1._M_buckets.size(); ++__n) {
    _Node* __cur1 = __ht1._M_buckets[__n];
    _Node* __cur2 = __ht2._M_buckets[__n];
	// ����Ͱλ�е�ֵ������Ƿ����
    for ( ; __cur1 && __cur2 && __cur1->_M_val == __cur2->_M_val;
          __cur1 = __cur1->_M_next, __cur2 = __cur2->_M_next)
      {}
	// ���û�е���������ѭ���ˣ�ֱ�ӷ���false
    if (__cur1 || __cur2)
      return false;
  }
  return true;
}  

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
inline bool operator!=(const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht1,
                       const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht2) {
  return !(__ht1 == __ht2);
}

template <class _Val, class _Key, class _HF, class _Extract, class _EqKey, 
          class _All>
inline void swap(hashtable<_Val, _Key, _HF, _Extract, _EqKey, _All>& __ht1,
                 hashtable<_Val, _Key, _HF, _Extract, _EqKey, _All>& __ht2) {
  __ht1.swap(__ht2);
}

#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

// Ψһ����
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
pair<typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator, bool> 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::insert_unique_noresize(const value_type& __obj)
{
	// ����Ͱλ
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = _M_buckets[__n];
  // ����Ͱλ���Ƚϼ�ֵ���������ȵķ��ش���
  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next) 
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj)))
      return pair<iterator, bool>(iterator(__cur, this), false);
  // �����½ڵ�
  _Node* __tmp = _M_new_node(__obj);
  // �ҽ���Ͱλ�ĵ�һ��λ��
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  // ����Ԫ�ظ���
  ++_M_num_elements;
  return pair<iterator, bool>(iterator(__tmp, this), true);
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::insert_equal_noresize(const value_type& __obj)
{
	// ����Ͱλ
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = _M_buckets[__n];

  // ����Ͱλ���Ƚϼ�ֵ���������ȣ��ͽ��ڵ�ҽ�����Ƚڵ���棬Ȼ�󷵻�
  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next) 
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj))) {
      _Node* __tmp = _M_new_node(__obj);
      __tmp->_M_next = __cur->_M_next;
      __cur->_M_next = __tmp;
      ++_M_num_elements;
      return iterator(__tmp, this);
    }

  // �����½ڵ�
  _Node* __tmp = _M_new_node(__obj);
  // �ҽ���Ͱλ�ĵ�һ��λ��
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  // ����Ԫ�ظ���
  ++_M_num_elements;
  return iterator(__tmp, this);
}
// �ҵ���Ӧ��ֵ��Ӧ�Ľڵ㣬���û�оͲ���һ��
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::reference 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::find_or_insert(const value_type& __obj)
{
  resize(_M_num_elements + 1);
  // �ҵ���Ӧ��Ͱλ
  size_type __n = _M_bkt_num(__obj);
  _Node* __first = _M_buckets[__n];

  // ����Ͱλ��Ԫ����ͼ�ҵ�ֵ������ҵ��ͷ���
  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next)
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj)))
      return __cur->_M_val;
  // �����½ڵ�
  _Node* __tmp = _M_new_node(__obj);
  // �ҽ���ͷ��
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  // ����Ԫ����Ŀ
  ++_M_num_elements;
  return __tmp->_M_val;
}

// �ҵ�����ȵ�����[��һ������key, ��һ��������key)
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
pair<typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator,
     typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator> 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::equal_range(const key_type& __key)
{
  typedef pair<iterator, iterator> _Pii;
  // �õ�Ͱλ����
  const size_type __n = _M_bkt_num_key(__key);
  // ����Ͱ�����Ԫ��
  for (_Node* __first = _M_buckets[__n]; __first; __first = __first->_M_next)
	  // ���
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
		// ����ȵ�Ԫ�ؿ�ʼ���ҵ���һ������ȵ�Ԫ��
      for (_Node* __cur = __first->_M_next; __cur; __cur = __cur->_M_next)
        if (!_M_equals(_M_get_key(__cur->_M_val), __key))
          return _Pii(iterator(__first, this), iterator(__cur, this));
	  // ���__nͰλ���Ĳ��ּ�ֵ����ͬ���Ҳ�����ֵ��ͬ�ģ����Һ���ǿ�Ͱλ�ĵ�һ��
      for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m)
        if (_M_buckets[__m])
          return _Pii(iterator(__first, this),
                     iterator(_M_buckets[__m], this));
	  // �������û�ҵ����Ǿͽ�β����end
      return _Pii(iterator(__first, this), end());
    }
  // ���ԭ��hash tableû����������Ǿͷ���end(), end()
  return _Pii(end(), end());
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
pair<typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::const_iterator, 
     typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::const_iterator> 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::equal_range(const key_type& __key) const
{
  typedef pair<const_iterator, const_iterator> _Pii;
  // �õ�Ͱλ����
  const size_type __n = _M_bkt_num_key(__key);
  // ����Ͱ�����Ԫ��
  for (const _Node* __first = _M_buckets[__n] ;
       __first; 
       __first = __first->_M_next) {
	// ���
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      for (const _Node* __cur = __first->_M_next;
           __cur;
           __cur = __cur->_M_next)
		// ����ȵ�Ԫ�ؿ�ʼ���ҵ���һ������ȵ�Ԫ��
        if (!_M_equals(_M_get_key(__cur->_M_val), __key))
          return _Pii(const_iterator(__first, this),
                      const_iterator(__cur, this));
	  // ���__nͰλ���Ĳ��ּ�ֵ����ͬ���Ҳ�����ֵ��ͬ�ģ����Һ���ǿ�Ͱλ�ĵ�һ��
      for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m)
        if (_M_buckets[__m])
          return _Pii(const_iterator(__first, this),
                      const_iterator(_M_buckets[__m], this));
	  // �������û�ҵ����Ǿͽ�β����end
      return _Pii(const_iterator(__first, this), end());
    }
  }
  // ���ԭ��hash tableû����������Ǿͷ���end(), end()
  return _Pii(end(), end());
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::size_type 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::erase(const key_type& __key)
{
  // �õ�Ͱλ����
  const size_type __n = _M_bkt_num_key(__key);
  _Node* __first = _M_buckets[__n];
  size_type __erased = 0;

  if (__first) {
    _Node* __cur = __first;
    _Node* __next = __cur->_M_next;
	// ע�⣬�������׽ڵ�
    while (__next) {
		// ������
      if (_M_equals(_M_get_key(__next->_M_val), __key)) {
		  // ��next�ڵ��������ɾ��
        __cur->_M_next = __next->_M_next;
		// ���ٽڵ�
        _M_delete_node(__next);
		// �õ���һ���ڵ�
        __next = __cur->_M_next;
		// ����ɾ���ڵ����
        ++__erased;
		// ����Ԫ������
        --_M_num_elements;
      }
      else {
        __cur = __next;
        __next = __cur->_M_next;
      }
    }
	// �����һ���ڵ�
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
		// ��__first�ڵ��������ɾ��
      _M_buckets[__n] = __first->_M_next;
	  // ���ٽڵ�
      _M_delete_node(__first);
	  // ����ɾ���ڵ����
      ++__erased;
	  // ����Ԫ������
      --_M_num_elements;
    }
  }
  return __erased;
}

// ɾ��ָ����������Ӧ�Ľڵ�
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::erase(const iterator& __it)
{
  _Node* __p = __it._M_cur;
  if (__p) {
	  // ����Ͱλ����
    const size_type __n = _M_bkt_num(__p->_M_val);
    _Node* __cur = _M_buckets[__n];
	// �Ƿ��׽ڵ�
    if (__cur == __p) {
		// ���׽ڵ�
      _M_buckets[__n] = __cur->_M_next;
	  // �����׽ڵ�
      _M_delete_node(__cur);
      --_M_num_elements;
    }
    else {
		// ����Ͱλ�������ڵ�
      _Node* __next = __cur->_M_next;
      while (__next) {
		  // �ҵ��ڵ�
        if (__next == __p) {
			// ���ҵ��Ľڵ��������ɾ��
          __cur->_M_next = __next->_M_next;
		  // ���ٽڵ�
          _M_delete_node(__next);
		  // Ԫ����Ŀ����
          --_M_num_elements;
          break;
        }
        else {
          __cur = __next;
          __next = __cur->_M_next;
        }
      }
    }
  }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::erase(iterator __first, iterator __last)
{
	// ����__first��������Ͱλ����
  size_type __f_bucket = __first._M_cur ? 
    _M_bkt_num(__first._M_cur->_M_val) : _M_buckets.size();
  // ����__last��������Ͱλ����
  size_type __l_bucket = __last._M_cur ? 
    _M_bkt_num(__last._M_cur->_M_val) : _M_buckets.size();

  // û��Ҫɾ���ģ��Լ�����
  if (__first._M_cur == __last._M_cur)
    return;
  // ͬһ��Ͱ
  else if (__f_bucket == __l_bucket)
    _M_erase_bucket(__f_bucket, __first._M_cur, __last._M_cur);
  else {
	  // ɾ����һ��Ͱ
    _M_erase_bucket(__f_bucket, __first._M_cur, 0);
	  // ɾ�����һ��Ͱ�����������Ͱ
    for (size_type __n = __f_bucket + 1; __n < __l_bucket; ++__n)
      _M_erase_bucket(__n, 0);
	// ɾ�����һ��Ͱ
    if (__l_bucket != _M_buckets.size())
      _M_erase_bucket(__l_bucket, __last._M_cur);
  }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
inline void
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::erase(const_iterator __first,
                                             const_iterator __last)
{
  erase(iterator(const_cast<_Node*>(__first._M_cur),
                 const_cast<hashtable*>(__first._M_ht)),
        iterator(const_cast<_Node*>(__last._M_cur),
                 const_cast<hashtable*>(__last._M_ht)));
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
inline void
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::erase(const const_iterator& __it)
{
  erase(iterator(const_cast<_Node*>(__it._M_cur),
                 const_cast<hashtable*>(__it._M_ht)));
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::resize(size_type __num_elements_hint)
{
  // �õ�Ͱλ��С
  const size_type __old_n = _M_buckets.size();
  // Ԫ����Ŀ����Ͱλ��
  if (__num_elements_hint > __old_n) {
	  // �õ���һ��Ͱλ����չ�ߴ�
    const size_type __n = _M_next_size(__num_elements_hint);
	// �����Ҫ��չ
    if (__n > __old_n) {
		// ����һ���µ�Ͱλ����
      vector<_Node*, _All> __tmp(__n, (_Node*)(0),
                                 _M_buckets.get_allocator());
      __STL_TRY {
		  // �����ϵ�Ͱλ
        for (size_type __bucket = 0; __bucket < __old_n; ++__bucket) {
          _Node* __first = _M_buckets[__bucket];
		  // ���Ͱλ��Ϊ��
          while (__first) {
			// ���¼���Ͱλ����
            size_type __new_bucket = _M_bkt_num(__first->_M_val, __n);
			// ��__first�ڵ��Ͱλ������
            _M_buckets[__bucket] = __first->_M_next;
			// ��__first�ҽ����µ�Ͱλ��
            __first->_M_next = __tmp[__new_bucket];
            __tmp[__new_bucket] = __first;
			// __first��Ϊ��һ��
            __first = _M_buckets[__bucket];          
          }
        }
		// �����ϵĺ��µ�
        _M_buckets.swap(__tmp);
      }
#         ifdef __STL_USE_EXCEPTIONS
      catch(...) {
        for (size_type __bucket = 0; __bucket < __tmp.size(); ++__bucket) {
          while (__tmp[__bucket]) {
            _Node* __next = __tmp[__bucket]->_M_next;
            _M_delete_node(__tmp[__bucket]);
            __tmp[__bucket] = __next;
          }
        }
        throw;
      }
#         endif /* __STL_USE_EXCEPTIONS */
    }
  }
}
// ɾ��Ͱλ����__n��[__first, __last)��Ԫ��
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::_M_erase_bucket(const size_type __n, _Node* __first, _Node* __last)
{
	// �õ�Ͱλ��Ӧ���׽ڵ�
  _Node* __cur = _M_buckets[__n];
  // �Ƿ���׽ڵ㿪ʼ
  if (__cur == __first)
    _M_erase_bucket(__n, __last);
  else {
    _Node* __next;
	// ���ҵ�__first��Ӧ�ڵ�
    for (__next = __cur->_M_next; 
         __next != __first; 
         __cur = __next, __next = __cur->_M_next)
      ;
	// ���ҵ��ĵ�һ���ڵ㿪ʼɾ��
    while (__next != __last) {
      __cur->_M_next = __next->_M_next;
      _M_delete_node(__next);
      __next = __cur->_M_next;
      --_M_num_elements;
    }
  }
}

// ɾ��Ͱλ����__n��__last�ڵ�֮ǰ������Ԫ��
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::_M_erase_bucket(const size_type __n, _Node* __last)
{
  _Node* __cur = _M_buckets[__n];
  // ��ͷɾ����__last
  while (__cur != __last) {
    _Node* __next = __cur->_M_next;
	// ���ٽڵ�
    _M_delete_node(__cur);
    __cur = __next;
    _M_buckets[__n] = __cur;
    --_M_num_elements;
  }
}

// ������еĽڵ�
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::clear()
{
	// �������е�Ͱλ
  for (size_type __i = 0; __i < _M_buckets.size(); ++__i) {
    _Node* __cur = _M_buckets[__i];
	// ����Ͱλ�����еĽڵ�
    while (__cur != 0) {
      _Node* __next = __cur->_M_next;
      _M_delete_node(__cur);
      __cur = __next;
    }
	// ��Ͱλ�׵�ַ���
    _M_buckets[__i] = 0;
  }
  // ��Ԫ����Ŀ���
  _M_num_elements = 0;
}

    
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::_M_copy_from(const hashtable& __ht)
{
	// ���Ͱλ����
  _M_buckets.clear();
  // Ԥ����Ӧ���ڴ�
  _M_buckets.reserve(__ht._M_buckets.size());
  // ������ȥ
  _M_buckets.insert(_M_buckets.end(), __ht._M_buckets.size(), (_Node*) 0);
  __STL_TRY {
	  // ����Ͱλ����
    for (size_type __i = 0; __i < __ht._M_buckets.size(); ++__i) {
		// ����Ͱλ��һ���ڵ�
      const _Node* __cur = __ht._M_buckets[__i];
      if (__cur) {
        _Node* __copy = _M_new_node(__cur->_M_val);
        _M_buckets[__i] = __copy;
		// ����Ͱ�����Ԫ��
        for (_Node* __next = __cur->_M_next; 
             __next; 
             __cur = __next, __next = __cur->_M_next) {
          __copy->_M_next = _M_new_node(__next->_M_val);
          __copy = __copy->_M_next;
        }
      }
    }
	// ����Ԫ����Ŀ
    _M_num_elements = __ht._M_num_elements;
  }
  __STL_UNWIND(clear());
}

__STL_END_NAMESPACE

#endif /* __SGI_STL_INTERNAL_HASHTABLE_H */

// Local Variables:
// mode:C++
// End:
