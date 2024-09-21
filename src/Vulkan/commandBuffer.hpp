#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {

class commandBuffer {
  friend class
      commandPool; // 封装命令池的commandPool类负责分配和释放命令缓冲区，需要让其能访问私有成员handle
  VkCommandBuffer handle = VK_NULL_HANDLE;

public:
  commandBuffer();
  commandBuffer(commandBuffer &&other) noexcept;
  commandBuffer(const commandBuffer &other) noexcept {
    this->handle = other.handle;
  }
  // 命令缓冲区的析构函数定义在封装命令池的commandPool类中
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  commandBuffer operator=(const VkCommandBuffer &other) {
    handle = other;
    return *this;
  }
  // Const Function
  // 这里没给inheritanceInfo设定默认参数，因为C++标准中规定对空指针解引用是未定义行为（尽管运行期不必发生，且至少MSVC编译器允许这种代码），而我又一定要传引用而非指针，因而形成了两个Begin(...)
  VkResultThrowable
  Begin(VkCommandBufferUsageFlags usageFlags,
        VkCommandBufferInheritanceInfo &inheritanceInfo) const;
  VkResultThrowable Begin(VkCommandBufferUsageFlags usageFlags = 0) const;
  VkResultThrowable End() const;
};

} // namespace vulkanWrapper
