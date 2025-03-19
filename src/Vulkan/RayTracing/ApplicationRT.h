#pragma once
#include "Vulkan/ApplicationRZ.h"
#include "Vulkan/XVKFrameBuffer.h"
#include "XVKRayTracingCore.h"
#include "XVKRayFuncManager.h"

namespace xvk
{
	class XVKBuffer;
	class XVKDeviceMemory;
	class XVKImage;
	class XVKImageView;
	class XVKCommandBuffers;
}

namespace xvk::ray
{
	class XVKBLAccelerationStructure;
	class XVKTLAccelerationStructure;
	class XVKRayTracingPipeline;
	class XVKShaderBindingManager;

	class ApplicationRT : public Application
	{

	protected:

		void CreateAccelerationStructures();
		void DeleteAccelerationStructures();
		void CreateSwapChain() override;
		void DeleteSwapChain() override;
		void Render(VkCommandBuffer commandBuffer, size_t currentFrame, uint32_t imageIndex) override;
	
	public:
		
		ApplicationRT(const WindowState& windowState, VkPresentModeKHR presentMode,
			bool enableValidationLayer);
		~ApplicationRT();

	private:
		
		void CreateBLAS(VkCommandBuffer commandBuffer);
		void CreateTLAS(VkCommandBuffer commandBuffer);
		void CreateOutputImage();

		std::unique_ptr<XVKRayFuncManager> m_deviceFunc;
		std::unique_ptr<XVKRayTracingContext> m_rayTracingContext;

		std::vector<XVKBLAccelerationStructure> m_blas;
		std::unique_ptr<XVKBuffer> m_blasBuffer;
		std::unique_ptr<XVKDeviceMemory> m_blasBufferMemory;
		std::unique_ptr<XVKBuffer> m_blasScratchBuffer;
		std::unique_ptr<XVKDeviceMemory> m_blasScratchBufferMemory;

		std::vector<XVKTLAccelerationStructure> m_tlas;
		std::unique_ptr<XVKBuffer> m_tlasBuffer;
		std::unique_ptr<XVKDeviceMemory> m_tlasBufferMemory;
		std::unique_ptr<XVKBuffer> m_tlasScratchBuffer;
		std::unique_ptr<XVKDeviceMemory> m_tlasScratchBufferMemory;

		std::unique_ptr<XVKBuffer> m_instancesBuffer;
		std::unique_ptr<XVKDeviceMemory> m_intancesBufferMemory;

		std::unique_ptr<XVKImage> m_accumulatedImage;
		std::unique_ptr<XVKImageView> m_accumulatedImageView;
		std::unique_ptr<XVKDeviceMemory> m_accumulatedImageMemory;

		std::unique_ptr<XVKImage> m_outputImage;
		std::unique_ptr<XVKImageView> m_outputImageView;
		std::unique_ptr<XVKDeviceMemory> m_outputImageMemory;

		std::unique_ptr<XVKRayTracingPipeline> m_rayTracingPipeline;
		std::unique_ptr<XVKShaderBindingManager> m_shaderBindings;
	};
}

