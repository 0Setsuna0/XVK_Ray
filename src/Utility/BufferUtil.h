#pragma once

#include "Vulkan/XVKCommandPool.h"
#include "Vulkan/XVKBuffer.h"
#include "Vulkan/XVKDeviceMemory.h"
#include "Vulkan/XVKDevice.h"
#include <memory>
#include <vector>
#include <cstring>

namespace utility
{
	class BufferUtility
	{
	public:
		template<class T>
		static void CopyFromBuffer(xvk::XVKCommandPool& commandPool, xvk::XVKBuffer& dstBuffer, const std::vector<T>& srcData)
		{
			const xvk::XVKDevice& device = commandPool.GetDevice();
			VkDeviceSize bufferSize = sizeof(srcData[0]) * srcData.size();

			auto stagingBuffer = std::make_unique<xvk::XVKBuffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			auto stagingBufferMemory = stagingBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			void* dstData = stagingBufferMemory.MapMemory(0, bufferSize);
			std::memcpy(dstData, srcData.data(), bufferSize);
			stagingBufferMemory.UnmapMemory();

            dstBuffer.CopyFromBuffer(commandPool, *stagingBuffer, bufferSize);
		}

		template<class T>
		static void CreateBuffer(xvk::XVKCommandPool& commandPool, const VkBufferUsageFlags usage,
			const std::vector<T>& data, std::unique_ptr<xvk::XVKBuffer>& buffer, std::unique_ptr<xvk::XVKDeviceMemory>& bufferMemory)
		{
			const xvk::XVKDevice& device = commandPool.GetDevice();
			VkDeviceSize bufferSize = sizeof(data[0]) * data.size();
			
			const VkMemoryAllocateFlags allocateFlags = usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
				? VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
				: 0;

			buffer.reset(new xvk::XVKBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage));
			bufferMemory.reset(new xvk::XVKDeviceMemory(buffer->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocateFlags)));

			CopyFromBuffer(commandPool, *buffer, data);
		}
	};


}