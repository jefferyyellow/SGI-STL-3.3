/*
 *
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
 *
 */

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_STL_INTERNAL_TREE_H
#define __SGI_STL_INTERNAL_TREE_H

/*

Red-black tree class, designed for use in implementing STL
associative containers (set, multiset, map, and multimap). The
insertion and deletion algorithms are based on those in Cormen,
Leiserson, and Rivest, Introduction to Algorithms (MIT Press, 1990),
except that

(1) the header cell is maintained with links not only to the root
but also to the leftmost node of the tree, to enable constant time
begin(), and to the rightmost node of the tree, to enable linear time
performance when used with the generic set algorithms (set_union,
etc.);

(2) when a node being deleted has two children its successor node is
relinked into its place, rather than copied, so that the only
iterators invalidated are those referring to the deleted node.

*/

#include <stl_algobase.h>
#include <stl_alloc.h>
#include <stl_construct.h>
#include <stl_function.h>

__STL_BEGIN_NAMESPACE 

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1375
#endif

typedef bool _Rb_tree_Color_type;
const _Rb_tree_Color_type _S_rb_tree_red = false;			// 红色为false
const _Rb_tree_Color_type _S_rb_tree_black = true;			// 黑色为true

struct _Rb_tree_node_base
{
  typedef _Rb_tree_Color_type _Color_type;
  typedef _Rb_tree_node_base* _Base_ptr;

  _Color_type _M_color;			// 颜色
  _Base_ptr _M_parent;			// 父节点
  _Base_ptr _M_left;			// 左子节点
  _Base_ptr _M_right;			// 右子节点

  // 找到最小的值
  static _Base_ptr _S_minimum(_Base_ptr __x)
  {
	  // 一直往左走
    while (__x->_M_left != 0) __x = __x->_M_left;
    return __x;
  }

  // 找到最大的值
  static _Base_ptr _S_maximum(_Base_ptr __x)
  {
	  // 一直往右走
    while (__x->_M_right != 0) __x = __x->_M_right;
    return __x;
  }
};
// 红黑树节点
template <class _Value>
struct _Rb_tree_node : public _Rb_tree_node_base
{
  typedef _Rb_tree_node<_Value>* _Link_type;
  _Value _M_value_field;
};


struct _Rb_tree_base_iterator
{
  typedef _Rb_tree_node_base::_Base_ptr _Base_ptr;
  typedef bidirectional_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;
  _Base_ptr _M_node;

  void _M_increment()
  {
	  // 中序遍历的方式，找到右子树的最左节点
    if (_M_node->_M_right != 0) {
		// 右子树
      _M_node = _M_node->_M_right;
	  // 最左节点
      while (_M_node->_M_left != 0)
        _M_node = _M_node->_M_left;
    }
    else {
		// 找到父节点
      _Base_ptr __y = _M_node->_M_parent;
	  // 如果节点上父节点的右子节点，一直回溯，一直到节点不是右子节点为止
      while (_M_node == __y->_M_right) {
        _M_node = __y;
        __y = __y->_M_parent;
      }
	  // 如果此时的右子节点不等于父节点，那就是找到整个树里面最小的大于原来节点的节点
      if (_M_node->_M_right != __y)
        _M_node = __y;
    }
  }

  void _M_decrement()
  {
	  // 如果是红节点，且节点的父亲的父亲等于自己
    if (_M_node->_M_color == _S_rb_tree_red &&
        _M_node->_M_parent->_M_parent == _M_node)
      _M_node = _M_node->_M_right;
	// 如果有左子树，找到左子树的最有节点
    else if (_M_node->_M_left != 0) {
      _Base_ptr __y = _M_node->_M_left;
      while (__y->_M_right != 0)
        __y = __y->_M_right;
      _M_node = __y;
    }
    else {
		// 如果父节点一直是左节点，那就一直回溯，一直到节点不是左子节点为止
      _Base_ptr __y = _M_node->_M_parent;
      while (_M_node == __y->_M_left) {
        _M_node = __y;
        __y = __y->_M_parent;
      }
	  // 得到那个不是左子节点的父节点
      _M_node = __y;
    }
  }
};

template <class _Value, class _Ref, class _Ptr>
struct _Rb_tree_iterator : public _Rb_tree_base_iterator
{
  typedef _Value value_type;
  typedef _Ref reference;
  typedef _Ptr pointer;
  typedef _Rb_tree_iterator<_Value, _Value&, _Value*>             
    iterator;
  typedef _Rb_tree_iterator<_Value, const _Value&, const _Value*> 
    const_iterator;
  typedef _Rb_tree_iterator<_Value, _Ref, _Ptr>                   
    _Self;
  typedef _Rb_tree_node<_Value>* _Link_type;

  _Rb_tree_iterator() {}
  _Rb_tree_iterator(_Link_type __x) { _M_node = __x; }
  _Rb_tree_iterator(const iterator& __it) { _M_node = __it._M_node; }

  reference operator*() const { return _Link_type(_M_node)->_M_value_field; }
#ifndef __SGI_STL_NO_ARROW_OPERATOR
  pointer operator->() const { return &(operator*()); }
#endif /* __SGI_STL_NO_ARROW_OPERATOR */
  // 直接调用父类的_M_increment函数
  _Self& operator++() { _M_increment(); return *this; }
  _Self operator++(int) {
    _Self __tmp = *this;
    _M_increment();
    return __tmp;
  }
    // 直接调用父类的_M_decrement函数
  _Self& operator--() { _M_decrement(); return *this; }
  _Self operator--(int) {
    _Self __tmp = *this;
    _M_decrement();
    return __tmp;
  }
};

inline bool operator==(const _Rb_tree_base_iterator& __x,
                       const _Rb_tree_base_iterator& __y) {
  return __x._M_node == __y._M_node;
}

inline bool operator!=(const _Rb_tree_base_iterator& __x,
                       const _Rb_tree_base_iterator& __y) {
  return __x._M_node != __y._M_node;
}

#ifndef __STL_CLASS_PARTIAL_SPECIALIZATION

inline bidirectional_iterator_tag
iterator_category(const _Rb_tree_base_iterator&) {
  return bidirectional_iterator_tag();
}

inline _Rb_tree_base_iterator::difference_type*
distance_type(const _Rb_tree_base_iterator&) {
  return (_Rb_tree_base_iterator::difference_type*) 0;
}

template <class _Value, class _Ref, class _Ptr>
inline _Value* value_type(const _Rb_tree_iterator<_Value, _Ref, _Ptr>&) {
  return (_Value*) 0;
}

#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

