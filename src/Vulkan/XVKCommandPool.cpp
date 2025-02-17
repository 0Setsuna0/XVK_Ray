#include "XVKCommandPool.h"
#include "XVKDevice.h"

namespace xvk
{
	XVKCommandPool::XVKCommandPool(const XVKDevice& device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
		: xvk_device(device)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = flags;//default is VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT

		VULKAN_RUNTIME_CHECK(vkCreateCommandPool(xvk_device.Handle(), &poolInfo, nullptr, &vk_commandPool), "create command pool");
	}

	XVKCommandPool::~XVKCommandPool()
	{
		if (vk_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(xvk_device.Handle(), vk_commandPool, nullptr);
			vk_commandPool = VK_NULL_HANDLE;
		}
	}
}