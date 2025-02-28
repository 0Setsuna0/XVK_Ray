#pragma once
#include "XVKCommon.h"
#include "XVKDeviceMemory.h"

namespace xvk
{
	class XVKCommandPool;
	class XVKDevice;

	class XVKBufferUtil
	{
		template <class T>
		static void CopyFromStagingBuffer(XVKCommandPool& commandPool, XVKBuffer& dstBuffer, const std::vector<T>& data);

		template <class T>
		static void CreateBuffer(XVKCommandPool& commandPool, VkBufferUsageFlags usage, const std::vector<T>& data,
			std::unique_ptr<XVKBuffer>& buffer, std::unique_ptr<XVKDeviceMemory>& memory);
	};

	class XVKBuffer
	{
	public:
		XVKBuffer(const XVKDevice& device, VkDeviceSize size, VkBufferUsageFlags usage);
		~XVKBuffer();

		VkBuffer Handle() const { return vk_buffer; };
		const XVKDevice& GetDevice() const { return xvk_device; };
		void CopyFromBuffer(XVKCommandPool& commandPool, const XVKBuffer& src, VkDeviceSize size);

		XVKDeviceMemory AllocateMemory(VkMemoryPropertyFlags properties) const;
		XVKDeviceMemory AllocateMemory(VkMemoryPropertyFlags properties, VkMemoryAllocateFlags allocateFlags) const;
		VkDeviceAddress GetDeviceAddress() const;

	private:
		VkBuffer vk_buffer;
		const XVKDevice& xvk_device;
	};
}