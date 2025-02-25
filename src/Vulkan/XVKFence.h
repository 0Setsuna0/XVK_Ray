#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKDevice;

	class XVKFence
	{
	public:
		XVKFence(const XVKDevice& device, bool signaled);
		XVKFence(XVKFence&& other) noexcept;
		~XVKFence();

		void Reset();
		void Wait(const uint64_t timeout) const;

		const VkFence& Handle() const { return vk_fence; }
		const XVKDevice& GetDevice() const { xvk_device; }
	private:
		VkFence vk_fence;
		
		const XVKDevice& xvk_device;
	};
}