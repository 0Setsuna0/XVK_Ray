#include "ApplicationRT.h"
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
#include "Utility/BufferUtil.h"
#include <numeric>

namespace xvk::ray
{
	template <class TAccelerationStructure>
	VkAccelerationStructureBuildSizesInfoKHR GetTotalRequirements(const std::vector<TAccelerationStructure>& accelerationStructures)
	{
		VkAccelerationStructureBuildSizesInfoKHR total{};

		for (const auto& accelerationStructure : accelerationStructures)
		{
			total.accelerationStructureSize += accelerationStructure.GetBuildSizesInfo().accelerationStructureSize;
			total.buildScratchSize += accelerationStructure.GetBuildSizesInfo().buildScratchSize;
			total.updateScratchSize += accelerationStructure.GetBuildSizesInfo().updateScratchSize;
		}

		return total;
	}

	ApplicationRT::ApplicationRT(const WindowState& windowState, VkPresentModeKHR presentMode,
		bool enableValidationLayer)
		:Application(windowState, presentMode, enableValidationLayer)
	{
		m_deviceFunc.reset(new XVKRayFuncManager(GetDevice()));
		m_rayTracingContext.reset(new XVKRayTracingContext(GetDevice()));
	}

	ApplicationRT::~ApplicationRT()
	{
		Application::DeleteSwapChain();
		DeleteAccelerationStructures();

		m_rayTracingContext.reset();
		m_deviceFunc.reset();
	}

	void ApplicationRT::CreateAccelerationStructures()
	{
		XVKTransientCommands::Submit(GetCommandPool(), [this](VkCommandBuffer commandBuffer) {
			CreateBLAS(commandBuffer);
			CreateTLAS(commandBuffer);
			});

		m_tlasScratchBuffer.reset();
		m_tlasScratchBufferMemory.reset();
		m_blasScratchBuffer.reset();
		m_blasScratchBufferMemory.reset();
	}

	void ApplicationRT::DeleteAccelerationStructures()
	{
		m_tlas.clear();
		m_instancesBuffer.reset();
		m_intancesBufferMemory.reset();
		m_tlasScratchBuffer.reset();
		m_tlasScratchBufferMemory.reset();
		m_tlasBuffer.reset();
		m_tlasBufferMemory.reset();

		m_blas.clear();
		m_blasScratchBuffer.reset();
		m_blasScratchBufferMemory.reset();
		m_blasBuffer.reset();
		m_blasBufferMemory.reset();
	}

	void ApplicationRT::CreateSwapChain()
	{
		xvk::Application::CreateSwapChain();

		CreateOutputImage();

		m_rayTracingPipeline.reset(new XVKRayTracingPipeline(*m_deviceFunc, GetSwapChain(), m_tlas[0],
			*m_accumulatedImageView, *m_outputImageView, GetUniformBuffers(), *m_scene));

		std::vector<XVKShaderBindingManager::Entry> rayGenPrograms = { {m_rayTracingPipeline->GetRayGenShaderIndex(), {}} };
		std::vector<XVKShaderBindingManager::Entry> rayMissPrograms = { {m_rayTracingPipeline->GetRayMissShaderIndex(), {}} };
		std::vector<XVKShaderBindingManager::Entry> rayHitGroups = { {m_rayTracingPipeline->GetRayTriangleHitGroupShaderIndex(), {}} };

		m_shaderBindings.reset(new XVKShaderBindingManager(*m_deviceFunc, *m_rayTracingPipeline, *m_rayTracingContext,
			rayGenPrograms, rayMissPrograms, rayHitGroups));

	}

	void ApplicationRT::DeleteSwapChain()
	{
		m_shaderBindings.reset();
		m_rayTracingPipeline.reset();
		m_accumulatedImage.reset();
		m_accumulatedImageView.reset();
		m_accumulatedImageMemory.reset();
		m_outputImage.reset();
		m_outputImageView.reset();
		m_outputImageMemory.reset();

		xvk::Application::DeleteSwapChain();
	}