// 左旋，x是旋转子树的根节点，root是整棵树的根节点
// 左旋就是将右子节点提升为旋转子树根节点，然后把右子节点的左子节点挂在当前节点的右子节点上
inline void 
_Rb_tree_rotate_left(_Rb_tree_node_base* __x, _Rb_tree_node_base*& __root)
{
	// 右子节点，最终会提升为旋转子树根节点的节点
  _Rb_tree_node_base* __y = __x->_M_right;
  // 把右子节点的左子节点挂在当前节点的右子节点上
  __x->_M_right = __y->_M_left;
  // 原来的节点的父节点是_y，现在改成_x
  if (__y->_M_left !=0)
    __y->_M_left->_M_parent = __x;
  // _y的父节点以前是__x，现在改成__x的父节点
  __y->_M_parent = __x->_M_parent;

  // 如果__x以前是根节点，还需要修改根节点
  if (__x == __root)
    __root = __y;
  // 如果__x是__x父节点的左子节点，那得把父节点得左子节点改成y
  else if (__x == __x->_M_parent->_M_left)
    __x->_M_parent->_M_left = __y;
   // 如果__x是__x父节点的右子节点，那得把父节点得右子节点改成y
  else
    __x->_M_parent->_M_right = __y;
  // ___x变成了__y的左子节点
  __y->_M_left = __x;
  // __x的父节点变成了提升为根节点的__y
  __x->_M_parent = __y;
}

// 右旋，x是旋转子树的根节点，root是整棵树的根节点
// 右旋就是将左子节点提升为旋转子树根节点，然后把左子节点的右子节点挂在当前节点的左子节点上
inline void 
_Rb_tree_rotate_right(_Rb_tree_node_base* __x, _Rb_tree_node_base*& __root)
{
	// 左子节点，最终会提升为旋转子树根节点的节点
  _Rb_tree_node_base* __y = __x->_M_left;
  // 把左子节点的右子节点挂在当前节点的左子节点上
  __x->_M_left = __y->_M_right;

  // 原来的节点的父节点是_y，现在改成_x
  if (__y->_M_right != 0)
    __y->_M_right->_M_parent = __x;
  // _y的父节点以前是__x，现在改成__x的父节点
  __y->_M_parent = __x->_M_parent;

  // 如果__x以前是根节点，还需要修改根节点
  if (__x == __root)
    __root = __y;
  // 如果__x是__x父节点的右子节点，那得把父节点得右子节点改成y
  else if (__x == __x->_M_parent->_M_right)
    __x->_M_parent->_M_right = __y;
  // 如果__x是__x父节点的左子节点，那得把父节点得左子节点改成y
  else
    __x->_M_parent->_M_left = __y;
  // __x变成了__y的右子节点
  __y->_M_right = __x;
  // __x的父节点变成了提升为根节点的__y
  __x->_M_parent = __y;
}

// 红黑树的重新平衡
inline void 
_Rb_tree_rebalance(_Rb_tree_node_base* __x, _Rb_tree_node_base*& __root)
{
	// x的节点设置为红色
  __x->_M_color = _S_rb_tree_red;
  // 不是根节点，并且父节点的颜色为红色
  while (__x != __root && __x->_M_parent->_M_color == _S_rb_tree_red) {
	  // 父节点是左子节点
    if (__x->_M_parent == __x->_M_parent->_M_parent->_M_left) {
		// 找到叔叔节点
      _Rb_tree_node_base* __y = __x->_M_parent->_M_parent->_M_right;
	  // 如果叔叔存在并且是红色
      if (__y && __y->_M_color == _S_rb_tree_red) {
		  // 将父亲的颜色改成黑色
        __x->_M_parent->_M_color = _S_rb_tree_black;
		// 叔叔的颜色改成黑色
        __y->_M_color = _S_rb_tree_black;
		// 祖父的颜色改成红色
        __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
		// 将当前节点的祖父改成当前节点，继续循环
        __x = __x->_M_parent->_M_parent;
      }
	  // 如果叔叔不存在或者是黑色
      else {
		  // 当前节点是右节点，（将当前节点的父节点为子树左旋，就会变成左节点和父节点都是红色的情况）
        if (__x == __x->_M_parent->_M_right) {
			// 将父亲当成当前节点
          __x = __x->_M_parent;
		  // 以父亲为子树根节点，左旋
          _Rb_tree_rotate_left(__x, __root);
        }
		// 将父节点改成黑色
        __x->_M_parent->_M_color = _S_rb_tree_black;
		// 祖父节点改成红色
        __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
		// 再右旋
        _Rb_tree_rotate_right(__x->_M_parent->_M_parent, __root);
      }
    }
	 // 父节点是右子节点
    else {
		// 找到叔叔节点
      _Rb_tree_node_base* __y = __x->_M_parent->_M_parent->_M_left;
	  // 如果叔叔存在并且是红色
      if (__y && __y->_M_color == _S_rb_tree_red) {
		// 将父亲的颜色改成黑色
        __x->_M_parent->_M_color = _S_rb_tree_black;
		// 将叔叔的颜色改成黑色
        __y->_M_color = _S_rb_tree_black;
		// 将祖父的颜色改成红色
        __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
		// 将当前节点的祖父改成当前节点，继续循环
        __x = __x->_M_parent->_M_parent;
      }
	   // 如果叔叔不存在或者是黑色
      else {
		  // 当前节点是左子节点，（将当前节点的父节点为子树右旋，就会变成右节点和父节点都是红色的情况）
        if (__x == __x->_M_parent->_M_left) {
			// 当前节点的父节点当前节点
          __x = __x->_M_parent;
		  // // 以父亲为子树根节点，右旋
          _Rb_tree_rotate_right(__x, __root);
        }
		// 将父节点改成黑色
        __x->_M_parent->_M_color = _S_rb_tree_black;
		// 祖父节点改成红色
        __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
		// 再左旋
        _Rb_tree_rotate_left(__x->_M_parent->_M_parent, __root);
      }
    }
  }

  // 将根节点的颜色置成黑色
  __root->_M_color = _S_rb_tree_black;
}

