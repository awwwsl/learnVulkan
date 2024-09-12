#pragma once

#include "../Models/graphic.hpp"

#define DestroyHandleBy(Func)                                                  \
  if (handle) {                                                                \
    Func(graphicsBase::Singleton().Device(), handle, nullptr);                 \
    handle = VK_NULL_HANDLE;                                                   \
  }

#define MoveHandle                                                             \
  handle = other.handle;                                                       \
  other.handle = VK_NULL_HANDLE;

#define DefineMoveAssignmentOperator(type)                                     \
  type &operator=(type &&other) {                                              \
    this->~type();                                                             \
    MoveHandle;                                                                \
    return *this;                                                              \
  }

#define DefineMoveAssignmentOperatorHeader (type) type &operator=(type &&other);

#define DefineHandleTypeOperator                                               \
  operator decltype(handle)() const { return handle; }

#define DefineHandleTypeOperatorHeader operator decltype(handle)() const;

#define DefineAddressFunction                                                  \
  const decltype(handle) *Address() const { return &handle; }

#define DefineAddressFunctionHeader const decltype(handle) *Address() const;

#define ExecuteOnce(...)                                                       \
  {                                                                            \
    static bool executed = false;                                              \
    if (executed)                                                              \
      return __VA_ARGS__;                                                      \
    executed = true;                                                           \
  }
