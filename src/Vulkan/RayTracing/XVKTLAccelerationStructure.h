#pragma once

#include "XVKAccelerationStructure.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan [0, 1] depth range, instead of OpenGL [-1, +1]
#define GLM_FORCE_RIGHT_HANDED // Vulkan has a left handed coordinate system (same as DirectX), OpenGL is right handed
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace xvk::ray
{
	class XVKBLAccelerationStructure;

	class XVKTLAccelerationStructure : public XVKAccelerationStructure
	{
	public:

		XVKTLAccelerationStructure(
			const XVKRayFuncManager& deviceFunc,
			const XVKRayTracingContext& rayTracingContext,
			VkDeviceAddress instanceAddress,
			uint32_t instancesCount);
		XVKTLAccelerationStructure(XVKTLAccelerationStructure&& other) noexcept;
		~XVKTLAccelerationStructure();

		void GenerateTLAS(VkCommandBuffer commandBuffer,
			XVKBuffer& scratchBuffer,
			VkDeviceSize scratchOffset,
			XVKBuffer& resultBuffer,
			VkDeviceSize resultOffset);

		static VkAccelerationStructureInstanceKHR CreateInstance(
			const XVKBLAccelerationStructure& blas,
			const glm::mat4& transform,
			uint32_t instanceId,
			uint32_t hitGroupId
		);

	private:

		uint32_t instancesCount;
		VkAccelerationStructureGeometryInstancesDataKHR vk_instancesData;
		VkAccelerationStructureGeometryKHR vk_TLASGeometry;
	};
}