// __z：是要删除的点，
// __root：根节点
// __leftmost：左子树的根节点
// __rightmost：右子树的根节点
inline _Rb_tree_node_base*
_Rb_tree_rebalance_for_erase(_Rb_tree_node_base* __z,
                             _Rb_tree_node_base*& __root,
                             _Rb_tree_node_base*& __leftmost,
                             _Rb_tree_node_base*& __rightmost)
{

  _Rb_tree_node_base* __y = __z;
  _Rb_tree_node_base* __x = 0;
  // x的父节点
  _Rb_tree_node_base* __x_parent = 0;
  // 如果左子节点为空，则为右子节点
  if (__y->_M_left == 0)     // __z has at most one non-null child. y == z.
    __x = __y->_M_right;     // __x might be null.
  else
	 // 如果右子节点为空，则为左子节点
    if (__y->_M_right == 0)  // __z has exactly one non-null child. y == z.
      __x = __y->_M_left;    // __x is not null.
    else {                   // __z has two non-null children.  Set __y to
		// __y的大于__z的最小节点
      __y = __y->_M_right;   //   __z's successor.  __x might be null.
      while (__y->_M_left != 0)
        __y = __y->_M_left;
	  // __x是接替者的右子树
      __x = __y->_M_right;
    }
  if (__y != __z) {          // relink y in place of z.  y is z's successor
	  // 将__z的左子节点链接上__y
    __z->_M_left->_M_parent = __y; 
    __y->_M_left = __z->_M_left;

	// 如果__y不是__z的右子节点
    if (__y != __z->_M_right) {
      __x_parent = __y->_M_parent;
	  // 将__x挂在__y的父节点的左节点上
      if (__x) __x->_M_parent = __y->_M_parent;
      __y->_M_parent->_M_left = __x;      // __y must be a child of _M_left

	  // 将__z的右子节点链接上__y
      __y->_M_right = __z->_M_right;
      __z->_M_right->_M_parent = __y;
    }
	// __y是__z的右子节点
    else
		// __y是__x的父节点
      __x_parent = __y;  
	// 如果以前__z是根节点，现在__y就是根节点了
    if (__root == __z)
      __root = __y;
	// z是原来的左子节点
    else if (__z->_M_parent->_M_left == __z)
      __z->_M_parent->_M_left = __y;
	// z是原来的右子节点
    else 
      __z->_M_parent->_M_right = __y;
    __y->_M_parent = __z->_M_parent;
    __STD::swap(__y->_M_color, __z->_M_color);
    __y = __z;
    // __y now points to node to be actually deleted
  }
   // __y == __z， __x是取代的那个节点
  else {                       
    __x_parent = __y->_M_parent;
    if (__x) __x->_M_parent = __y->_M_parent;   
    if (__root == __z)
      __root = __x;
    else 
		// z是父亲的左子节点
      if (__z->_M_parent->_M_left == __z)
        __z->_M_parent->_M_left = __x;
		// z是父亲的右子节点
      else
        __z->_M_parent->_M_right = __x;
	// 最左节点为删除节点，需要更新
    if (__leftmost == __z) 
		// 如果没有右子节点，那父节点就是最左的了
      if (__z->_M_right == 0)        // __z->_M_left must be null also
        __leftmost = __z->_M_parent;
    // makes __leftmost == _M_header if __z == __root
      else
        __leftmost = _Rb_tree_node_base::_S_minimum(__x);
	// 最右节点为删除节点
    if (__rightmost == __z)  
		// 如果没有左子节点，那父节点就是最右的了
      if (__z->_M_left == 0)         // __z->_M_right must be null also
        __rightmost = __z->_M_parent;  
    // makes __rightmost == _M_header if __z == __root
      else                      // __x == __z->_M_left
        __rightmost = _Rb_tree_node_base::_S_maximum(__x);
  }
  // 如果删除的节点不是红的（那意味着删除的节点是黑的）
  // y节点是只有一个儿子的节点，如果颜色是红色，就不需要处理，直接替换
  if (__y->_M_color != _S_rb_tree_red) { 
    while (__x != __root && (__x == 0 || __x->_M_color == _S_rb_tree_black))
		// 接替节点是左节点
      if (__x == __x_parent->_M_left) {
		  // 如果兄弟节点是红色，也会变成黑色，所以，后面的兄弟节点肯定是黑色
		  // 找到接替节点的兄弟节点
        _Rb_tree_node_base* __w = __x_parent->_M_right;
		// 如果兄弟节点是红色
        if (__w->_M_color == _S_rb_tree_red) {
			// 将兄弟节点置成黑色
          __w->_M_color = _S_rb_tree_black;
		  // 父节点置成红色
          __x_parent->_M_color = _S_rb_tree_red;
		  // 左旋
          _Rb_tree_rotate_left(__x_parent, __root);
          __w = __x_parent->_M_right;
        }
		// 到这里的时候兄弟节点肯定是黑色了
		// （兄弟节点的左子节点为空或者颜色为黑色）且（兄弟节点的右子节点为空或者颜色为黑色）
        if ((__w->_M_left == 0 || 
             __w->_M_left->_M_color == _S_rb_tree_black) &&
            (__w->_M_right == 0 || 
             __w->_M_right->_M_color == _S_rb_tree_black)) {
			// 将兄弟节点置成红色
          __w->_M_color = _S_rb_tree_red;
		  // 直接将__x替代了就好
          __x = __x_parent;
          __x_parent = __x_parent->_M_parent;
        } else {
			// 如果兄弟的右子节点为空或者为黑色节点
          if (__w->_M_right == 0 || 
              __w->_M_right->_M_color == _S_rb_tree_black) {
			  // 如果兄弟有左子节点，将左子节点的颜色置为黑色
            if (__w->_M_left) __w->_M_left->_M_color = _S_rb_tree_black;
			// 兄弟节点的颜色为红色
            __w->_M_color = _S_rb_tree_red;
			// 右旋
            _Rb_tree_rotate_right(__w, __root);
			// 兄弟节点变成了替代节点父节点的右节点
            __w = __x_parent->_M_right;
          }
		  // 父节点的颜色赋给兄弟节点的颜色
          __w->_M_color = __x_parent->_M_color;
		  // 父节点的颜色赋值给黑色
          __x_parent->_M_color = _S_rb_tree_black;
		  // 如果兄弟有右子节点，将兄弟右子节点的颜色设置为黑色
          if (__w->_M_right) __w->_M_right->_M_color = _S_rb_tree_black;
		  // 左旋
          _Rb_tree_rotate_left(__x_parent, __root);
          break;
        }
		// 接替节点是右节点
      } else {                  // same as above, with _M_right <-> _M_left.
		  // 找到接替节点的兄弟节点
        _Rb_tree_node_base* __w = __x_parent->_M_left;
		// 如果兄弟节点是红色
        if (__w->_M_color == _S_rb_tree_red) {
			// 把兄弟节点的颜色置为黑色
          __w->_M_color = _S_rb_tree_black;
		  // 父节点的颜色置为红色
          __x_parent->_M_color = _S_rb_tree_red;
		  // 右旋
          _Rb_tree_rotate_right(__x_parent, __root);
          __w = __x_parent->_M_left;
        }
		// （兄弟节点的右子节点为空或者颜色为黑色）且（兄弟节点的左子节点为空或者颜色为黑色）
        if ((__w->_M_right == 0 || 
             __w->_M_right->_M_color == _S_rb_tree_black) &&
            (__w->_M_left == 0 || 
             __w->_M_left->_M_color == _S_rb_tree_black)) {
			// 将兄弟节点置成红色
          __w->_M_color = _S_rb_tree_red;
		  // 直接将__x替代了就好
          __x = __x_parent;
          __x_parent = __x_parent->_M_parent;
        } else {
			// 如果兄弟的左子节点为空或者为黑色节点
          if (__w->_M_left == 0 || 
              __w->_M_left->_M_color == _S_rb_tree_black) {
			  // 兄弟的右子节点不为空，将兄弟的右子节点颜色置为黑色
            if (__w->_M_right) __w->_M_right->_M_color = _S_rb_tree_black;
			// 兄弟的节点颜色置为红色
            __w->_M_color = _S_rb_tree_red;
			// 左旋
            _Rb_tree_rotate_left(__w, __root);
			// 兄弟节点变成了替代节点父节点的左节点
            __w = __x_parent->_M_left;
          }
		  // 父节点的颜色赋给兄弟节点的颜色
          __w->_M_color = __x_parent->_M_color;
		  // 父亲节点的颜色为黑色
          __x_parent->_M_color = _S_rb_tree_black;
		  // 兄弟节点的左子节点颜色为黑色
          if (__w->_M_left) __w->_M_left->_M_color = _S_rb_tree_black;
		  // 右旋
          _Rb_tree_rotate_right(__x_parent, __root);
          break;
        }
      }
    if (__x) __x->_M_color = _S_rb_tree_black;
  }
  return __y;
}

