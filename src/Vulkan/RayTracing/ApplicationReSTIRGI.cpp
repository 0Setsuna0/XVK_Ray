#include "ApplicationReSTIRGI.h"
#include "ReSTIRGIPipeline.h"
#include "XVKBLAccelerationStructure.h"
#include "XVKRayFuncManager.h"
#include "XVKRayTracingCore.h"
#include "XVKRayTracingPipeline.h"
#include "XVKShaderBindingManager.h"
#include "XVKTLAccelerationStructure.h"
#include "Asset/Model.h"
#include "Asset/Scene.h"
#include "Vulkan/XVKBuffer.h"
#include "Vulkan/XVKImage.h"
#include "Vulkan/XVKImageView.h"
#include "Vulkan/XVKPipelineLayout.h"
#include "Vulkan/XVKTransientCommand.h"
#include "Vulkan/XVKSwapChain.h"
#include "Vulkan/XVKDeviceMemory.h"
#include "Vulkan/XVKDevice.h"
#include "ReSTIRGICommon.h"
#include "Utility/BufferUtil.h"
#include <numeric>

namespace xvk::ray
{
	ApplicationReSTIRGI::ApplicationReSTIRGI(const WindowState& windowState, VkPresentModeKHR presentMode,
		bool enableValidationLayer)
		: ApplicationRT(windowState, presentMode, enableValidationLayer)
	{
	}

	ApplicationReSTIRGI::~ApplicationReSTIRGI()
	{
	}

	void ApplicationReSTIRGI::CreateSwapChain()
	{
		ApplicationRT::CreateSwapChain();

		InitReSTIRGITracer();
		m_restirgiPipeline.reset(new XVKReSTIRGIPipeline(*m_deviceFunc, GetSwapChain(), m_tlas[0],
			*m_accumulatedImageView, *m_outputImageView, GetUniformBuffers(), *m_scene, m_restir_initialSamplesBuffer, m_restir_initialOldSamplesBuffer,
			m_restir_temporalReservoirBuffer, m_restir_spatialReservoirBuffer));

		// === Initial Sample Pipeline ===
		{
			std::vector<XVKShaderBindingManager::Entry> rayGenPrograms = {
				{ m_restirgiPipeline->GetReSTIRGIInitialSampleShaderIndex(), {} }
			};
			std::vector<XVKShaderBindingManager::Entry> rayMissPrograms = {
				{ m_restirgiPipeline->GetRayMissShaderIndex(), {} },
				{ m_restirgiPipeline->GetRayShadowMissShaderIndex(), {} } // Èç¹ûÓÐ shadow miss
			};
			std::vector<XVKShaderBindingManager::Entry> rayHitGroups = {
				{ m_restirgiPipeline->GetRayTriangleHitGroupShaderIndex(), {} },
				{ m_restirgiPipeline->GetRayAnyHitShaderIndex(), {} }
			};

			m_initialSampleSBT.reset(new XVKShaderBindingManager(*m_deviceFunc, m_restirgiPipeline->InitialSampleHandle(), *m_rayTracingContext,
				rayGenPrograms, rayMissPrograms, rayHitGroups));
		}

		// === Temporal Reuse Pipeline ===
		{
			std::vector<XVKShaderBindingManager::Entry> rayGenPrograms = {
				{ m_restirgiPipeline->GetReSTIRGITemporalReuseShaderIndex(), {} }
			};
			std::vector<XVKShaderBindingManager::Entry> rayMissPrograms = {
				{ m_restirgiPipeline->GetRayMissShaderIndex(), {} },
				{ m_restirgiPipeline->GetRayShadowMissShaderIndex(), {} }
			};
			std::vector<XVKShaderBindingManager::Entry> rayHitGroups = {
				{ m_restirgiPipeline->GetRayTriangleHitGroupShaderIndex(), {} },
				{ m_restirgiPipeline->GetRayAnyHitShaderIndex(), {} }
			};

			m_temporalReuseSBT.reset(new XVKShaderBindingManager(*m_deviceFunc, m_restirgiPipeline->TemporalReuseHandle(), *m_rayTracingContext,
				rayGenPrograms, rayMissPrograms, rayHitGroups));
		}

		// === Spatial Reuse Pipeline ===
		{
			std::vector<XVKShaderBindingManager::Entry> rayGenPrograms = {
				{ m_restirgiPipeline->GetReSTIRGISpatialReuseShaderIndex(), {} }
			};
			std::vector<XVKShaderBindingManager::Entry> rayMissPrograms = {
				{ m_restirgiPipeline->GetRayMissShaderIndex(), {} },
				{ m_restirgiPipeline->GetRayShadowMissShaderIndex(), {} }
			};
			std::vector<XVKShaderBindingManager::Entry> rayHitGroups = {
				{ m_restirgiPipeline->GetRayTriangleHitGroupShaderIndex(), {} },
				{ m_restirgiPipeline->GetRayAnyHitShaderIndex(), {} }
			};

			m_spatialReuseSBT.reset(new XVKShaderBindingManager(*m_deviceFunc, m_restirgiPipeline->SpatialReuseHandle(), *m_rayTracingContext,
				rayGenPrograms, rayMissPrograms, rayHitGroups));
		}


	}

	void ApplicationReSTIRGI::DeleteSwapChain()
	{
		ApplicationRT::DeleteSwapChain();
	}

	void ApplicationReSTIRGI::Render(VkCommandBuffer commandBuffer, size_t currentFrame, uint32_t imageIndex)
	{
		
	}

	void ApplicationReSTIRGI::InitReSTIRGITracer()
	{
		// Initialize ReSTIRGI tracer resources here
		m_restir_initialSamplesBuffer.reset(new XVKBuffer(GetDevice(), 
			GetSwapChain().GetExtent().width * GetSwapChain().GetExtent().height * sizeof(ReSTIRGISample), 
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
		m_restir_initialSamplesBufferMemory.reset(new XVKDeviceMemory(m_restir_initialSamplesBuffer->AllocateMemory(
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)));

		m_restir_initialOldSamplesBuffer.reset(new XVKBuffer(GetDevice(),
			GetSwapChain().GetExtent().width * GetSwapChain().GetExtent().height * sizeof(ReSTIRGISample),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
		m_restir_initialOldSamplesBufferMemory.reset(new XVKDeviceMemory(m_restir_initialSamplesBuffer->AllocateMemory(
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)));

		m_restir_temporalReservoirBuffer.reset(new XVKBuffer(GetDevice(), 
			GetSwapChain().GetExtent().width * GetSwapChain().GetExtent().height * sizeof(ReSTIRGIReservoir),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT));
		m_restir_temporalReservoirBufferMemory.reset(new XVKDeviceMemory(m_restir_temporalReservoirBuffer->AllocateMemory(
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)));
	
		m_restir_spatialReservoirBuffer.reset(new XVKBuffer(GetDevice(),
			GetSwapChain().GetExtent().width * GetSwapChain().GetExtent().height * sizeof(ReSTIRGIReservoir),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT));
		m_restir_spatialReservoirBufferMemory.reset(new XVKDeviceMemory(m_restir_spatialReservoirBuffer->AllocateMemory(
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)));
	}
}