#pragma once

#include "Vulkan/XVKCommon.h"
#include <memory>

namespace vkAsset
{
	class AScene;
	class AUniformBuffer;
}

namespace xvk
{
	class XVKImageView;
	class XVKPipelineLayout;
	class XVKDescriptorSetManager;
	class XVKSwapChain;
}

namespace xvk::ray
{
	class XVKRayFuncManager;
	class XVKTLAccelerationStructure;

	class XVKRayTracingPipeline
	{
	public:

		XVKRayTracingPipeline(
			const XVKRayFuncManager& deviceFunc,
			const XVKSwapChain& swapChain,
			const XVKTLAccelerationStructure& accelerationStructure,
			const XVKImageView& accumulatedImageView,
			const XVKImageView& outputImageView,
			const std::vector<vkAsset::AUniformBuffer>& uniformBuffers,
			const vkAsset::AScene& scene
		);
		~XVKRayTracingPipeline();
			
		VkPipeline Handle() const { return vk_rayTracingPipeline; }
		
		uint32_t GetRayGenShaderIndex() const { return rayGenIndex; }
		uint32_t GetRayMissShaderIndex() const { return rayMissIndex; }
		uint32_t GetRayTriangleHitGroupShaderIndex() const { return rayTriangleHitGroupIndex; }

		VkDescriptorSet GetDescriptorSet(uint32_t index) const;
		const XVKPipelineLayout& GetPipelineLayout() const { return *xvk_pipelineLayout; }

	private:

		const XVKSwapChain& xvk_swapChain;
		VkPipeline vk_rayTracingPipeline;

		std::unique_ptr<XVKDescriptorSetManager> xvk_descriptorSetManager;
		std::unique_ptr<XVKPipelineLayout> xvk_pipelineLayout;

		uint32_t rayGenIndex;
		uint32_t rayMissIndex;
		uint32_t rayTriangleHitGroupIndex;
	};
}