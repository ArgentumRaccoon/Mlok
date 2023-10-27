#include "VulkanUtils.h"

bool VulkanUtils::ResultIsSuccess(vk::Result Result)
{
    switch (Result) 
    {
        // Success Codes
        default:
        case vk::Result::eSuccess:
        case vk::Result::eNotReady:
        case vk::Result::eTimeout:
        case vk::Result::eEventSet:
        case vk::Result::eEventReset:
        case vk::Result::eIncomplete:
        case vk::Result::eSuboptimalKHR:
        case vk::Result::eThreadIdleKHR:
        case vk::Result::eThreadDoneKHR:
        case vk::Result::eOperationDeferredKHR:
        case vk::Result::eOperationNotDeferredKHR:
        case vk::Result::ePipelineCompileRequiredEXT:
            return true;

        // Error codes
        case vk::Result::eErrorOutOfHostMemory:
        case vk::Result::eErrorOutOfDeviceMemory:
        case vk::Result::eErrorInitializationFailed:
        case vk::Result::eErrorDeviceLost:
        case vk::Result::eErrorMemoryMapFailed:
        case vk::Result::eErrorLayerNotPresent:
        case vk::Result::eErrorExtensionNotPresent:
        case vk::Result::eErrorFeatureNotPresent:
        case vk::Result::eErrorIncompatibleDriver:
        case vk::Result::eErrorTooManyObjects:
        case vk::Result::eErrorFormatNotSupported:
        case vk::Result::eErrorFragmentedPool:
        case vk::Result::eErrorSurfaceLostKHR:
        case vk::Result::eErrorNativeWindowInUseKHR:
        case vk::Result::eErrorOutOfDateKHR:
        case vk::Result::eErrorIncompatibleDisplayKHR:
        case vk::Result::eErrorInvalidShaderNV:
        case vk::Result::eErrorOutOfPoolMemory:
        case vk::Result::eErrorInvalidExternalHandle:
        case vk::Result::eErrorFragmentation:
        case vk::Result::eErrorInvalidDeviceAddressEXT:
        // NOTE: Same as above
        //case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        case vk::Result::eErrorUnknown:
            return false;
    }
}

