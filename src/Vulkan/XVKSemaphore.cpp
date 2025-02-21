#include "XVKSemaphore.h"
#include "XVKDevice.h"

namespace xvk
{
	XVKSemaphore::XVKSemaphore(const XVKDevice& device)
		:xvk_device(device)
	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VULKAN_RUNTIME_CHECK(vkCreateSemaphore(xvk_device.Handle(), &semaphoreInfo, nullptr, &vk_semaphore),
			"create semaphore");
	}

	XVKSemaphore::XVKSemaphore(XVKSemaphore&& other) noexcept
		:xvk_device(other.xvk_device),
		vk_semaphore(other.vk_semaphore)
	{
		other.vk_semaphore = VK_NULL_HANDLE;
	}

	XVKSemaphore::~XVKSemaphore()
	{
		if (vk_semaphore != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(xvk_device.Handle(), vk_semaphore, nullptr);
			vk_semaphore = VK_NULL_HANDLE;
		}
	}
}