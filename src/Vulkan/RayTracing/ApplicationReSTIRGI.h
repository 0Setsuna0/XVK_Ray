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

		std::unique_ptr<XVKImage> m_restir_GBufferPos;
		std::unique_ptr<XVKImageView> m_restir_GBufferPosView;
		std::unique_ptr<XVKDeviceMemory> m_restir_GBufferPosMemory;

		std::unique_ptr<XVKImage> m_restir_GBufferNrm;
		std::unique_ptr<XVKImageView> m_restir_GBufferNrmView;
		std::unique_ptr<XVKDeviceMemory> m_restir_GBufferNrmMemory;

		std::unique_ptr<XVKImage> m_restir_GBufferMatUV;
		std::unique_ptr<XVKImageView> m_restir_GBufferMatUVView;
		std::unique_ptr<XVKDeviceMemory> m_restir_GBufferMatUVMemory;

		std::unique_ptr<XVKImage> m_restir_GBufferMotion;
		std::unique_ptr<XVKImageView> m_restir_GBufferMotionView;
		std::unique_ptr<XVKDeviceMemory> m_restir_GBufferMotionMemory;

		std::unique_ptr<XVKImage> m_restir_GBufferPosPrev;
		std::unique_ptr<XVKImageView> m_restir_GBufferPosPrevView;
		std::unique_ptr<XVKDeviceMemory> m_restir_GBufferPosPrevMemory;

		std::unique_ptr<XVKImage> m_restir_GBufferNrmPrev;
		std::unique_ptr<XVKImageView> m_restir_GBufferNrmPrevView;
		std::unique_ptr<XVKDeviceMemory> m_restir_GBufferNrmPrevMemory;

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