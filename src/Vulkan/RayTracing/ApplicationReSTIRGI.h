#pragma once
#include "ApplicationRT.h"
#include "Vulkan/XVKBuffer.h"

namespace xvk::ray
{
	class XVKReSTIRGIPipeline;

	class ApplicationReSTIRGI : public ApplicationRT
	{
	protected:

		void InitReSTIRGITracer();

		void CreateSwapChain() override;
		void DeleteSwapChain() override;
		void Render(VkCommandBuffer commandBuffer, size_t currentFrame, uint32_t imageIndex) override;
	
	public:
		ApplicationReSTIRGI(const WindowState& windowState, VkPresentModeKHR presentMode,
			bool enableValidationLayer);
		~ApplicationReSTIRGI();

	private:
		std::vector<VkStridedDeviceAddressRegionKHR> GetShaderBindingTableRegions(
			const std::unique_ptr<XVKShaderBindingManager>& sbt) const;

		std::unique_ptr<XVKBuffer> m_restir_initialSamplesBuffer;
		std::unique_ptr<XVKDeviceMemory> m_restir_initialSamplesBufferMemory;

		std::unique_ptr<XVKBuffer> m_restir_initialOldSamplesBuffer;
		std::unique_ptr<XVKDeviceMemory> m_restir_initialOldSamplesBufferMemory;

		std::unique_ptr<XVKBuffer> m_restir_temporalReservoirBuffer;
		std::unique_ptr<XVKDeviceMemory> m_restir_temporalReservoirBufferMemory;

		std::unique_ptr<XVKBuffer> m_restir_spatialReservoirBuffer;
		std::unique_ptr<XVKDeviceMemory> m_restir_spatialReservoirBufferMemory;

		std::unique_ptr<XVKImage> m_restir_accumulatedImage;
		std::unique_ptr<XVKImageView> m_restir_accumulatedImageView;
		std::unique_ptr<XVKDeviceMemory> m_restir_accumulatedImageMemory;

		std::unique_ptr<XVKImage> m_restir_outputImage;
		std::unique_ptr<XVKImageView> m_restir_outputImageView;
		std::unique_ptr<XVKDeviceMemory> m_restir_outputImageMemory;

		std::unique_ptr<XVKReSTIRGIPipeline> m_restirgiPipeline;

		std::unique_ptr<XVKShaderBindingManager> m_initialSampleSBT;
		std::unique_ptr<XVKShaderBindingManager> m_temporalReuseSBT;
		std::unique_ptr<XVKShaderBindingManager> m_spatialReuseSBT;
	};
}