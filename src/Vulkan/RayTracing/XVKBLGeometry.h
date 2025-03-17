#pragma once

#include "Vulkan/XVKCommon.h"

namespace vkAsset
{
	class AScene;
}

namespace xvk::ray
{
	class XVKBLGeometry
	{
	public:
		
		uint32_t GetSize() const { return static_cast<uint32_t>(vk_geometry.size()); }

		const std::vector<VkAccelerationStructureGeometryKHR>& GetGeometry() const { return vk_geometry; }
		const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& GetBuildOffsetInfo() const { return vk_buildOffsetInfo; }

		void AddGeometryTriangles(const vkAsset::AScene& scene,
			uint32_t vertexOffset,
			uint32_t vertexCount,
			uint32_t indexOffset,
			uint32_t indexCount,
			bool isOpaque);

	private:

		// The geometry to build, addresses of vertices and indices.
		std::vector<VkAccelerationStructureGeometryKHR> vk_geometry;

		// the number of elements to build and offsets
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> vk_buildOffsetInfo;
	};
}