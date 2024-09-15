#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include <vulkan/vulkan.h>

namespace vulkanWrapper {

class imageView {
  VkImageView handle = VK_NULL_HANDLE;

public:
  imageView();
  imageView(VkImageViewCreateInfo &createInfo);
  imageView(VkImage image, VkImageViewType viewType, VkFormat format,
            const VkImageSubresourceRange &subresourceRange,
            VkImageViewCreateFlags flags = 0);
  imageView(imageView &&other) noexcept;
  ~imageView();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Non-const Function
  VkResultThrowable Create(VkImageViewCreateInfo &createInfo);
  VkResultThrowable Create(VkImage image, VkImageViewType viewType,
                           VkFormat format,
                           const VkImageSubresourceRange &subresourceRange,
                           VkImageViewCreateFlags flags = 0);
};

} // namespace vulkanWrapper
