#include "XVKAccelerationStructure.h"
#include "XVKRayFuncManager.h"
#include "XVKRayTracingCore.h"
#include "Vulkan/XVKBuffer.h"
#include "Vulkan/XVKDevice.h"

uint64_t RoundUp(uint64_t size, uint64_t granularity)
{
	const auto divUp = (size + granularity - 1) / granularity;
	return divUp * granularity;
}

namespace xvk::ray
{
	XVKAccelerationStructure::XVKAccelerationStructure(const XVKRayFuncManager& deviceFunc, const XVKRayTracingContext& rayTracingContext)
		:xvk_deviceFunc(deviceFunc), 
		 xvk_device(rayTracingContext.GetDevice()), 
		 xvk_rtContext(rayTracingContext), 
		 vk_flags(VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR)
	{

	}

	XVKAccelerationStructure::XVKAccelerationStructure(XVKAccelerationStructure&& other) noexcept
		:xvk_deviceFunc(other.xvk_deviceFunc), 
		 xvk_device(other.xvk_device), 
		 xvk_rtContext(other.xvk_rtContext), 
		 vk_flags(other.vk_flags), 
		 vk_buildGeometryInfo(other.vk_buildGeometryInfo), 
		 vk_buildSizesInfo(other.vk_buildSizesInfo), 
		 vk_accelerationStructure(other.vk_accelerationStructure)
	{
		other.vk_accelerationStructure = VK_NULL_HANDLE;
	}

	XVKAccelerationStructure::~XVKAccelerationStructure()
	{
		if (vk_accelerationStructure != VK_NULL_HANDLE)
		{
			xvk_deviceFunc.vkDestroyAccelerationStructureKHR(xvk_device.Handle(), vk_accelerationStructure, nullptr);
			vk_accelerationStructure = VK_NULL_HANDLE;
		}
	}

	VkAccelerationStructureBuildSizesInfoKHR XVKAccelerationStructure::GetBuildSizesInfo(const uint32_t* pMaxPrimitiveCounts) const
	{
		VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {};
		buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

		xvk_deviceFunc.vkGetAccelerationStructureBuildSizesKHR(
			xvk_device.Handle(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&vk_buildGeometryInfo,
			pMaxPrimitiveCounts,
			&buildSizesInfo);

		// AccelerationStructure offset needs to be 256 bytes aligned
		const uint64_t AccelerationStructureAlignment = 256;
		const uint64_t ScratchAlignment = xvk_rtContext.MinAccelerationStructureScratchOffsetAlignment();

		buildSizesInfo.accelerationStructureSize = RoundUp(buildSizesInfo.accelerationStructureSize, AccelerationStructureAlignment);
		buildSizesInfo.buildScratchSize = RoundUp(buildSizesInfo.buildScratchSize, ScratchAlignment);
		
		return buildSizesInfo;
	}

	void XVKAccelerationStructure::CreateAccelerationStructure(XVKBuffer& storageBuffer, VkDeviceSize resultOffset)
	{
		VkAccelerationStructureCreateInfoKHR accelerationStructureInfo = {};
		accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureInfo.pNext = nullptr;
		accelerationStructureInfo.type = vk_buildGeometryInfo.type;
		accelerationStructureInfo.size = GetBuildSizesInfo().accelerationStructureSize;
		accelerationStructureInfo.buffer = storageBuffer.Handle();
		accelerationStructureInfo.offset = resultOffset;

		VULKAN_RUNTIME_CHECK(xvk_deviceFunc.vkCreateAccelerationStructureKHR(xvk_device.Handle(),
			&accelerationStructureInfo, nullptr, &vk_accelerationStructure), "create acceleration structure");
	}

	void XVKAccelerationStructure::MemoryBarrierRT(VkCommandBuffer commandBuffer)
	{
		VkMemoryBarrier memoryBarrier = {};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier.pNext = nullptr;
		memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
		memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			0,
			1, &memoryBarrier,
			0, nullptr,
			0, nullptr);
	}
}