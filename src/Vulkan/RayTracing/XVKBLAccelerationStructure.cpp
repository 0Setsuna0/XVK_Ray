#include "XVKBLAccelerationStructure.h"
#include "XVKBLGeometry.h"
#include "XVKRayFuncManager.h"
#include "Asset/Scene.h"
#include "Asset/Vertex.h"
#include "Vulkan/XVKBuffer.h"

namespace xvk::ray
{
	XVKBLAccelerationStructure::XVKBLAccelerationStructure(
		const XVKRayFuncManager& deviceFunc,
		const XVKRayTracingContext& rayTracingContext,
		const XVKBLGeometry& geometries)
		: XVKAccelerationStructure(deviceFunc, rayTracingContext),
		xvk_geometries(geometries)
	{
		vk_buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		vk_buildGeometryInfo.flags = vk_flags;
		vk_buildGeometryInfo.geometryCount = static_cast<uint32_t>(xvk_geometries.GetGeometry().size());
		vk_buildGeometryInfo.pGeometries = xvk_geometries.GetGeometry().data();
		vk_buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		vk_buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		vk_buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;

		std::vector<uint32_t> maxPrimitiveCounts(geometries.GetBuildOffsetInfo().size());
		for (size_t i = 0; i < geometries.GetBuildOffsetInfo().size(); i++)
		{
			maxPrimitiveCounts[i] = geometries.GetBuildOffsetInfo()[i].primitiveCount;
		}

		vk_buildSizesInfo = GetBuildSizesInfo(maxPrimitiveCounts.data());
	}

	XVKBLAccelerationStructure::XVKBLAccelerationStructure(XVKBLAccelerationStructure&& other) noexcept
		: XVKAccelerationStructure(std::move(other)),
		xvk_geometries(std::move(other.xvk_geometries))
	{
	}

	XVKBLAccelerationStructure::~XVKBLAccelerationStructure()
	{
	}

	void XVKBLAccelerationStructure::GenerateBLAS(VkCommandBuffer commandBuffer,
		XVKBuffer& scratchBuffer,
		VkDeviceSize scratchOffset,
		XVKBuffer& resultBuffer,
		VkDeviceSize resultOffset)
	{
		CreateAccelerationStructure(resultBuffer, resultOffset);

		const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = xvk_geometries.GetBuildOffsetInfo().data();

		vk_buildGeometryInfo.dstAccelerationStructure = Handle();
		vk_buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.GetDeviceAddress() + scratchOffset;

		xvk_deviceFunc.vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &vk_buildGeometryInfo, &pBuildOffsetInfo);
	}
}