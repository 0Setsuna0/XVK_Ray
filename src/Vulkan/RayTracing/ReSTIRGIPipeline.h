#pragma once
#include "Vulkan/XVKCommon.h"

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
	class XVKBuffer;
}

namespace xvk::ray
{
	class XVKRayFuncManager;
	class XVKTLAccelerationStructure;

	class XVKReSTIRGIPipeline
	{
	public:
		XVKReSTIRGIPipeline(
			const XVKRayFuncManager& deviceFunc,
			const XVKSwapChain& swapChain,
			const XVKTLAccelerationStructure& accelerationStructure,
			const XVKImageView& accumulatedImageView,
			const XVKImageView& outputImageView,
			const std::vector<vkAsset::AUniformBuffer>& uniformBuffers,
			const vkAsset::AScene& scene,
			const std::unique_ptr<XVKBuffer>& initialSampleBuffer,
			const std::unique_ptr<XVKBuffer>& initialOldSampleBuffer,
			const std::unique_ptr<XVKBuffer>& temporalResevoirBuffer,
			const std::unique_ptr<XVKBuffer>& spatialReservoirBuffer
			);

		~XVKReSTIRGIPipeline();

		VkPipeline InitialSampleHandle() const { return vk_restirGI_initialSamplePipeline; }
		VkPipeline TemporalReuseHandle() const { return vk_restirGI_temporalReusePipeline; }
		VkPipeline SpatialReuseHandle() const { return vk_restirGI_spatialReusePipeline; }
		VkPipeline OutputHandle() const { return vk_restirGI_outputPipeline; }

		uint32_t GetReSTIRGIInitialSampleShaderIndex() const { return restirgiInitialSampleIndex; }
		uint32_t GetReSTIRGITemporalReuseShaderIndex() const { return restirgiTemporalReuseIndex; }
		uint32_t GetReSTIRGISpatialReuseShaderIndex() const { return restirgiSpatialReuseIndex; }
		uint32_t GetReSTIRGIOutputShaderIndex() const { return restirgiOutputIndex; }
		uint32_t GetRayMissShaderIndex() const { return rayMissIndex; }
		uint32_t GetRayShadowMissShaderIndex() const { return rayShadowMissIndex; }
		uint32_t GetRayTriangleHitGroupShaderIndex() const { return rayTriangleHitGroupIndex; }
		uint32_t GetRayAnyHitShaderIndex() const { return rayAnyHitIndex; }

		VkDescriptorSet GetDescriptorSet(uint32_t index) const;
		const XVKPipelineLayout& GetPipelineLayout() const { return *xvk_pipelineLayout; }
	
	private:
		const XVKSwapChain& xvk_swapChain;

		VkPipeline vk_restirGI_initialSamplePipeline;
		VkPipeline vk_restirGI_temporalReusePipeline;
		VkPipeline vk_restirGI_spatialReusePipeline;
		VkPipeline vk_restirGI_outputPipeline;

		std::unique_ptr<XVKDescriptorSetManager> xvk_descriptorSetManager;
		std::unique_ptr<XVKPipelineLayout> xvk_pipelineLayout;

		uint32_t restirgiInitialSampleIndex;
		uint32_t restirgiTemporalReuseIndex;
		uint32_t restirgiSpatialReuseIndex;
		uint32_t restirgiOutputIndex;
		uint32_t rayMissIndex;
		uint32_t rayTriangleHitGroupIndex;
		uint32_t rayShadowMissIndex;
		uint32_t rayAnyHitIndex;
	};
}