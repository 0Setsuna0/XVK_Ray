#pragma once
#include "Vulkan/XVKCommon.h"

namespace xvk
{
	class XVKDevice;
	class XVKDeviceMemory;
	class XVKBuffer;
}

namespace xvk::ray
{
	class XVKRayFuncManager;
	class XVKRayTracingContext;

	class XVKAccelerationStructure
	{
	public:
		XVKAccelerationStructure(XVKAccelerationStructure&& other) noexcept;
		virtual ~XVKAccelerationStructure();
		
		VkAccelerationStructureKHR Handle() const { return vk_accelerationStructure; }
		const XVKDevice& GetDevice() const { return xvk_device; }
		const XVKRayTracingContext& GetRayTracingContext() const { return xvk_rtContext; }
		const XVKRayFuncManager& GetRayFuncManager() const { return xvk_deviceFunc; }
		const VkAccelerationStructureBuildSizesInfoKHR GetBuildSizesInfo() const { return vk_buildSizesInfo; }

		static void MemoryBarrier(VkCommandBuffer commandBuffer);
	protected:
		XVKAccelerationStructure(const XVKRayFuncManager& deviceFunc, const XVKRayTracingContext& rayTracingContext);

		VkAccelerationStructureBuildSizesInfoKHR GetBuildSizesInfo(const uint32_t* pMaxPrimitiveCounts) const;
		void CreateAccelerationStructure(XVKBuffer& storageBuffer, VkDeviceSize resultOffset);

		const XVKRayFuncManager& xvk_deviceFunc;
		const VkBuildAccelerationStructureFlagsKHR vk_flags;

		VkAccelerationStructureBuildGeometryInfoKHR vk_buildGeometryInfo{};
		VkAccelerationStructureBuildSizesInfoKHR vk_buildSizesInfo{};
	private:

		const XVKDevice& xvk_device;
		const XVKRayTracingContext& xvk_rtContext;

		VkAccelerationStructureKHR vk_accelerationStructure;
	};
}