#include "XVKRayTracingCore.h"
#include "Vulkan/XVKDevice.h"

namespace xvk::ray
{
	XVKRayTracingContext::XVKRayTracingContext(const XVKDevice& device)
		:xvk_device(device)
	{
		vk_accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
		vk_rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		vk_rayTracingPipelineProperties.pNext = &vk_accelerationStructureProperties;

		VkPhysicalDeviceProperties2 props = {};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		props.pNext = &vk_rayTracingPipelineProperties;
		vkGetPhysicalDeviceProperties2(device.PhysicalHandle(), &props);
	}
}