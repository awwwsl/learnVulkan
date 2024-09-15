#include "shader.hpp"

#include "../Models/graphic.hpp"

#include <fstream>

vulkanWrapper::shader::shader(shader &&other) noexcept { MoveHandle; }
vulkanWrapper::shader::~shader() {
  DestroyHandleBy(vkDestroyShaderModule, "shader");
}

// Const Function
VkPipelineShaderStageCreateInfo
vulkanWrapper::shader::StageCreateInfo(VkShaderStageFlagBits stage,
                                       const char *entry) const {
  return {
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType
      nullptr,                                             // pNext
      0,                                                   // flags
      stage,                                               // stage
      handle,                                              // module
      entry,                                               // pName
      nullptr // pSpecializationInfo
  };
}
// Non-const Function
VkResultThrowable
vulkanWrapper::shader::Create(VkShaderModuleCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  VkResult result = vkCreateShaderModule(graphic::Singleton().Device(),
                                         &createInfo, nullptr, &handle);
  if (result)
    printf("[ shader ] ERROR: Failed to create a shader "
           "module!\nError code: %d\n",
           int32_t(result));
  return result;
}
VkResultThrowable vulkanWrapper::shader::Create(
    const char *filepath /*VkShaderModuleCreateFlags flags*/) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);
  if (!file) {
    printf("[ shader ] ERROR: Failed to open the file: %s\n", filepath);
    return VK_RESULT_MAX_ENUM; // 没有合适的错误代码，别用VK_ERROR_UNKNOWN
  }
  size_t fileSize = size_t(file.tellg());
  std::vector<uint32_t> binaries(fileSize / 4);
  file.seekg(0);
  file.read(reinterpret_cast<char *>(binaries.data()), fileSize);
  file.close();
  return Create(fileSize, binaries.data());
}
VkResultThrowable vulkanWrapper::shader::Create(
    size_t codeSize,
    const uint32_t *pCode /*VkShaderModuleCreateFlags flags*/) {
  VkShaderModuleCreateInfo createInfo = {.codeSize = codeSize, .pCode = pCode};
  return Create(createInfo);
};
