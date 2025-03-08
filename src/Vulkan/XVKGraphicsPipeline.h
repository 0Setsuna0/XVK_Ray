#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKInstance;
	class XVKDevice;
	class XVKSwapChain;
	class XVKRenderPass;
	class XVKPipelineLayout;

	class XVKGraphicsPipeline
	{
	public:
		XVKGraphicsPipeline(const XVKDevice& device, const XVKSwapChain& swapChain, const VkRenderPass renderPass);
		~XVKGraphicsPipeline();

		VkPipeline Handle() const { return vk_graphicsPipeline; }
	

	private:
		VkPipeline vk_graphicsPipeline;

		const XVKSwapChain& xvk_swapChain;
		const bool isWireFrame = false;

		std::unique_ptr<XVKPipelineLayout> xvk_pipelineLayout;
	};
}