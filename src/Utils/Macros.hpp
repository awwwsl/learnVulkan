#pragma once

#ifndef NDEBUG
#define DestroyHandleBy(Func, name)                                            \
  if (handle) {                                                                \
    printf("[ %s ] DEBUG: Destroying handle: %p\n", name, (void *)handle);     \
    Func(graphic::Singleton().Device(), handle, nullptr);                      \
    handle = VK_NULL_HANDLE;                                                   \
  }
#else
#define DestroyHandleBy(Func, name)                                            \
  if (handle) {                                                                \
    Func(graphicsBase::Singleton().Device(), handle, nullptr);                 \
    handle = VK_NULL_HANDLE;                                                   \
  }
#endif

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

#define DefineAddressFunction                                                  \
  const decltype(handle) *Address() const { return &handle; }

#define ExecuteOnce(...)                                                       \
  {                                                                            \
    static bool executed = false;                                              \
    if (executed)                                                              \
      return __VA_ARGS__;                                                      \
    executed = true;                                                           \
  }

#ifndef NDEBUG
#define AddCallback(container, callback, containerName)                        \
  if (callback) {                                                              \
    container.push_back(callback);                                             \
  }
#else
#define AddCallback(container, callback, containerName)                        \
  printf("[ %s ] DEBUG: Adding callback\n", containerName);                    \
  if (callback) {                                                              \
    container.push_back(callback);                                             \
  }
#endif
