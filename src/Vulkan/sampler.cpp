#include "sampler.hpp"

#include "../Models/graphic.hpp"

vulkanWrapper::sampler::sampler() = default;
vulkanWrapper::sampler::sampler(VkSamplerCreateInfo &createInfo) {
  Create(createInfo);
}
vulkanWrapper::sampler::sampler(sampler &&other) noexcept { MoveHandle; }
vulkanWrapper::sampler::~sampler() {
  DestroyHandleBy(vkDestroySampler, "sampler");
}
// Non-const Function
VkResultThrowable
vulkanWrapper::sampler::Create(VkSamplerCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  VkResult result = vkCreateSampler(graphic::Singleton().Device(), &createInfo,
                                    nullptr, &handle);
  if (result)
    printf("[ sampler ] ERROR: Failed to create a sampler!\nError code: %d\n",
           int32_t(result));
  return result;
}
