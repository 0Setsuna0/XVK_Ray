#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKDevice;
	class XVKDeviceMemory
	{
	public:
		XVKDeviceMemory(const XVKDevice& device, size_t size, uint32_t memoryTypeBits,
			VkMemoryAllocateFlags allocateFlags, VkMemoryPropertyFlags propertyFlags);
		XVKDeviceMemory(XVKDeviceMemory&& other) noexcept;
		~XVKDeviceMemory();

		VkDeviceMemory Handle() const { return vk_deviceMemory; }
		const XVKDevice& GetDevice() const { return xvk_device; }

		void* MapMemory(size_t offset, size_t size);
		void UnmapMemory();

	private:
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		VkDeviceMemory vk_deviceMemory;

		const XVKDevice& xvk_device;
	};
}