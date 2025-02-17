#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKDevice;

	class XVKCommandPool
	{
	public:
		XVKCommandPool(const XVKDevice& device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);
		~XVKCommandPool();

		VkCommandPool Handle() const { return vk_commandPool; }
		const XVKDevice& GetDevice() const { return xvk_device; }

	private:
		VkCommandPool vk_commandPool;
		const XVKDevice& xvk_device;
	};
}