std::string VulkanUtils::VulkanResultString(vk::Result Result, bool bGetExtended)
{
    switch (Result) 
    {
        default:
        case vk::Result::eSuccess:
            return !bGetExtended ? "VK_SUCCESS" : "VK_SUCCESS Command successfully completed";
        case vk::Result::eNotReady:
            return !bGetExtended ? "VK_NOT_READY" : "VK_NOT_READY A fence or query has not yet completed";
        case vk::Result::eTimeout:
            return !bGetExtended ? "VK_TIMEOUT" : "VK_TIMEOUT A wait operation has not completed in the specified time";
        case vk::Result::eEventSet:
            return !bGetExtended ? "VK_EVENT_SET" : "VK_EVENT_SET An event is signaled";
        case vk::Result::eEventReset:
            return !bGetExtended ? "VK_EVENT_RESET" : "VK_EVENT_RESET An event is unsignaled";
        case vk::Result::eIncomplete:
            return !bGetExtended ? "VK_INCOMPLETE" : "VK_INCOMPLETE A return array was too small for the result";
        case vk::Result::eSuboptimalKHR:
            return !bGetExtended ? "VK_SUBOPTIMAL_KHR" : "VK_SUBOPTIMAL_KHR A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.";
        case vk::Result::eThreadIdleKHR:
            return !bGetExtended ? "VK_THREAD_IDLE_KHR" : "VK_THREAD_IDLE_KHR A deferred operation is not complete but there is currently no work for this thread to do at the time of this call.";
        case vk::Result::eThreadDoneKHR:
            return !bGetExtended ? "VK_THREAD_DONE_KHR" : "VK_THREAD_DONE_KHR A deferred operation is not complete but there is no work remaining to assign to additional threads.";
        case vk::Result::eOperationDeferredKHR:
            return !bGetExtended ? "VK_OPERATION_DEFERRED_KHR" : "VK_OPERATION_DEFERRED_KHR A deferred operation was requested and at least some of the work was deferred.";
        case vk::Result::eOperationNotDeferredKHR:
            return !bGetExtended ? "VK_OPERATION_NOT_DEFERRED_KHR" : "VK_OPERATION_NOT_DEFERRED_KHR A deferred operation was requested and no operations were deferred.";
        case vk::Result::ePipelineCompileRequiredEXT:
            return !bGetExtended ? "VK_PIPELINE_COMPILE_REQUIRED_EXT" : "VK_PIPELINE_COMPILE_REQUIRED_EXT A requested pipeline creation would have required compilation, but the application requested compilation to not be performed.";

        // Error codes
        case vk::Result::eErrorOutOfHostMemory:
            return !bGetExtended ? "VK_ERROR_OUT_OF_HOST_MEMORY" : "VK_ERROR_OUT_OF_HOST_MEMORY A host memory allocation has failed.";
        case vk::Result::eErrorOutOfDeviceMemory:
            return !bGetExtended ? "VK_ERROR_OUT_OF_DEVICE_MEMORY" : "VK_ERROR_OUT_OF_DEVICE_MEMORY A device memory allocation has failed.";
        case vk::Result::eErrorInitializationFailed:
            return !bGetExtended ? "VK_ERROR_INITIALIZATION_FAILED" : "VK_ERROR_INITIALIZATION_FAILED Initialization of an object could not be completed for implementation-specific reasons.";
        case vk::Result::eErrorDeviceLost:
            return !bGetExtended ? "VK_ERROR_DEVICE_LOST" : "VK_ERROR_DEVICE_LOST The logical or physical device has been lost. See Lost Device";
        case vk::Result::eErrorMemoryMapFailed:
            return !bGetExtended ? "VK_ERROR_MEMORY_MAP_FAILED" : "VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory object has failed.";
        case vk::Result::eErrorLayerNotPresent:
            return !bGetExtended ? "VK_ERROR_LAYER_NOT_PRESENT" : "VK_ERROR_LAYER_NOT_PRESENT A requested layer is not present or could not be loaded.";
        case vk::Result::eErrorExtensionNotPresent:
            return !bGetExtended ? "VK_ERROR_EXTENSION_NOT_PRESENT" : "VK_ERROR_EXTENSION_NOT_PRESENT A requested extension is not supported.";
        case vk::Result::eErrorFeatureNotPresent:
            return !bGetExtended ? "VK_ERROR_FEATURE_NOT_PRESENT" : "VK_ERROR_FEATURE_NOT_PRESENT A requested feature is not supported.";
        case vk::Result::eErrorIncompatibleDriver:
            return !bGetExtended ? "VK_ERROR_INCOMPATIBLE_DRIVER" : "VK_ERROR_INCOMPATIBLE_DRIVER The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
        case vk::Result::eErrorTooManyObjects:
            return !bGetExtended ? "VK_ERROR_TOO_MANY_OBJECTS" : "VK_ERROR_TOO_MANY_OBJECTS Too many objects of the type have already been created.";
        case vk::Result::eErrorFormatNotSupported:
            return !bGetExtended ? "VK_ERROR_FORMAT_NOT_SUPPORTED" : "VK_ERROR_FORMAT_NOT_SUPPORTED A requested format is not supported on this device.";
        case vk::Result::eErrorFragmentedPool:
            return !bGetExtended ? "VK_ERROR_FRAGMENTED_POOL" : "VK_ERROR_FRAGMENTED_POOL A pool allocation has failed due to fragmentation of the pool’s memory. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. This should be returned in preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the implementation is certain that the pool allocation failure was due to fragmentation.";
        case vk::Result::eErrorSurfaceLostKHR:
            return !bGetExtended ? "VK_ERROR_SURFACE_LOST_KHR" : "VK_ERROR_SURFACE_LOST_KHR A surface is no longer available.";
        case vk::Result::eErrorNativeWindowInUseKHR:
            return !bGetExtended ? "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" : "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again.";
        case vk::Result::eErrorOutOfDateKHR:
            return !bGetExtended ? "VK_ERROR_OUT_OF_DATE_KHR" : "VK_ERROR_OUT_OF_DATE_KHR A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail. Applications must query the new surface properties and recreate their swapchain if they wish to continue presenting to the surface.";
        case vk::Result::eErrorIncompatibleDisplayKHR:
            return !bGetExtended ? "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" : "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image.";
        case vk::Result::eErrorInvalidShaderNV:
            return !bGetExtended ? "VK_ERROR_INVALID_SHADER_NV" : "VK_ERROR_INVALID_SHADER_NV One or more shaders failed to compile or link. More details are reported back to the application via VK_EXT_debug_report if enabled.";
        case vk::Result::eErrorOutOfPoolMemory:
            return !bGetExtended ? "VK_ERROR_OUT_OF_POOL_MEMORY" : "VK_ERROR_OUT_OF_POOL_MEMORY A pool memory allocation has failed. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. If the failure was definitely due to fragmentation of the pool, VK_ERROR_FRAGMENTED_POOL should be returned instead.";
        case vk::Result::eErrorInvalidExternalHandle:
            return !bGetExtended ? "VK_ERROR_INVALID_EXTERNAL_HANDLE" : "VK_ERROR_INVALID_EXTERNAL_HANDLE An external handle is not a valid handle of the specified type.";
        case vk::Result::eErrorFragmentation:
            return !bGetExtended ? "VK_ERROR_FRAGMENTATION" : "VK_ERROR_FRAGMENTATION A descriptor pool creation has failed due to fragmentation.";
        case vk::Result::eErrorInvalidDeviceAddressEXT:
            return !bGetExtended ? "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT" : "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT A buffer creation failed because the requested address is not available.";
        // NOTE: Same as above
        //case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        //    return !bGetExtended ? "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS" :"VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS A buffer creation or memory allocation failed because the requested address is not available. A shader group handle assignment failed because the requested shader group handle information is no longer valid.";
        case vk::Result::eErrorUnknown:
            return !bGetExtended ? "VK_ERROR_UNKNOWN" : "VK_ERROR_UNKNOWN An unknown error has occurred; either the application has provided invalid input, or an implementation failure has occurred.";
    }
}