// Base class to encapsulate the differences between old SGI-style
// allocators and standard-conforming allocators.  In order to avoid
// having an empty base class, we arbitrarily move one of rb_tree's
// data members into the base class.
// 封装了老的SGI风格的分配器和符号标准的分配器之间差异的基类。为了防止有一个空基类，我们把了rb_tree的
// 数据成员移动到基类里面来。
#ifdef __STL_USE_STD_ALLOCATORS

// _Base for general standard-conforming allocators.
// 符合标准的分配器的基类
template <class _Tp, class _Alloc, bool _S_instanceless>
class _Rb_tree_alloc_base {
public:
  typedef typename _Alloc_traits<_Tp, _Alloc>::allocator_type allocator_type;
  allocator_type get_allocator() const { return _M_node_allocator; }

  _Rb_tree_alloc_base(const allocator_type& __a)
    : _M_node_allocator(__a), _M_header(0) {}

protected:
  typename _Alloc_traits<_Rb_tree_node<_Tp>, _Alloc>::allocator_type
           _M_node_allocator;
  _Rb_tree_node<_Tp>* _M_header;

  // 分配节点
  _Rb_tree_node<_Tp>* _M_get_node() 
    { return _M_node_allocator.allocate(1); }

  // 释放节点
  void _M_put_node(_Rb_tree_node<_Tp>* __p) 
    { _M_node_allocator.deallocate(__p, 1); }
};

// Specialization for instanceless allocators.
// 无实例分配器的特化版本
template <class _Tp, class _Alloc>
class _Rb_tree_alloc_base<_Tp, _Alloc, true> {
public:
  typedef typename _Alloc_traits<_Tp, _Alloc>::allocator_type allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Rb_tree_alloc_base(const allocator_type&) : _M_header(0) {}

protected:
  _Rb_tree_node<_Tp>* _M_header;

  typedef typename _Alloc_traits<_Rb_tree_node<_Tp>, _Alloc>::_Alloc_type
          _Alloc_type;

    // 分配节点
  _Rb_tree_node<_Tp>* _M_get_node()
    { return _Alloc_type::allocate(1); }

  // 释放节点
  void _M_put_node(_Rb_tree_node<_Tp>* __p)
    { _Alloc_type::deallocate(__p, 1); }
};

template <class _Tp, class _Alloc>
struct _Rb_tree_base
  : public _Rb_tree_alloc_base<_Tp, _Alloc,
                               _Alloc_traits<_Tp, _Alloc>::_S_instanceless>
{
  typedef _Rb_tree_alloc_base<_Tp, _Alloc,
                              _Alloc_traits<_Tp, _Alloc>::_S_instanceless>
          _Base;
  typedef typename _Base::allocator_type allocator_type;

  // 初始化_M_header节点
  _Rb_tree_base(const allocator_type& __a) 
    : _Base(__a) { _M_header = _M_get_node(); }

  // 析构函数，析构_M_header节点
  ~_Rb_tree_base() { _M_put_node(_M_header); }

};

#else /* __STL_USE_STD_ALLOCATORS */

template <class _Tp, class _Alloc>
struct _Rb_tree_base
{
  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Rb_tree_base(const allocator_type&) 
    : _M_header(0) { _M_header = _M_get_node(); }
  ~_Rb_tree_base() { _M_put_node(_M_header); }

protected:
  _Rb_tree_node<_Tp>* _M_header;

  typedef simple_alloc<_Rb_tree_node<_Tp>, _Alloc> _Alloc_type;

  _Rb_tree_node<_Tp>* _M_get_node()
    { return _Alloc_type::allocate(1); }

  void _M_put_node(_Rb_tree_node<_Tp>* __p)
    { _Alloc_type::deallocate(__p, 1); }
};

#endif /* __STL_USE_STD_ALLOCATORS */

template <class _Key, class _Value, class _KeyOfValue, class _Compare,
          class _Alloc = __STL_DEFAULT_ALLOCATOR(_Value) >
class _Rb_tree : protected _Rb_tree_base<_Value, _Alloc> {
  typedef _Rb_tree_base<_Value, _Alloc> _Base;
protected:
  typedef _Rb_tree_node_base* _Base_ptr;
  typedef _Rb_tree_node<_Value> _Rb_tree_node;
  typedef _Rb_tree_Color_type _Color_type;
public:
  typedef _Key key_type;
  typedef _Value value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef _Rb_tree_node* _Link_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef typename _Base::allocator_type allocator_type;
  allocator_type get_allocator() const { return _Base::get_allocator(); }

protected:
#ifdef __STL_USE_NAMESPACES
  using _Base::_M_get_node;
  using _Base::_M_put_node;
  using _Base::_M_header;
#endif /* __STL_USE_NAMESPACES */

protected:
	// 创建节点并且构造
  _Link_type _M_create_node(const value_type& __x)
  {
    _Link_type __tmp = _M_get_node();
    __STL_TRY {
      construct(&__tmp->_M_value_field, __x);
    }
    __STL_UNWIND(_M_put_node(__tmp));
    return __tmp;
  }
  // 克隆节点，克隆值和颜色
  _Link_type _M_clone_node(_Link_type __x)
  {
    _Link_type __tmp = _M_create_node(__x->_M_value_field);
    __tmp->_M_color = __x->_M_color;
    __tmp->_M_left = 0;
    __tmp->_M_right = 0;
    return __tmp;
  }
  // 销毁节点，析构节点和释放内存
  void destroy_node(_Link_type __p)
  {
    destroy(&__p->_M_value_field);
    _M_put_node(__p);
  }

protected:
  size_type _M_node_count; // keeps track of size of tree
  _Compare _M_key_compare;
  // 根节点
  _Link_type& _M_root() const 
    { return (_Link_type&) _M_header->_M_parent; }
  // 最左节点
  _Link_type& _M_leftmost() const 
    { return (_Link_type&) _M_header->_M_left; }
  // 最右节点
  _Link_type& _M_rightmost() const 
    { return (_Link_type&) _M_header->_M_right; }

