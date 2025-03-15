#pragma once

#include "Vulkan/XVKCommon.h"

namespace xvk
{
	class XVKDevice;

	namespace ray
	{
		class XVKRayTracingCore
		{
			VkPhysicalDeviceAccelerationStructurePropertiesKHR vk_accelerationStructureProperties;
			VkPhysicalDeviceRayTracingPipelinePropertiesKHR vk_rayTracingPipelineProperties;
		};
	}
}