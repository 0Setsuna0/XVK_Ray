#pragma once
#include "XVKCommon.h"
#include "XVKDevice.h"
#include "XVKCommandPool.h"
#include "XVKCommandBuffers.h"
#include <functional>
namespace xvk
{
	class XVKTransientCommands
	{
	public:
		static void Submit(XVKCommandPool& commandPool, const std::function<void(VkCommandBuffer)>& func)
		{
			XVKCommandBuffers commandBuffers(commandPool, 1);
			//begin recording
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(commandBuffers[0], &beginInfo);

			func(commandBuffers[0]);
			//end recording
			vkEndCommandBuffer(commandBuffers[0]);

			//submit
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffers[0];

			vkQueueSubmit(commandPool.GetDevice().GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(commandPool.GetDevice().GraphicsQueue());
		}

	};

}