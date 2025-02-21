#include "XVKFence.h"
#include "XVKDevice.h"

namespace xvk
{
	XVKFence::XVKFence(const XVKDevice& device, bool signaled)
		:xvk_device(device)
	{
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		VULKAN_RUNTIME_CHECK(vkCreateFence(xvk_device.Handle(), &fenceInfo, nullptr, &vk_fence),
			"create fence");
	}

	XVKFence::XVKFence(XVKFence&& other) noexcept
		:xvk_device(other.xvk_device),
		vk_fence(other.vk_fence)
	{
		vk_fence = VK_NULL_HANDLE;
	}

	XVKFence::~XVKFence()
	{
		if (vk_fence != VK_NULL_HANDLE)
		{
			vkDestroyFence(xvk_device.Handle(), vk_fence, nullptr);
			vk_fence = VK_NULL_HANDLE;
		}
	}

	void XVKFence::Reset()
	{
		VULKAN_RUNTIME_CHECK(vkResetFences(xvk_device.Handle(), 1, &vk_fence),
			"reset fence");
	}

	void XVKFence::Wait(const uint64_t timeout) const
	{
		VULKAN_RUNTIME_CHECK(vkWaitForFences(xvk_device.Handle(), 1, &vk_fence, VK_TRUE, timeout),
			"wait fence");
	}
}