#pragma once

#include "XVKCommon.h"
#include <memory>

namespace vkAsset
{
	class AScene;
	class AUniformBuffer;
}

namespace xvk
{
	class XVKInstance;
	class XVKDevice;
	class XVKSwapChain;
	class XVKRenderPass;
	class XVKPipelineLayout;
	class XVKDescriptorSetManager;
	class XVKDepthBuffer;
	class XVKGraphicsPipeline
	{
	public:
		XVKGraphicsPipeline(const XVKDevice& device,
			const XVKSwapChain& swapChain,
			const XVKDepthBuffer& depthBuffer,
			const std::vector<vkAsset::AUniformBuffer>& uniformBuffers,
			const vkAsset::AScene& scene);
		~XVKGraphicsPipeline();

		VkPipeline Handle() const { return vk_graphicsPipeline; }
	
		VkDescriptorSet GetDescriptorSet(size_t index) const;

	private:
		VkPipeline vk_graphicsPipeline;

		const XVKSwapChain& xvk_swapChain;
		const bool isWireFrame = false;

		std::unique_ptr<XVKPipelineLayout> xvk_pipelineLayout;
		std::unique_ptr<XVKDescriptorSetManager> xvk_descriptorSetManager;
		std::unique_ptr<XVKRenderPass> xvk_renderPass;
	};
}