	void ApplicationRT::Render(VkCommandBuffer commandBuffer, size_t currentFrame, uint32_t imageIndex)
	{
		VkExtent2D extent = GetSwapChain().GetExtent();

		VkDescriptorSet descriptorSets[] = { m_rayTracingPipeline->GetDescriptorSet(currentFrame) };

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		//accquire images for rendering
		XVKImage::Insert(commandBuffer, m_accumulatedImage->Handle(), subresourceRange, 0,
			VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	
		XVKImage::Insert(commandBuffer, m_outputImage->Handle(), subresourceRange, 0,
			VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rayTracingPipeline->Handle());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rayTracingPipeline->GetPipelineLayout().Handle(),
			0, 1, descriptorSets, 0, nullptr);

		//describe the shader binding table
		VkStridedDeviceAddressRegionKHR rayGenShaderBindingTable = {};
		rayGenShaderBindingTable.deviceAddress = m_shaderBindings->RayGenDeviceAddress();
		rayGenShaderBindingTable.stride = m_shaderBindings->RayGenEntrySize();
		rayGenShaderBindingTable.size = m_shaderBindings->RayGenSize();

		VkStridedDeviceAddressRegionKHR missShaderBindingTable = {};
		missShaderBindingTable.deviceAddress = m_shaderBindings->RayMissDeviceAddress();
		missShaderBindingTable.stride = m_shaderBindings->MissEntrySize();
		missShaderBindingTable.size = m_shaderBindings->MissSize();

		VkStridedDeviceAddressRegionKHR hitShaderBindingTable = {};
		hitShaderBindingTable.deviceAddress = m_shaderBindings->RayHitGroupDeviceAddress();
		hitShaderBindingTable.stride = m_shaderBindings->HitGroupEntrySize();
		hitShaderBindingTable.size = m_shaderBindings->HitGroupSize();

		VkStridedDeviceAddressRegionKHR callableShaderBindingTable = {};

		m_deviceFunc->vkCmdTraceRaysKHR(commandBuffer, &rayGenShaderBindingTable,
			&missShaderBindingTable, &hitShaderBindingTable, &callableShaderBindingTable,
			extent.width, extent.height, 1);

		XVKImage::Insert(commandBuffer, m_outputImage->Handle(), subresourceRange,
			VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		XVKImage::Insert(commandBuffer, GetSwapChain().GetImages()[imageIndex], subresourceRange, 0,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Copy output image into swap-chain image.
		VkImageCopy copyRegion;
		copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.srcOffset = { 0, 0, 0 };
		copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.dstOffset = { 0, 0, 0 };
		copyRegion.extent = { extent.width, extent.height, 1 };

		vkCmdCopyImage(commandBuffer, m_outputImage->Handle(),
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, GetSwapChain().GetImages()[currentFrame],
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		XVKImage::Insert(commandBuffer, GetSwapChain().GetImages()[imageIndex], subresourceRange, 
			VK_ACCESS_TRANSFER_WRITE_BIT, 0, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}


	void ApplicationRT::CreateBLAS(VkCommandBuffer commandBuffer)
	{
		//blas: triangles->vertex buffers
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;

		for (const auto& model : m_scene->GetModels())
		{
			uint32_t vertexCount = model->NumVertices();
			uint32_t indexCount = model->NumIndices();
			XVKBLGeometry geometry;

			geometry.AddGeometryTriangles(*m_scene, vertexOffset, vertexCount, 
				indexOffset, indexCount, true);

			m_blas.emplace_back(*m_deviceFunc, *m_rayTracingContext, geometry);

			vertexOffset += vertexCount * sizeof(vkAsset::AVertex);
			indexOffset += indexCount * sizeof(uint32_t);
		}
		//allocate all geometry memory
		const auto total = GetTotalRequirements(m_blas);
		m_blasBuffer.reset(new XVKBuffer(GetDevice(), total.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT));
		m_blasBufferMemory.reset(new XVKDeviceMemory(m_blasBuffer->AllocateMemory(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		m_blasScratchBuffer.reset(new XVKBuffer(GetDevice(), total.buildScratchSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT));
		m_blasScratchBufferMemory.reset(new XVKDeviceMemory(m_blasScratchBuffer->AllocateMemory(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

		//generate structure
		VkDeviceSize scratchOffset = 0;
		VkDeviceSize resultOffset = 0;

		for (int i = 0; i < m_blas.size(); i++)
		{
			m_blas[i].GenerateBLAS(commandBuffer, *m_blasScratchBuffer, scratchOffset, *m_blasBuffer, resultOffset);
			
			resultOffset += m_blas[i].GetBuildSizesInfo().accelerationStructureSize;
			scratchOffset += m_blas[i].GetBuildSizesInfo().buildScratchSize;
		}
	}

	void ApplicationRT::CreateTLAS(VkCommandBuffer commandBuffer)
	{
		std::vector<VkAccelerationStructureInstanceKHR> instances;

		uint32_t instanceId = 0;

		for (const auto& model : m_scene->GetModels())
		{
			instances.push_back(XVKTLAccelerationStructure::CreateInstance(m_blas[instanceId],
				glm::mat4(1.0f), instanceId, 0));
			instanceId++;
		}

		utility::BufferUtility::CreateBuffer(GetCommandPool(), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			instances, m_instancesBuffer, m_intancesBufferMemory);

		xvk::ray::XVKAccelerationStructure::MemoryBarrierRT(commandBuffer);

		m_tlas.emplace_back(*m_deviceFunc, *m_rayTracingContext,
			m_instancesBuffer->GetDeviceAddress(), instances.size());

		//allocate all geometry memory
		const auto total = GetTotalRequirements(m_tlas);

		m_tlasBuffer.reset(new XVKBuffer(GetDevice(), total.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR));
		m_tlasBufferMemory.reset(new XVKDeviceMemory(m_tlasBuffer->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

		m_tlasScratchBuffer.reset(new XVKBuffer(GetDevice(), total.buildScratchSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT));
		m_tlasScratchBufferMemory.reset(new XVKDeviceMemory(m_tlasScratchBuffer->AllocateMemory(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

		// Generate the structures.
		m_tlas[0].GenerateTLAS(commandBuffer, *m_tlasScratchBuffer, 0, *m_tlasBuffer, 0);
	}
	void ApplicationRT::CreateOutputImage()
	{
		const auto extent = GetSwapChain().GetExtent();
		const auto format = GetSwapChain().GetImageFormat();
		const auto tiling = VK_IMAGE_TILING_OPTIMAL;

		m_accumulatedImage.reset(new XVKImage(GetDevice(), extent, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT));
		m_accumulatedImageMemory.reset(new XVKDeviceMemory(m_accumulatedImage->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		m_accumulatedImageView.reset(new XVKImageView(GetDevice(), m_accumulatedImage->Handle(), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT));

		m_outputImage.reset(new XVKImage(GetDevice(), extent, format, tiling, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
		m_outputImageMemory.reset(new XVKDeviceMemory(m_outputImage->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		m_outputImageView.reset(new XVKImageView(GetDevice(), m_outputImage->Handle(), format, VK_IMAGE_ASPECT_COLOR_BIT));
	}
}