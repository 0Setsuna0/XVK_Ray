#include "XVKRayTracingPipeline.h"
#include "XVKRayFuncManager.h"
#include "XVKTLAccelerationStructure.h"
#include "Asset/Scene.h"
#include "Asset/UniformBuffer.h"
#include "Vulkan/XVKBuffer.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKDescriptorSetManager.h"
#include "Vulkan/XVKDescriptorSetLayout.h"
#include "Vulkan/XVKDescriptorSet.h"
#include "Vulkan/XVKImageView.h"
#include "Vulkan/XVKPipelineLayout.h"
#include "Vulkan/XVKShaderModule.h"
#include "Vulkan/XVKSwapChain.h"

namespace xvk::ray
{
	XVKRayTracingPipeline::XVKRayTracingPipeline(
		const XVKRayFuncManager& deviceFunc,
		const XVKSwapChain& swapChain,
		const XVKTLAccelerationStructure& accelerationStructure,
		const XVKImageView& accumulatedImageView,
		const XVKImageView& outputImageView,
		const std::vector<vkAsset::AUniformBuffer>& uniformBuffers,
		const vkAsset::AScene& scene)
		:xvk_swapChain(swapChain)
	{
		// descriptor set
		const XVKDevice& device = xvk_swapChain.GetDevice();
		const std::vector<DescriptorBinding> descriptorBindings =
		{
			// Top level acceleration structure.
			{0, 1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR},

			// Image accumulation & output
			{1, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR},
			{2, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR},

			// Camera information & co
			{3, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR},

			// Vertex buffer, Index buffer, Material buffer, Offset buffer
			{4, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
			{5, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
			{6, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
			{7, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},

			// Textures and image samplers
			{8, static_cast<uint32_t>(scene.GetTextureSamplers().size()), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
		};

		xvk_descriptorSetManager.reset(new XVKDescriptorSetManager(device, descriptorBindings, uniformBuffers.size()));
	
		XVKDescriptorSets& descriptorSets = xvk_descriptorSetManager->GetDescriptorSets();
		
		//update descriptor
		for (uint32_t i = 0; i < xvk_swapChain.GetImageViews().size(); i++)
		{
			//tlas 
			const auto asHandle = accelerationStructure.Handle();
			VkWriteDescriptorSetAccelerationStructureKHR asInfo = {};
			asInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
			asInfo.pNext = nullptr;
			asInfo.accelerationStructureCount = 1;
			asInfo.pAccelerationStructures = &asHandle;

			//accumulation image
			VkDescriptorImageInfo accumulatedImageInfo = {};
			accumulatedImageInfo.imageView = accumulatedImageView.Handle();
			accumulatedImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			//output image
			VkDescriptorImageInfo outputImageInfo = {};
			outputImageInfo.imageView = outputImageView.Handle();
			outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			//uniform buffer
			VkDescriptorBufferInfo uniformBufferInfo = {};
			uniformBufferInfo.buffer = uniformBuffers[i].GetBuffer().Handle();
			uniformBufferInfo.range = VK_WHOLE_SIZE;

			//vertex buffer
			VkDescriptorBufferInfo vertexBufferInfo = {};
			vertexBufferInfo.buffer = scene.GetVertexBuffer().Handle();
			vertexBufferInfo.range = VK_WHOLE_SIZE;

			//index buffer
			VkDescriptorBufferInfo indexBufferInfo = {};
			indexBufferInfo.buffer = scene.GetIndexBuffer().Handle();
			indexBufferInfo.range = VK_WHOLE_SIZE;

			//material buffer
			VkDescriptorBufferInfo materialBufferInfo = {};
			materialBufferInfo.buffer = scene.GetMaterialBuffer().Handle();
			materialBufferInfo.range = VK_WHOLE_SIZE;

			//offsets buffer
			VkDescriptorBufferInfo offsetsBufferInfo = {};
			offsetsBufferInfo.buffer = scene.GetOffsetsBuffer().Handle();

			std::vector<VkDescriptorImageInfo> imageInfos(scene.GetTextureSamplers().size());

			for (uint32_t t = 0; t < imageInfos.size(); t++)
			{
				imageInfos[t].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfos[t].imageView = scene.GetTextureImageViews()[t];
				imageInfos[t].sampler = scene.GetTextureSamplers()[t];
			}

			std::vector<VkWriteDescriptorSet> descriptorWrites =
			{
				descriptorSets.Bind(i, 0, asInfo),
				descriptorSets.Bind(i, 1, accumulatedImageInfo),
				descriptorSets.Bind(i, 2, outputImageInfo),
				descriptorSets.Bind(i, 3, uniformBufferInfo),
				descriptorSets.Bind(i, 4, vertexBufferInfo),
				descriptorSets.Bind(i, 5, indexBufferInfo),
				descriptorSets.Bind(i, 6, materialBufferInfo),
				descriptorSets.Bind(i, 7, offsetsBufferInfo),
				descriptorSets.Bind(i, 8, *imageInfos.data(), imageInfos.size())
			};

			descriptorSets.UpdateDescriptors(descriptorWrites);
		}//end for

		xvk_pipelineLayout.reset(new XVKPipelineLayout(device, xvk_descriptorSetManager->GetDescriptorSetLayout()));
		
		//load shaders
		const XVKShaderModule rayGenShader(device, SHADER_DIR"HLSL/rgen.spv");
		const XVKShaderModule rayMissShader(device, SHADER_DIR"HLSL/rmiss.spv");
		const XVKShaderModule rayHitShader(device, SHADER_DIR"HLSL/rhit.spv");

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages =
		{
			rayGenShader.CreateShaderStage(VK_SHADER_STAGE_RAYGEN_BIT_KHR),
			rayMissShader.CreateShaderStage(VK_SHADER_STAGE_MISS_BIT_KHR),
			rayHitShader.CreateShaderStage(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
		};

		VkRayTracingShaderGroupCreateInfoKHR rayGenGroupInfo = {};
		rayGenGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		rayGenGroupInfo.pNext = nullptr;
		rayGenGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		rayGenGroupInfo.generalShader = 0;
		rayGenGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		rayGenGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		rayGenGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		rayGenIndex = 0;

		VkRayTracingShaderGroupCreateInfoKHR missGroupInfo = {};
		missGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		missGroupInfo.pNext = nullptr;
		missGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		missGroupInfo.generalShader = 1;
		missGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		missGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		missGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		rayMissIndex = 1;

		VkRayTracingShaderGroupCreateInfoKHR triangleHitGroupInfo = {};
		triangleHitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		triangleHitGroupInfo.pNext = nullptr;
		triangleHitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		triangleHitGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
		triangleHitGroupInfo.closestHitShader = 2;
		triangleHitGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		triangleHitGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		rayTriangleHitGroupIndex = 2;

		std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups =
		{
			rayGenGroupInfo,
			missGroupInfo,
			triangleHitGroupInfo,
		};

		// Create graphic pipeline
		VkRayTracingPipelineCreateInfoKHR pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		pipelineInfo.pNext = nullptr;
		pipelineInfo.flags = 0;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.groupCount = static_cast<uint32_t>(groups.size());
		pipelineInfo.pGroups = groups.data();
		pipelineInfo.maxPipelineRayRecursionDepth = 1;
		pipelineInfo.layout = xvk_pipelineLayout->Handle();
		pipelineInfo.basePipelineHandle = nullptr;
		pipelineInfo.basePipelineIndex = 0;

		VULKAN_RUNTIME_CHECK(deviceFunc.vkCreateRayTracingPipelinesKHR(device.Handle(), nullptr,
			nullptr, 1, &pipelineInfo, nullptr, &vk_rayTracingPipeline), "create ray tracing pipeline");
	}


	XVKRayTracingPipeline::~XVKRayTracingPipeline()
	{
		if (vk_rayTracingPipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(xvk_swapChain.GetDevice().Handle(), vk_rayTracingPipeline, nullptr);
			vk_rayTracingPipeline = VK_NULL_HANDLE;
		}

		xvk_pipelineLayout.reset();
		xvk_descriptorSetManager.reset();
	}

	VkDescriptorSet XVKRayTracingPipeline::GetDescriptorSet(uint32_t index) const
	{
		return xvk_descriptorSetManager->GetDescriptorSets().Handle(index);
	}
}