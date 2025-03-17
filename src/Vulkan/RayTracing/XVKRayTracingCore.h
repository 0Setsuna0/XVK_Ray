#pragma once

#include "Vulkan/XVKCommon.h"

namespace xvk
{
	class XVKDevice;

	namespace ray
	{
		class XVKRayTracingContext
		{
		public:
			XVKRayTracingContext(const XVKDevice& device);
			~XVKRayTracingContext() {}

			const XVKDevice& GetDevice() const { return xvk_device; }
		
			uint32_t MaxDescriptorSetAccelerationStructures() const { return vk_accelerationStructureProperties.maxDescriptorSetAccelerationStructures; }
			uint64_t MaxGeometryCount() const { return vk_accelerationStructureProperties.maxGeometryCount; }
			uint64_t MaxInstanceCount() const { return vk_accelerationStructureProperties.maxInstanceCount; }
			uint64_t MaxPrimitiveCount() const { return vk_accelerationStructureProperties.maxPrimitiveCount; }
			uint32_t MaxRecursionDepth() const { return vk_rayTracingPipelineProperties.maxRayRecursionDepth; }
			uint32_t MaxShaderGroupStride() const { return vk_rayTracingPipelineProperties.maxShaderGroupStride; }
			uint32_t MinAccelerationStructureScratchOffsetAlignment() const { return vk_accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment; }
			uint32_t ShaderGroupHandleSize() const { return vk_rayTracingPipelineProperties.shaderGroupHandleSize; }
			uint32_t ShaderGroupBaseAlignment() const { return vk_rayTracingPipelineProperties.shaderGroupBaseAlignment; }
			uint32_t ShaderGroupHandleCaptureReplaySize() const { return vk_rayTracingPipelineProperties.shaderGroupHandleCaptureReplaySize; }
		private:
			const XVKDevice& xvk_device;
			VkPhysicalDeviceAccelerationStructurePropertiesKHR vk_accelerationStructureProperties{};
			VkPhysicalDeviceRayTracingPipelinePropertiesKHR vk_rayTracingPipelineProperties{};
		};
	}
}