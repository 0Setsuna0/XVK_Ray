#include "XVKDeviceMemory.h"
#include "XVKDevice.h"

namespace xvk
{
	XVKDeviceMemory::XVKDeviceMemory(const XVKDevice& device, size_t size, uint32_t memoryTypeBits,
		VkMemoryAllocateFlags allocateFlags, VkMemoryPropertyFlags propertyFlags)
		:xvk_device(device)
	{
		VkMemoryAllocateFlagsInfo flagsInfo = {};
		flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		flagsInfo.flags = allocateFlags;
		flagsInfo.pNext = nullptr;

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = size;
		allocInfo.pNext = &flagsInfo;
		allocInfo.memoryTypeIndex = FindMemoryType(memoryTypeBits, propertyFlags);

		VULKAN_RUNTIME_CHECK(vkAllocateMemory(device.Handle(), &allocInfo, nullptr, &vk_deviceMemory), "allocate memory");
	}

	XVKDeviceMemory::~XVKDeviceMemory()
	{
		if (vk_deviceMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(xvk_device.Handle(), vk_deviceMemory, nullptr);
			vk_deviceMemory = VK_NULL_HANDLE;
		}
	}

	void* XVKDeviceMemory::MapMemory(size_t offset, size_t size)
	{
		void* data;
		VULKAN_RUNTIME_CHECK(vkMapMemory(xvk_device.Handle(), vk_deviceMemory, offset, size, 0, &data), "map memory");
		return data;
	}

	void XVKDeviceMemory::UnmapMemory()
	{
		vkUnmapMemory(xvk_device.Handle(), vk_deviceMemory);
	}

	uint32_t XVKDeviceMemory::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(xvk_device.PhysicalHandle(), &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		RUNTIME_ERROR("Failed to find suitable memory type");
	}
}