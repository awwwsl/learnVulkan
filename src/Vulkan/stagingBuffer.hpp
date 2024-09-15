#pragma once

#include "../Models/graphic.hpp"

#include "bufferMemory.hpp"
#include "image.hpp"

#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace vulkanWrapper {

class stagingBuffer {
  static std::unordered_map<std::thread::id, stagingBuffer *> buffers;
  static std::shared_mutex buffersMutex;

protected:
  bufferMemory memory;
  VkDeviceSize memoryUsage = 0;
  image aliasedImage;

public:
  stagingBuffer();
  stagingBuffer(VkDeviceSize size);
  // Getter
  operator VkBuffer() const { return memory.Buffer(); }
  const VkBuffer *Address() const { return memory.AddressOfBuffer(); }
  VkDeviceSize AllocationSize() const { return memory.AllocationSize(); }
  VkImage AliasedImage() const { return aliasedImage; }
  // Const Function
  void RetrieveData(void *pData_src, VkDeviceSize size) const;
  // Non-const Function
  void Expand(VkDeviceSize size);
  void Release();
  void *MapMemory(VkDeviceSize size);
  void UnmapMemory();
  void BufferData(const void *pData_src, VkDeviceSize size);
  [[nodiscard]]
  VkImage AliasedImage2d(VkFormat format, VkExtent2D extent);

  // Static Methods
  static stagingBuffer &CurrentThread(bool create = false) {
    if (create && buffers.find(std::this_thread::get_id()) == buffers.end())
      return RegisterCurrentThread();
    return *buffers[std::this_thread::get_id()];
  }

  static stagingBuffer &RegisterCurrentThread() {
    buffersMutex.lock();
    stagingBuffer *buffer = new stagingBuffer();
    buffers[std::this_thread::get_id()] = buffer;
    graphic ::Singleton().AddDestroyDeviceCallback([buffer]() {
      buffersMutex.lock();
      ExecuteOnce();
      buffer->~stagingBuffer();
      buffers.erase(std::this_thread::get_id());
      buffersMutex.unlock();
    });
    return *buffer;
  }

  // Static Methods for CurrentThread
  static VkBuffer Buffer_CurrentThread() { return CurrentThread(); }
  static void Expand_CurrentThread(VkDeviceSize size) {
    CurrentThread().Expand(size);
  }
  static void Release_CurrentThread() { CurrentThread().Release(); }
  static void *MapMemory_CurrentThread(VkDeviceSize size) {
    return CurrentThread().MapMemory(size);
  }
  static void UnmapMemory_CurrentThread() { CurrentThread().UnmapMemory(); }
  static void BufferData_CurrentThread(const void *pData_src,
                                       VkDeviceSize size) {
    CurrentThread().BufferData(pData_src, size);
  }
  static void RetrieveData_CurrentThread(void *pData_src, VkDeviceSize size) {
    CurrentThread().RetrieveData(pData_src, size);
  }
  [[nodiscard]]
  static VkImage AliasedImage2d_CurrentThread(VkFormat format,
                                              VkExtent2D extent) {
    return CurrentThread().AliasedImage2d(format, extent);
  }
};

} // namespace vulkanWrapper
