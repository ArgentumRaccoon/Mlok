#pragma once

#include "VulkanTypes.inl"

// Static helpers used for VulkanRenderer
class VulkanUtils
{
    public:
        static bool ResultIsSuccess(vk::Result Result);
        static std::string VulkanResultString(vk::Result Result, bool bGetExtended);
};