  static _Link_type& _S_left(_Link_type __x)
    { return (_Link_type&)(__x->_M_left); }
  static _Link_type& _S_right(_Link_type __x)
    { return (_Link_type&)(__x->_M_right); }
  static _Link_type& _S_parent(_Link_type __x)
    { return (_Link_type&)(__x->_M_parent); }
  static reference _S_value(_Link_type __x)
    { return __x->_M_value_field; }
  static const _Key& _S_key(_Link_type __x)
    { return _KeyOfValue()(_S_value(__x)); }
  static _Color_type& _S_color(_Link_type __x)
    { return (_Color_type&)(__x->_M_color); }

  static _Link_type& _S_left(_Base_ptr __x)
    { return (_Link_type&)(__x->_M_left); }
  static _Link_type& _S_right(_Base_ptr __x)
    { return (_Link_type&)(__x->_M_right); }
  static _Link_type& _S_parent(_Base_ptr __x)
    { return (_Link_type&)(__x->_M_parent); }
  static reference _S_value(_Base_ptr __x)
    { return ((_Link_type)__x)->_M_value_field; }
  static const _Key& _S_key(_Base_ptr __x)
    { return _KeyOfValue()(_S_value(_Link_type(__x)));} 
  static _Color_type& _S_color(_Base_ptr __x)
    { return (_Color_type&)(_Link_type(__x)->_M_color); }
  // 最小键的节点
  static _Link_type _S_minimum(_Link_type __x) 
    { return (_Link_type)  _Rb_tree_node_base::_S_minimum(__x); }
  // 最大键的节点
  static _Link_type _S_maximum(_Link_type __x)
    { return (_Link_type) _Rb_tree_node_base::_S_maximum(__x); }

public:
  typedef _Rb_tree_iterator<value_type, reference, pointer> iterator;
  typedef _Rb_tree_iterator<value_type, const_reference, const_pointer> 
          const_iterator;

#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
  typedef reverse_iterator<const_iterator> const_reverse_iterator;
  typedef reverse_iterator<iterator> reverse_iterator;
#else /* __STL_CLASS_PARTIAL_SPECIALIZATION */
  typedef reverse_bidirectional_iterator<iterator, value_type, reference,
                                         difference_type>
          reverse_iterator; 
  typedef reverse_bidirectional_iterator<const_iterator, value_type,
                                         const_reference, difference_type>
          const_reverse_iterator;
#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */ 

private:
  iterator _M_insert(_Base_ptr __x, _Base_ptr __y, const value_type& __v);
  _Link_type _M_copy(_Link_type __x, _Link_type __p);
  void _M_erase(_Link_type __x);

public:
                                // allocation/deallocation
  _Rb_tree()
    : _Base(allocator_type()), _M_node_count(0), _M_key_compare()
    { _M_empty_initialize(); }

  _Rb_tree(const _Compare& __comp)
    : _Base(allocator_type()), _M_node_count(0), _M_key_compare(__comp) 
    { _M_empty_initialize(); }

  _Rb_tree(const _Compare& __comp, const allocator_type& __a)
    : _Base(__a), _M_node_count(0), _M_key_compare(__comp) 
    { _M_empty_initialize(); }

  // 拷贝构造
  _Rb_tree(const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x) 
    : _Base(__x.get_allocator()),
      _M_node_count(0), _M_key_compare(__x._M_key_compare)
  { 
    if (__x._M_root() == 0)
      _M_empty_initialize();
    else {
		// header节点为红色
      _S_color(_M_header) = _S_rb_tree_red;
      _M_root() = _M_copy(__x._M_root(), _M_header);
      _M_leftmost() = _S_minimum(_M_root());
      _M_rightmost() = _S_maximum(_M_root());
    }
    _M_node_count = __x._M_node_count;
  }
  ~_Rb_tree() { clear(); }
  _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& 
  operator=(const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x);

private:
  void _M_empty_initialize() {
	  // _M_header的颜色是红的，用来和根节点区分
    _S_color(_M_header) = _S_rb_tree_red; // used to distinguish header from 
                                          // __root, in iterator.operator++
	// 根节点为空
    _M_root() = 0;
	// 最左,最右节点都为_M_header
    _M_leftmost() = _M_header;
    _M_rightmost() = _M_header;
  }

public:    
                                // accessors:
  _Compare key_comp() const { return _M_key_compare; }
  iterator begin() { return _M_leftmost(); }
  const_iterator begin() const { return _M_leftmost(); }
  iterator end() { return _M_header; }
  const_iterator end() const { return _M_header; }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const { 
    return const_reverse_iterator(end()); 
  }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const { 
    return const_reverse_iterator(begin());
  } 
  bool empty() const { return _M_node_count == 0; }
  size_type size() const { return _M_node_count; }
  size_type max_size() const { return size_type(-1); }

  void swap(_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __t) {
    __STD::swap(_M_header, __t._M_header);
    __STD::swap(_M_node_count, __t._M_node_count);
    __STD::swap(_M_key_compare, __t._M_key_compare);
  }
    
public:
                                // insert/erase
  pair<iterator,bool> insert_unique(const value_type& __x);
  iterator insert_equal(const value_type& __x);

  iterator insert_unique(iterator __position, const value_type& __x);
  iterator insert_equal(iterator __position, const value_type& __x);

#ifdef __STL_MEMBER_TEMPLATES  
  template <class _InputIterator>
  void insert_unique(_InputIterator __first, _InputIterator __last);
  template <class _InputIterator>
  void insert_equal(_InputIterator __first, _InputIterator __last);
#else /* __STL_MEMBER_TEMPLATES */
  void insert_unique(const_iterator __first, const_iterator __last);
  void insert_unique(const value_type* __first, const value_type* __last);
  void insert_equal(const_iterator __first, const_iterator __last);
  void insert_equal(const value_type* __first, const value_type* __last);
#endif /* __STL_MEMBER_TEMPLATES */

