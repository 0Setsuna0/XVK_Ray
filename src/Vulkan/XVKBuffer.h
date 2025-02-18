#pragma once
#include "XVKCommon.h"
#include "XVKDeviceMemory.h"

namespace xvk
{
	class XVKCommandPool;
	class XVKDevice;

	class XVKBuffer
	{
	public:
		XVKBuffer(const XVKDevice& device, VkDeviceSize size, VkBufferUsageFlags usage);
		~XVKBuffer();

		VkBuffer Handle() const { return vk_buffer; };
		const XVKDevice& GetDevice() const { return xvk_device; };
		void CopyFromBuffer(XVKCommandPool commandPool, const XVKBuffer& src, VkDeviceSize size);

		XVKDeviceMemory AllocateMemory(VkMemoryPropertyFlags properties) const;
		XVKDeviceMemory AllocateMemory(VkMemoryPropertyFlags properties, VkMemoryAllocateFlags allocateFlags) const;
		VkDeviceAddress GetDeviceAddress() const;

	private:
		VkBuffer vk_buffer;
		const XVKDevice& xvk_device;
	};
}