#pragma once
#include "XVKCommon.h"

namespace xvk
{
	class XVKDevice;

	class XVKSemaphore
	{
	public:
		XVKSemaphore(const XVKDevice& device);
		XVKSemaphore(XVKSemaphore&& other) noexcept;
		~XVKSemaphore();

		VkSemaphore Handle() const { return vk_semaphore; }
		const XVKDevice& GetDevice() const { return xvk_device; }
	private:
		VkSemaphore vk_semaphore;

		const XVKDevice& xvk_device;
	};
}