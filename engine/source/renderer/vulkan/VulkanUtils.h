#pragma once

#include "VulkanTypes.inl"

// Static helpers used for VulkanRenderer
class VulkanUtils
{
    public:
        static bool ResultIsSuccess(VkResult Result);
        static std::string VulkanResultString(VkResult Result, bool bGetExtended);
};