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
		ApplicationReSTIRGI::DeleteSwapChain();
	}

	void ApplicationReSTIRGI::CreateSwapChain()
	{
		ApplicationRT::CreateSwapChain();

		InitReSTIRGITracer();

		m_restirgiPipeline.reset(new XVKReSTIRGIPipeline(*m_deviceFunc, GetSwapChain(), m_tlas[0],
			*m_restir_accumulatedImageView, *m_restir_outputImageView, GetUniformBuffers(), *m_scene, m_restir_initialSamplesBuffer, m_restir_initialOldSamplesBuffer,
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
		m_initialSampleSBT.reset();
		m_temporalReuseSBT.reset();
		m_spatialReuseSBT.reset();
		m_restirgiPipeline.reset();
		m_restir_initialSamplesBuffer.reset();
		m_restir_initialSamplesBufferMemory.reset();
		m_restir_initialOldSamplesBuffer.reset();
		m_restir_initialOldSamplesBufferMemory.reset();
		m_restir_temporalReservoirBuffer.reset();
		m_restir_temporalReservoirBufferMemory.reset();
		m_restir_spatialReservoirBuffer.reset();
		m_restir_spatialReservoirBufferMemory.reset();
		m_restir_accumulatedImage.reset();
		m_restir_accumulatedImageView.reset();
		m_restir_accumulatedImageMemory.reset();
		m_restir_outputImage.reset();
		m_restir_outputImageView.reset();
		m_restir_outputImageMemory.reset();
		ApplicationRT::DeleteSwapChain();
	}

	void ApplicationReSTIRGI::Render(VkCommandBuffer commandBuffer, size_t currentFrame, uint32_t imageIndex)
	{
		VkExtent2D extent = GetSwapChain().GetExtent();
		VkDescriptorSet descriptorSets[] = { m_restirgiPipeline->GetDescriptorSet(currentFrame) };

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		// Transition images for rendering
		XVKImage::Insert(commandBuffer, m_restir_accumulatedImage->Handle(), subresourceRange, 0,
			VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		XVKImage::Insert(commandBuffer, m_restir_outputImage->Handle(), subresourceRange, 0,
			VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		// === Initial Sample Pass ===
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			m_restirgiPipeline->InitialSampleHandle());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			m_restirgiPipeline->GetPipelineLayout().Handle(), 0, 1, descriptorSets, 0, nullptr);
		auto initialSampleSBTRegions = GetShaderBindingTableRegions(m_initialSampleSBT);
		m_deviceFunc->vkCmdTraceRaysKHR(commandBuffer,
			&initialSampleSBTRegions[0], &initialSampleSBTRegions[1],
			&initialSampleSBTRegions[2], &initialSampleSBTRegions[3],
			extent.width, extent.height, 1);

		// === Barrier: Initial Sample ¡ú Temporal Reuse ===
		{
			VkMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				0, 1, &barrier, 0, nullptr, 0, nullptr);
		}

		// === Temporal Reuse Pass ===
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			m_restirgiPipeline->TemporalReuseHandle());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			m_restirgiPipeline->GetPipelineLayout().Handle(), 0, 1, descriptorSets, 0, nullptr);
		auto temporalReuseSBTRegions = GetShaderBindingTableRegions(m_temporalReuseSBT);
		m_deviceFunc->vkCmdTraceRaysKHR(commandBuffer,
			&temporalReuseSBTRegions[0], &temporalReuseSBTRegions[1],
			&temporalReuseSBTRegions[2], &temporalReuseSBTRegions[3],
			extent.width, extent.height, 1);

		// === Barrier: Temporal Reuse ¡ú Spatial Reuse ===
		{
			VkMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			
			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				0, 1, &barrier, 0, nullptr, 0, nullptr);
		}

		// === Spatial Reuse Pass ===
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			m_restirgiPipeline->SpatialReuseHandle());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			m_restirgiPipeline->GetPipelineLayout().Handle(), 0, 1, descriptorSets, 0, nullptr);
		auto spatialReuseSBTRegions = GetShaderBindingTableRegions(m_spatialReuseSBT);
		m_deviceFunc->vkCmdTraceRaysKHR(commandBuffer,
			&spatialReuseSBTRegions[0], &spatialReuseSBTRegions[1],
			&spatialReuseSBTRegions[2], &spatialReuseSBTRegions[3],
			extent.width, extent.height, 1);

		//// === Barrier: Spatial Reuse (RT) ¡ú Output (Compute) ===
		//{
		//	VkMemoryBarrier barrier{};
		//	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		//	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		//	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//	vkCmdPipelineBarrier(commandBuffer,
		//		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		//		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		//		0, 1, &barrier, 0, nullptr, 0, nullptr);
		//}

		//// === Output Image Resolve (Compute) ===
		//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		//	m_restirgiPipeline->OutputHandle());
		//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		//	m_restirgiPipeline->GetPipelineLayout().Handle(), 0, 1, descriptorSets, 0, nullptr);
		//vkCmdDispatch(commandBuffer, (extent.width + 7) / 8, (extent.height + 7) / 8, 1);

		// Transition images for presenting
		XVKImage::Insert(commandBuffer, m_restir_outputImage->Handle(), subresourceRange,
			VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		XVKImage::Insert(commandBuffer, GetSwapChain().GetImages()[imageIndex], subresourceRange, 0,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Copy result to swapchain image
		VkImageCopy copyRegion{};
		copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.srcOffset = { 0, 0, 0 };
		copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.dstOffset = { 0, 0, 0 };
		copyRegion.extent = { extent.width, extent.height, 1 };

		vkCmdCopyImage(commandBuffer,
			m_restir_outputImage->Handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			GetSwapChain().GetImages()[currentFrame], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegion);

		XVKImage::Insert(commandBuffer, GetSwapChain().GetImages()[imageIndex], subresourceRange,
			VK_ACCESS_TRANSFER_WRITE_BIT, 0,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	std::vector<VkStridedDeviceAddressRegionKHR> ApplicationReSTIRGI::GetShaderBindingTableRegions(
		const std::unique_ptr<XVKShaderBindingManager>& sbt) const
	{
		std::vector<VkStridedDeviceAddressRegionKHR> sbtRegions;
		sbtRegions.resize(4);

		//ray gen
		sbtRegions[0].deviceAddress = sbt->RayGenDeviceAddress();
		sbtRegions[0].stride = sbt->RayGenEntrySize();
		sbtRegions[0].size = sbt->RayGenEntrySize();

		//ray miss
		sbtRegions[1].deviceAddress = sbt->RayMissDeviceAddress();
		sbtRegions[1].stride = sbt->MissEntrySize();
		sbtRegions[1].size = sbt->MissSize();

		//ray hit
		sbtRegions[2].deviceAddress = sbt->RayHitGroupDeviceAddress();
		sbtRegions[2].stride = sbt->HitGroupEntrySize();
		sbtRegions[2].size = sbt->HitGroupSize();

		//ray callable
		sbtRegions[3] = {};

		return sbtRegions;
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
		m_restir_initialOldSamplesBufferMemory.reset(new XVKDeviceMemory(m_restir_initialOldSamplesBuffer->AllocateMemory(
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
	
		const auto extent = GetSwapChain().GetExtent();
		const auto format = GetSwapChain().GetImageFormat();
		const auto tiling = VK_IMAGE_TILING_OPTIMAL;

		m_restir_accumulatedImage.reset(new XVKImage(GetDevice(), extent, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT));
		m_restir_accumulatedImageMemory.reset(new XVKDeviceMemory(m_restir_accumulatedImage->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		m_restir_accumulatedImageView.reset(new XVKImageView(GetDevice(), m_restir_accumulatedImage->Handle(), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT));

		m_restir_outputImage.reset(new XVKImage(GetDevice(), extent, format, tiling, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
		m_restir_outputImageMemory.reset(new XVKDeviceMemory(m_restir_outputImage->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		m_restir_outputImageView.reset(new XVKImageView(GetDevice(), m_restir_outputImage->Handle(), format, VK_IMAGE_ASPECT_COLOR_BIT));
	}
}