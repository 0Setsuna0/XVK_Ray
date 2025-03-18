#include "XVKTLAccelerationStructure.h"
#include "XVKBLAccelerationStructure.h"
#include "XVKRayFuncManager.h"
#include "Vulkan/XVKBuffer.h"
#include "Vulkan/XVKDevice.h"

namespace xvk::ray
{
	XVKTLAccelerationStructure::XVKTLAccelerationStructure(
		const XVKRayFuncManager& deviceFunc,
		const XVKRayTracingContext& rayTracingContext,
		VkDeviceAddress instanceAddress,
		uint32_t instancesCount)
		:XVKAccelerationStructure(deviceFunc, rayTracingContext),
		instancesCount(instancesCount)
	{
		vk_instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		vk_instancesData.arrayOfPointers = VK_FALSE;
		vk_instancesData.data.deviceAddress = instanceAddress;

		vk_TLASGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		vk_TLASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		vk_TLASGeometry.geometry.instances = vk_instancesData;

		vk_buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		vk_buildGeometryInfo.flags = vk_flags;
		vk_buildGeometryInfo.geometryCount = 1;
		vk_buildGeometryInfo.pGeometries = &vk_TLASGeometry;
		vk_buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		vk_buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		vk_buildGeometryInfo.srcAccelerationStructure = nullptr;
		
		vk_buildSizesInfo = GetBuildSizesInfo(&instancesCount);
	}

	XVKTLAccelerationStructure::XVKTLAccelerationStructure(XVKTLAccelerationStructure&& other) noexcept
		:XVKAccelerationStructure(std::move(other)),
		instancesCount(other.instancesCount)
	{

	}

	XVKTLAccelerationStructure::~XVKTLAccelerationStructure()
	{

	}

	void XVKTLAccelerationStructure::GenerateTLAS(VkCommandBuffer commandBuffer,
		XVKBuffer& scratchBuffer,
		VkDeviceSize scratchOffset,
		XVKBuffer& resultBuffer,
		VkDeviceSize resultOffset)
	{
		CreateAccelerationStructure(resultBuffer, resultOffset);

		VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo = {};
		buildOffsetInfo.primitiveCount = instancesCount;

		const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = &buildOffsetInfo;
		
		vk_buildGeometryInfo.dstAccelerationStructure = Handle();
		vk_buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.GetDeviceAddress() + scratchOffset;
	
		xvk_deviceFunc.vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &vk_buildGeometryInfo, &pBuildOffsetInfo);
	}

	VkAccelerationStructureInstanceKHR XVKTLAccelerationStructure::CreateInstance(
		const XVKBLAccelerationStructure& blas,
		const glm::mat4& transform,
		uint32_t instanceId,
		uint32_t hitGroupId)
	{
		const XVKDevice& device = blas.GetDevice();
		const XVKRayFuncManager& deviceFunc = blas.GetRayFuncManager();

		VkAccelerationStructureDeviceAddressInfoKHR addressInfo = {};
		addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		addressInfo.accelerationStructure = blas.Handle();

		const VkDeviceAddress address = deviceFunc.vkGetAccelerationStructureDeviceAddressKHR(device.Handle(), &addressInfo);

		VkAccelerationStructureInstanceKHR instance = {};
		instance.instanceCustomIndex = instanceId;
		instance.mask = 0XFF;
		instance.instanceShaderBindingTableRecordOffset = hitGroupId;
		instance.accelerationStructureReference = address;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	
		std::memcpy(&instance.transform, &transform, sizeof(instance.transform));

		return instance;
	}
}