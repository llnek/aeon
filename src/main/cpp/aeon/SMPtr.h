#pragma once

// from stdc++

#include "fusilli.h"
NS_BEGIN(fusii)
//////////////////////////////////////////////////////////////////////////////
// Simple smart pointer.
template<typename _Tp1> struct MS_DLL SMPtr_ref {
  _Tp1 *_M_ptr;
  explicit
  SMPtr_ref(_Tp1 *__p) : _M_ptr(__p) { }
};

//////////////////////////////////////////////////////////////////////////////
//
template<typename _Tp> class MS_DLL SMPtr {

  _Tp *_M_ptr;

public:

  typedef _Tp pType;

  explicit
  SMPtr(pType *__p = nullptr) throw() : _M_ptr(__p) { }

  SMPtr(SMPtr &__a) throw() : _M_ptr(__a.release()) { }

  template<typename _Tp1>
  SMPtr(SMPtr<_Tp1> &__a) throw() : _M_ptr(__a.release()) { }

  SMPtr&
  operator=(SMPtr &__a) throw() {
    reset(__a.release());
    return *this;
  }

  template<typename _Tp1>
  SMPtr&
  operator=(SMPtr<_Tp1> &__a) throw() {
    reset(__a.release());
    return *this;
  }

  ~SMPtr() { delete _M_ptr; }

  pType&
  operator*() const throw() {
    assert(_M_ptr != nullptr);
    return *_M_ptr;
  }

  pType*
  operator->() const throw() {
    assert(_M_ptr != nullptr);
    return _M_ptr;
  }

  pType* get() const throw() { return _M_ptr; }

  pType* release() throw() {
    pType *__tmp = _M_ptr;
    _M_ptr = nullptr;
    return __tmp;
  }

  void reset(pType* __p = 0) throw() {
    if (__p != _M_ptr) {
      delete _M_ptr;
      _M_ptr = __p;
    }
  }

  SMPtr(SMPtr_ref<pType> __ref) throw()
  : _M_ptr(__ref._M_ptr) { }

  SMPtr&
  operator=(SMPtr_ref<pType> __ref) throw() {
    if (__ref._M_ptr != this->get()) {
      delete _M_ptr;
      _M_ptr = __ref._M_ptr;
    }
    return *this;
  }

  template<typename _Tp1>
  operator SMPtr_ref<_Tp1>() throw() {
    return SMPtr_ref<_Tp1>(this->release());
  }

  template<typename _Tp1>
  operator SMPtr<_Tp1>() throw() {
    return SMPtr<_Tp1>(this->release()); }

};


NS_END


