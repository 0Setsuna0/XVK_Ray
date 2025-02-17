#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKDevice;
	class XVKCommandPool;

	class XVKCommandBuffers
	{
	public:
		XVKCommandBuffers(const XVKCommandPool& commandPool, uint32_t size);
		~XVKCommandBuffers();

		uint32_t Size() const { return static_cast<uint32_t>(vk_commandBuffers.size()); }
		VkCommandBuffer& operator[](const size_t index) { return vk_commandBuffers[index]; }

		VkCommandBuffer Begin(size_t i);
		void End(size_t i);	

	private:
		std::vector<VkCommandBuffer> vk_commandBuffers;
		const XVKDevice& xvk_device;
		const XVKCommandPool& xvk_commandPool;
	};
}