  void erase(iterator __position);
  size_type erase(const key_type& __x);
  void erase(iterator __first, iterator __last);
  void erase(const key_type* __first, const key_type* __last);
  void clear() {
    if (_M_node_count != 0) {
		// 删除整棵树
      _M_erase(_M_root());
      _M_leftmost() = _M_header;
      _M_root() = 0;
      _M_rightmost() = _M_header;
      _M_node_count = 0;
    }
  }      

public:
                                // set operations:
  iterator find(const key_type& __x);
  const_iterator find(const key_type& __x) const;
  size_type count(const key_type& __x) const;
  iterator lower_bound(const key_type& __x);
  const_iterator lower_bound(const key_type& __x) const;
  iterator upper_bound(const key_type& __x);
  const_iterator upper_bound(const key_type& __x) const;
  pair<iterator,iterator> equal_range(const key_type& __x);
  pair<const_iterator, const_iterator> equal_range(const key_type& __x) const;

public:
                                // Debugging.
  bool __rb_verify() const;
};

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool 
operator==(const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
           const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y)
{
  return __x.size() == __y.size() &&
         equal(__x.begin(), __x.end(), __y.begin());
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool 
operator<(const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
          const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y)
{
  return lexicographical_compare(__x.begin(), __x.end(), 
                                 __y.begin(), __y.end());
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool 
operator!=(const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
           const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y) {
  return !(__x == __y);
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool 
operator>(const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
          const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y) {
  return __y < __x;
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool 
operator<=(const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
           const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y) {
  return !(__y < __x);
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool 
operator>=(const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
           const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y) {
  return !(__x < __y);
}


template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline void 
swap(_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
     _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y)
{
  __x.swap(__y);
}

#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */


template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::operator=(const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x)
{
  if (this != &__x) {
                                // Note that _Key may be a constant type.
    clear();
    _M_node_count = 0;
    _M_key_compare = __x._M_key_compare;     
	// 如果是空树
    if (__x._M_root() == 0) {
		// 初始化整棵树
      _M_root() = 0;
      _M_leftmost() = _M_header;
      _M_rightmost() = _M_header;
    }
    else {
		// 拷贝整棵树
      _M_root() = _M_copy(__x._M_root(), _M_header);
      _M_leftmost() = _S_minimum(_M_root());
      _M_rightmost() = _S_maximum(_M_root());
      _M_node_count = __x._M_node_count;
    }
  }
  return *this;
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::_M_insert(_Base_ptr __x_, _Base_ptr __y_, const _Value& __v)
{
  _Link_type __x = (_Link_type) __x_;
  _Link_type __y = (_Link_type) __y_;
  _Link_type __z;

  if (__y == _M_header || __x != 0 || 
      _M_key_compare(_KeyOfValue()(__v), _S_key(__y))) {
	  // 创建新的节点
    __z = _M_create_node(__v);
	// 挂接在左子树上
    _S_left(__y) = __z;               // also makes _M_leftmost() = __z 
                                      //    when __y == _M_header
	// 根节点
    if (__y == _M_header) {
      _M_root() = __z;
	  // 更新最大节点
      _M_rightmost() = __z;
    }
	// 如果__y是最小的节点
    else if (__y == _M_leftmost())
		// 更新最小节点
      _M_leftmost() = __z;   // maintain _M_leftmost() pointing to min node
  }
  else {
	  // 创建新的节点
    __z = _M_create_node(__v);
	// 挂接在右子树上
    _S_right(__y) = __z;
	// 更新最大节点
    if (__y == _M_rightmost())
      _M_rightmost() = __z;  // maintain _M_rightmost() pointing to max node
  }
  _S_parent(__z) = __y;
  _S_left(__z) = 0;
  _S_right(__z) = 0;
  _Rb_tree_rebalance(__z, _M_header->_M_parent);
  ++_M_node_count;
  return iterator(__z);
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::insert_equal(const _Value& __v)
{
  _Link_type __y = _M_header;
  _Link_type __x = _M_root();
  // 找到对应的父节点
  while (__x != 0) {
    __y = __x;
    __x = _M_key_compare(_KeyOfValue()(__v), _S_key(__x)) ? 
            _S_left(__x) : _S_right(__x);
  }
  return _M_insert(__x, __y, __v);
}

// 找到合适的位置并且插入
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
pair<typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator, 
     bool>
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::insert_unique(const _Value& __v)
{
  _Link_type __y = _M_header;
  _Link_type __x = _M_root();
  bool __comp = true;
  // 找到对应的父节点
  while (__x != 0) {
    __y = __x;
    __comp = _M_key_compare(_KeyOfValue()(__v), _S_key(__x));
	// 如果小于，遍历左边，或者遍历右边
    __x = __comp ? _S_left(__x) : _S_right(__x);
  }
  iterator __j = iterator(__y);   
  if (__comp)
    if (__j == begin())     
      return pair<iterator,bool>(_M_insert(__x, __y, __v), true);
    else
      --__j;
  // 比较是不是有键值一样的，如果没有一样的插入成功
  if (_M_key_compare(_S_key(__j._M_node), _KeyOfValue()(__v)))
    return pair<iterator,bool>(_M_insert(__x, __y, __v), true);
  // 如果有一样的插入失败
  return pair<iterator,bool>(__j, false);
}


template <class _Key, class _Val, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key, _Val, _KeyOfValue, _Compare, _Alloc>::iterator 
_Rb_tree<_Key, _Val, _KeyOfValue, _Compare, _Alloc>
  ::insert_unique(iterator __position, const _Val& __v)
{
	// 插入最左边
  if (__position._M_node == _M_header->_M_left) { // begin()
	  // 有元素的话，并且比最小值还小，就插入最左边
    if (size() > 0 && 
        _M_key_compare(_KeyOfValue()(__v), _S_key(__position._M_node)))
      return _M_insert(__position._M_node, __position._M_node, __v);
    // first argument just needs to be non-null 
    else
      return insert_unique(__v).first;
	// 
  } else if (__position._M_node == _M_header) { // end()
	  // 插入最右边
    if (_M_key_compare(_S_key(_M_rightmost()), _KeyOfValue()(__v)))
      return _M_insert(0, _M_rightmost(), __v);
    else
      return insert_unique(__v).first;
  } else {
    iterator __before = __position;
    --__before;
	// 在前一个和当前值之间
    if (_M_key_compare(_S_key(__before._M_node), _KeyOfValue()(__v)) 
        && _M_key_compare(_KeyOfValue()(__v), _S_key(__position._M_node))) {
		// 如果前一个节点的右子节点为空，挂在右边
      if (_S_right(__before._M_node) == 0)
        return _M_insert(0, __before._M_node, __v); 
      else
		  // 当前节点的左子节点为空，挂在左子节点上
        return _M_insert(__position._M_node, __position._M_node, __v);
    // first argument just needs to be non-null 
    } else
      return insert_unique(__v).first;
  }
}

template <class _Key, class _Val, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Val,_KeyOfValue,_Compare,_Alloc>::iterator 
_Rb_tree<_Key,_Val,_KeyOfValue,_Compare,_Alloc>
  ::insert_equal(iterator __position, const _Val& __v)
{
	// 最小节点
  if (__position._M_node == _M_header->_M_left) { // begin()
	  // 树大小不为0，并且__position._M_node大于等于__v
    if (size() > 0 && 
        !_M_key_compare(_S_key(__position._M_node), _KeyOfValue()(__v)))
		// 插入节点的左子节点
      return _M_insert(__position._M_node, __position._M_node, __v);
    // first argument just needs to be non-null 
    else
      return insert_equal(__v);
	// 
  } else if (__position._M_node == _M_header) {// end()
	  // __v大于等于最大值
    if (!_M_key_compare(_KeyOfValue()(__v), _S_key(_M_rightmost())))
		// 插入最大节点的最右边
      return _M_insert(0, _M_rightmost(), __v);
    else
      return insert_equal(__v);
  } else {
    iterator __before = __position;
    --__before;
	// __v大于等于__before._M_node且__position._M_node大于等于__v
    if (!_M_key_compare(_KeyOfValue()(__v), _S_key(__before._M_node))
        && !_M_key_compare(_S_key(__position._M_node), _KeyOfValue()(__v))) {
		// 如果__before右节点为空
      if (_S_right(__before._M_node) == 0)
        return _M_insert(0, __before._M_node, __v); 
      else
		  // 插入__position的左子树
        return _M_insert(__position._M_node, __position._M_node, __v);
    // first argument just needs to be non-null 
    } else
      return insert_equal(__v);
  }
}

#ifdef __STL_MEMBER_TEMPLATES  

template <class _Key, class _Val, class _KoV, class _Cmp, class _Alloc>
  template<class _II>
void _Rb_tree<_Key,_Val,_KoV,_Cmp,_Alloc>
  ::insert_equal(_II __first, _II __last)
{
  for ( ; __first != __last; ++__first)
    insert_equal(*__first);
}

template <class _Key, class _Val, class _KoV, class _Cmp, class _Alloc> 
  template<class _II>
void _Rb_tree<_Key,_Val,_KoV,_Cmp,_Alloc>
  ::insert_unique(_II __first, _II __last) {
  for ( ; __first != __last; ++__first)
    insert_unique(*__first);
}

#else /* __STL_MEMBER_TEMPLATES */

template <class _Key, class _Val, class _KoV, class _Cmp, class _Alloc>
void
_Rb_tree<_Key,_Val,_KoV,_Cmp,_Alloc>
  ::insert_equal(const _Val* __first, const _Val* __last)
{
  for ( ; __first != __last; ++__first)
    insert_equal(*__first);
}

template <class _Key, class _Val, class _KoV, class _Cmp, class _Alloc>
void
_Rb_tree<_Key,_Val,_KoV,_Cmp,_Alloc>
  ::insert_equal(const_iterator __first, const_iterator __last)
{
  for ( ; __first != __last; ++__first)
    insert_equal(*__first);
}

template <class _Key, class _Val, class _KoV, class _Cmp, class _Alloc>
void 
_Rb_tree<_Key,_Val,_KoV,_Cmp,_Alloc>
  ::insert_unique(const _Val* __first, const _Val* __last)
{
  for ( ; __first != __last; ++__first)
    insert_unique(*__first);
}

template <class _Key, class _Val, class _KoV, class _Cmp, class _Alloc>
void _Rb_tree<_Key,_Val,_KoV,_Cmp,_Alloc>
  ::insert_unique(const_iterator __first, const_iterator __last)
{
  for ( ; __first != __last; ++__first)
    insert_unique(*__first);
}

#endif /* __STL_MEMBER_TEMPLATES */
         
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline void _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::erase(iterator __position)
{
	// 删除并且删除以后重新平衡
  _Link_type __y = 
    (_Link_type) _Rb_tree_rebalance_for_erase(__position._M_node,
                                              _M_header->_M_parent,
                                              _M_header->_M_left,
                                              _M_header->_M_right);
  // 销毁节点
  destroy_node(__y);
  --_M_node_count;
}

// 删除键位__x的节点
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::size_type 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::erase(const _Key& __x)
{
  pair<iterator,iterator> __p = equal_range(__x);
  size_type __n = 0;
  distance(__p.first, __p.second, __n);
  erase(__p.first, __p.second);
  return __n;
}

// 拷贝
template <class _Key, class _Val, class _KoV, class _Compare, class _Alloc>
typename _Rb_tree<_Key, _Val, _KoV, _Compare, _Alloc>::_Link_type 
_Rb_tree<_Key,_Val,_KoV,_Compare,_Alloc>
  ::_M_copy(_Link_type __x, _Link_type __p)
{
                        // structural copy.  __x and __p must be non-null.
  _Link_type __top = _M_clone_node(__x);
  __top->_M_parent = __p;
 
  __STL_TRY {
	  // 如果有右子树
    if (__x->_M_right)
		// 递归拷贝右子树
      __top->_M_right = _M_copy(_S_right(__x), __top);
    __p = __top;
	// 遍历左子树
    __x = _S_left(__x);

    while (__x != 0) {
      _Link_type __y = _M_clone_node(__x);
      __p->_M_left = __y;
      __y->_M_parent = __p;
	  // 递归拷贝右子树
      if (__x->_M_right)
        __y->_M_right = _M_copy(_S_right(__x), __y);
      __p = __y;
	  // 遍历左子树
      __x = _S_left(__x);
    }
  }
  __STL_UNWIND(_M_erase(__top));

  return __top;
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
void _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::_M_erase(_Link_type __x)
{
                                // erase without rebalancing
  while (__x != 0) {
	  // 递归删除右子树
    _M_erase(_S_right(__x));
	// 继续往左边遍历
    _Link_type __y = _S_left(__x);
	// 删除节点
    destroy_node(__x);
    __x = __y;
  }
}

// 删除迭代器之间的节点
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
void _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::erase(iterator __first, iterator __last)
{
	// 如果是删除所有的节点，那就直接调用清空函数
  if (__first == begin() && __last == end())
    clear();
  else
	  // 遍历删除
    while (__first != __last) erase(__first++);
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
void _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::erase(const _Key* __first, const _Key* __last) 
{
	// 遍历删除
  while (__first != __last) erase(*__first++);
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::find(const _Key& __k)
{
  _Link_type __y = _M_header;      // Last node which is not less than __k. 
  _Link_type __x = _M_root();      // Current node. 

  // y是大于等于__k的最小节点
  while (__x != 0) 
    if (!_M_key_compare(_S_key(__x), __k))
      __y = __x, __x = _S_left(__x);
    else
      __x = _S_right(__x);

  iterator __j = iterator(__y);   
  // 如果__k >= y节点的键值，就表示__y节点的键值等于__k
  return (__j == end() || _M_key_compare(__k, _S_key(__j._M_node))) ? 
     end() : __j;
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::const_iterator 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::find(const _Key& __k) const
{
  _Link_type __y = _M_header; /* Last node which is not less than __k. */
  _Link_type __x = _M_root(); /* Current node. */
  // y是大于等于__k的最小节点
  while (__x != 0) {
    if (!_M_key_compare(_S_key(__x), __k))
      __y = __x, __x = _S_left(__x);
    else
      __x = _S_right(__x);
  }
  const_iterator __j = const_iterator(__y);   
  // 如果__k >= y节点的键值，就表示__y节点的键值等于__k
  return (__j == end() || _M_key_compare(__k, _S_key(__j._M_node))) ?
    end() : __j;
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::size_type 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::count(const _Key& __k) const
{
	// 找到__k的键的最小和最大的迭代器范围
  pair<const_iterator, const_iterator> __p = equal_range(__k);
  size_type __n = 0;
  distance(__p.first, __p.second, __n);
  return __n;
}

// 返回大于等于__k的第一个元素位置
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::lower_bound(const _Key& __k)
{
  _Link_type __y = _M_header; /* Last node which is not less than __k. */
  _Link_type __x = _M_root(); /* Current node. */

  while (__x != 0) 
	  // __x 大于等于__k
    if (!_M_key_compare(_S_key(__x), __k))
      __y = __x, __x = _S_left(__x);
    else
      __x = _S_right(__x);

  return iterator(__y);
}
// 返回大于等于__k的第一个元素位置
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::const_iterator 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::lower_bound(const _Key& __k) const
{
  _Link_type __y = _M_header; /* Last node which is not less than __k. */
  _Link_type __x = _M_root(); /* Current node. */

  while (__x != 0) 
  // __k <= x，往左走
    if (!_M_key_compare(_S_key(__x), __k))
      __y = __x, __x = _S_left(__x);
  // __x < k
    else
      __x = _S_right(__x);

  return const_iterator(__y);
}

// 返回大于__k的第一个元素位置（大于__k的最小地址）
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::upper_bound(const _Key& __k)
{
  _Link_type __y = _M_header; /* Last node which is greater than __k. */
  _Link_type __x = _M_root(); /* Current node. */

   while (__x != 0) 
		// k < __x
     if (_M_key_compare(__k, _S_key(__x)))
       __y = __x, __x = _S_left(__x);
     else
       __x = _S_right(__x);

   return iterator(__y);
}

// 返回大于__k的第一个元素位置（大于__k的最小地址）
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::const_iterator 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::upper_bound(const _Key& __k) const
{
  _Link_type __y = _M_header; /* Last node which is greater than __k. */
  _Link_type __x = _M_root(); /* Current node. */

   while (__x != 0) 
     if (_M_key_compare(__k, _S_key(__x)))
       __y = __x, __x = _S_left(__x);
     else
       __x = _S_right(__x);

   return const_iterator(__y);
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline 
pair<typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator,
     typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator>
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::equal_range(const _Key& __k)
{
  return pair<iterator, iterator>(lower_bound(__k), upper_bound(__k));
}

template <class _Key, class _Value, class _KoV, class _Compare, class _Alloc>
inline 
pair<typename _Rb_tree<_Key, _Value, _KoV, _Compare, _Alloc>::const_iterator,
     typename _Rb_tree<_Key, _Value, _KoV, _Compare, _Alloc>::const_iterator>
_Rb_tree<_Key, _Value, _KoV, _Compare, _Alloc>
  ::equal_range(const _Key& __k) const
{
  return pair<const_iterator,const_iterator>(lower_bound(__k),
                                             upper_bound(__k));
}

// 节点到根节点的黑色节点数目（递归计算）
inline int 
__black_count(_Rb_tree_node_base* __node, _Rb_tree_node_base* __root)
{
  if (__node == 0)
    return 0;
  else {
	  // 黑色的+1
    int __bc = __node->_M_color == _S_rb_tree_black ? 1 : 0;
	// 如果是根节点就返回
    if (__node == __root)
      return __bc;
    else
		// 递归上溯
      return __bc + __black_count(__node->_M_parent, __root);
  }
}

// 校验树是否满足红黑树的规则
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
bool _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::__rb_verify() const
{
  if (_M_node_count == 0 || begin() == end())
    return _M_node_count == 0 && begin() == end() &&
      _M_header->_M_left == _M_header && _M_header->_M_right == _M_header;
  // 计算最左边节点到根节点的黑色点的数目
  int __len = __black_count(_M_leftmost(), _M_root());
  // 遍历整棵树
  for (const_iterator __it = begin(); __it != end(); ++__it) {
    _Link_type __x = (_Link_type) __it._M_node;
    _Link_type __L = _S_left(__x);
    _Link_type __R = _S_right(__x);
	// 节点是红色，如果左右子节点某一个存在并且是红色，出错
    if (__x->_M_color == _S_rb_tree_red)
      if ((__L && __L->_M_color == _S_rb_tree_red) ||
          (__R && __R->_M_color == _S_rb_tree_red))
        return false;
	// 如果x < 左边子节点键，出错
    if (__L && _M_key_compare(_S_key(__x), _S_key(__L)))
      return false;
	// 如果右边子节点的键 < x, 出错
    if (__R && _M_key_compare(_S_key(__R), _S_key(__x)))
      return false;
	// 如果是叶子节点，到根节点的黑色点的数目不一致，出错
    if (!__L && !__R && __black_count(__x, _M_root()) != __len)
      return false;
  }
  // 最左的节点不是最小节点，出错
  if (_M_leftmost() != _Rb_tree_node_base::_S_minimum(_M_root()))
    return false;
  // 最右节点不是最大节点，出错
  if (_M_rightmost() != _Rb_tree_node_base::_S_maximum(_M_root()))
    return false;

  return true;
}

// Class rb_tree is not part of the C++ standard.  It is provided for
// compatibility with the HP STL.

template <class _Key, class _Value, class _KeyOfValue, class _Compare,
          class _Alloc = __STL_DEFAULT_ALLOCATOR(_Value) >
struct rb_tree : public _Rb_tree<_Key, _Value, _KeyOfValue, _Compare, _Alloc>
{
  typedef _Rb_tree<_Key, _Value, _KeyOfValue, _Compare, _Alloc> _Base;
  typedef typename _Base::allocator_type allocator_type;

  rb_tree(const _Compare& __comp = _Compare(),
          const allocator_type& __a = allocator_type())
    : _Base(__comp, __a) {}
  
  ~rb_tree() {}
};

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1375
#endif

__STL_END_NAMESPACE 

#endif /* __SGI_STL_INTERNAL_TREE_H */

// Local Variables:
// mode:C++
// End:
