#pragma once

#ifndef VKRESULTTHROWABLE_HPP
#define VKRESULTTHROWABLE_HPP

#include <vulkan/vulkan.h>
#include <stdexcept>

#ifdef VK_RESULT_THROW

class VkResultThrowable {
    VkResult result;

public:
    static void (*callback_throw)(VkResult);

    explicit VkResultThrowable(VkResult result) : result(result) {}

    VkResultThrowable(VkResultThrowable &&other) noexcept : result(other.result) {
        other.result = VK_SUCCESS;
    }

    ~VkResultThrowable() noexcept(false) {
        if (uint32_t(result) < VK_RESULT_MAX_ENUM)
            return;
        if (callback_throw)
            callback_throw(result);
        throw result;
    }

    operator VkResult() {
        VkResult result = this->result;
        this->result = VK_SUCCESS;
        return result;
    }
};

inline void (*VkResultThrowable::callback_throw)(VkResult) = nullptr;

#elif defined VK_RESULT_NODISCARD

struct [[nodiscard]] VkResultThrowable {
    VkResult result;

    explicit VkResultThrowable(VkResult result) : result(result) {}

    operator VkResult() const { return result; }
};

// 禁用 MSVC 编译器的一些警告
#pragma warning(disable : 4834)
#pragma warning(disable : 6031)

#else

using VkResultThrowable = VkResult;

#endif // VK_RESULT_THROW or VK_RESULT_NODISCARD

#endif // VKRESULTTHROWABLE_HPP
