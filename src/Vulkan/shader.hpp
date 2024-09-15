#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include <vulkan/vulkan.h>
namespace vulkanWrapper {

class shader {
  VkShaderModule handle = VK_NULL_HANDLE;

public:
  shader() = default;
  inline shader(VkShaderModuleCreateInfo &createInfo) { Create(createInfo); }
  inline shader(const char *filepath /*VkShaderModuleCreateFlags flags*/) {
    Create(filepath);
  }
  inline shader(size_t codeSize,
                const uint32_t *pCode /*VkShaderModuleCreateFlags flags*/) {
    Create(codeSize, pCode);
  }
  shader(shader &&other) noexcept;
  ~shader();

  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;

  // Const Function
  VkPipelineShaderStageCreateInfo
  StageCreateInfo(VkShaderStageFlagBits stage,
                  const char *entry = "main") const;

  // Non-const Function
  VkResultThrowable Create(VkShaderModuleCreateInfo &createInfo);
  VkResultThrowable
  Create(const char *filepath /*VkShaderModuleCreateFlags flags*/);
  VkResultThrowable
  Create(size_t codeSize,
         const uint32_t *pCode /*VkShaderModuleCreateFlags flags*/);
};

} // namespace vulkanWrapper
