#include "XVKBuffer.h"
#include "XVKDevice.h"
#include "XVKCommandPool.h"
#include "XVKTransientCommand.h"

namespace xvk
{
	XVKBuffer::XVKBuffer(const XVKDevice& device, VkDeviceSize size, VkBufferUsageFlags usage)
		:xvk_device(device)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VULKAN_RUNTIME_CHECK(vkCreateBuffer(device.Handle(), &bufferInfo, nullptr, &vk_buffer), "create buffer");
	}
	XVKBuffer::~XVKBuffer()
	{
		if (vk_buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(xvk_device.Handle(), vk_buffer, nullptr);
			vk_buffer = VK_NULL_HANDLE;
		}
	}
	XVKDeviceMemory XVKBuffer::AllocateMemory(VkMemoryPropertyFlags properties) const
	{
		return AllocateMemory(properties, 0);
	}

	XVKDeviceMemory XVKBuffer::AllocateMemory(VkMemoryPropertyFlags properties, VkMemoryAllocateFlags allocateFlags) const
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(xvk_device.Handle(), vk_buffer, &memRequirements);
		XVKDeviceMemory memory(xvk_device, memRequirements.size, memRequirements.memoryTypeBits, allocateFlags, properties);
		
		VULKAN_RUNTIME_CHECK(vkBindBufferMemory(xvk_device.Handle(), vk_buffer, memory.Handle(), 0),
			"bind buffer memory");
		return memory;
	}

	VkDeviceAddress XVKBuffer::GetDeviceAddress() const
	{
		VkBufferDeviceAddressInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		info.buffer = vk_buffer;
		info.pNext = nullptr;

		return vkGetBufferDeviceAddress(xvk_device.Handle(), &info);
	}

	void XVKBuffer::CopyFromBuffer(XVKCommandPool commandPool, const XVKBuffer& src, VkDeviceSize size)
	{
		XVKTransientCommands::Submit(commandPool, [&](VkCommandBuffer commandBuffer) {
			VkBufferCopy copyRegion = {};
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, src.Handle(), vk_buffer, 1, &copyRegion);
		});
	}
}