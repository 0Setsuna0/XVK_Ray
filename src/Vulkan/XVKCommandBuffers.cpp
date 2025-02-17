#include "XVKCommandBuffers.h"
#include "XVKCommandPool.h"
#include "XVKDevice.h"

namespace xvk
{
	XVKCommandBuffers::XVKCommandBuffers(const XVKCommandPool& commandPool, uint32_t size)
		: xvk_device(commandPool.GetDevice()), xvk_commandPool(commandPool)
	{
		vk_commandBuffers.resize(size);

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool.Handle();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = size;

		VULKAN_RUNTIME_CHECK(vkAllocateCommandBuffers(xvk_device.Handle(), &allocInfo, vk_commandBuffers.data()), "allocate command buffers");
	}

	XVKCommandBuffers::~XVKCommandBuffers()
	{
		if (!vk_commandBuffers.empty())
		{
			vkFreeCommandBuffers(xvk_device.Handle(), xvk_commandPool.Handle(), static_cast<uint32_t>(vk_commandBuffers.size()), vk_commandBuffers.data());
			vk_commandBuffers.clear();
		}
	}

	VkCommandBuffer XVKCommandBuffers::Begin(size_t i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		VULKAN_RUNTIME_CHECK(vkBeginCommandBuffer(vk_commandBuffers[i], &beginInfo), "begin command buffer");
		return vk_commandBuffers[i];
	}

	void XVKCommandBuffers::End(size_t i)
	{
		VULKAN_RUNTIME_CHECK(vkEndCommandBuffer(vk_commandBuffers[i]), "end command buffer");
	}
}