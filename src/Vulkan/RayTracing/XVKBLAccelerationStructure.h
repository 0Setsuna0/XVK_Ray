#pragma once

#include "XVKAccelerationStructure.h"
#include "XVKBLGeometry.h"

namespace vkAsset
{
	class AScene;
}

namespace xvk::ray
{
	class XVKBLAccelerationStructure : public XVKAccelerationStructure
	{
	public:

		XVKBLAccelerationStructure(const XVKRayFuncManager& deviceFunc,
			const XVKRayTracingContext& rayTracingContext,
			const XVKBLGeometry& geometries);
		XVKBLAccelerationStructure(XVKBLAccelerationStructure&& other) noexcept;
		~XVKBLAccelerationStructure();

		void GenerateBLAS(VkCommandBuffer commandBuffer,
			XVKBuffer& scratchBuffer,
			VkDeviceSize scratchOffset,
			XVKBuffer& resultBuffer,
			VkDeviceSize resultOffset);

	private:
		XVKBLGeometry xvk_geometries;
	};
}