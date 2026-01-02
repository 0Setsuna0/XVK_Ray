#include "ReSTIRGIPipeline.h"
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
	XVKReSTIRGIPipeline::XVKReSTIRGIPipeline(const XVKRayFuncManager& deviceFunc,
		const XVKSwapChain& swapChain,
		const XVKTLAccelerationStructure& accelerationStructure,
		const XVKImageView& accumulatedImageView,
		const XVKImageView& outputImageView,
		const std::vector<vkAsset::AUniformBuffer>& uniformBuffers,
		const vkAsset::AScene& scene,
		const std::unique_ptr<XVKBuffer>& initialSampleBuffer,
		const std::unique_ptr<XVKBuffer>& initialOldSampleBuffer,
		const std::unique_ptr<XVKBuffer>& temporalResevoirBuffer,
		const std::unique_ptr<XVKBuffer>& spatialReservoirBuffer,
		const XVKImageView& posGBuffer,
		const XVKImageView& nrmGBuffer,
		const XVKImageView& matUVGBuffer,
		const XVKImageView& motionGBuffer,
		const XVKImageView& posGBufferPrev,
		const XVKImageView& nrmGBufferPrev
		)
		:xvk_swapChain(swapChain)
	{
		// descriptor set
		const XVKDevice& device = xvk_swapChain.GetDevice();
		const std::vector<DescriptorBinding> descriptorBindings = 
		{
			// TLAS
			{0, 1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR},

			// Image accumulation & output
			{1, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR},
			{2, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR},

			// Camera information & co
			{3, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR},

			// Vertex buffer, Index buffer, Material buffer, Offset buffer
			{4, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
			{5, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
			{6, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
			{7, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},

			// Textures and image samplers
			{8, static_cast<uint32_t>(scene.GetTextureSamplers().size()), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
				VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},

			// ReSTIR GI
			{9,  1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR}, // Initial Samples
			{10, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR}, // Temporal
			{11, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR},  // Spatial
			// G-Buffer
			{12, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR}, // pos gbuffer
			{13, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR}, // nrm gbuffer
			{14, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR}, // mat and uv gbuffer
			{15, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR},  // motion gbuffer
			{16, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR},  // pos prev gbuffer
			{17, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR},  // nrm prev gbuffer
			{18, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR}, // Initial old Samples
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
			offsetsBufferInfo.range = VK_WHOLE_SIZE;

			std::vector<VkDescriptorImageInfo> imageInfos(scene.GetTextureSamplers().size());

			for (uint32_t t = 0; t < imageInfos.size(); t++)
			{
				imageInfos[t].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfos[t].imageView = scene.GetTextureImageViews()[t];
				imageInfos[t].sampler = scene.GetTextureSamplers()[t];
			}

			//restir gi
			//initial sample buffer
			VkDescriptorBufferInfo initialSampleBufferInfo = {};
			initialSampleBufferInfo.buffer = initialSampleBuffer->Handle();
			initialSampleBufferInfo.range = VK_WHOLE_SIZE;

			VkDescriptorBufferInfo initialoldSampleBufferInfo = {};
			initialSampleBufferInfo.buffer = initialOldSampleBuffer->Handle();
			initialSampleBufferInfo.range = VK_WHOLE_SIZE;

			//temporal buffer
			VkDescriptorBufferInfo temporalBufferInfo = {};
			temporalBufferInfo.buffer = temporalResevoirBuffer->Handle();
			temporalBufferInfo.range = VK_WHOLE_SIZE;
			//spatial buffer
			VkDescriptorBufferInfo spatialBufferInfo = {};
			spatialBufferInfo.buffer = spatialReservoirBuffer->Handle();
			spatialBufferInfo.range = VK_WHOLE_SIZE;

			//gbuffers
			VkDescriptorImageInfo posGBufferInfo = {};
			posGBufferInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			posGBufferInfo.imageView = posGBuffer.Handle();

			VkDescriptorImageInfo nrmGBufferInfo = {};
			nrmGBufferInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			nrmGBufferInfo.imageView = nrmGBuffer.Handle();

			VkDescriptorImageInfo matUVGBufferInfo = {};
			matUVGBufferInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			matUVGBufferInfo.imageView = matUVGBuffer.Handle();

			VkDescriptorImageInfo motionGBufferInfo = {};
			motionGBufferInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			motionGBufferInfo.imageView = motionGBuffer.Handle();

			VkDescriptorImageInfo posGBufferPrevInfo = {};
			posGBufferInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			posGBufferInfo.imageView = posGBufferPrev.Handle();

			VkDescriptorImageInfo nrmGBufferPrevInfo = {};
			nrmGBufferInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			nrmGBufferInfo.imageView = nrmGBufferPrev.Handle();

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
				descriptorSets.Bind(i, 8, *imageInfos.data(), imageInfos.size()),
				descriptorSets.Bind(i, 9, initialSampleBufferInfo),
				descriptorSets.Bind(i, 10, temporalBufferInfo),
				descriptorSets.Bind(i, 11, spatialBufferInfo),
				// Bind G-Buffers
				descriptorSets.Bind(i, 12, posGBufferInfo),
				descriptorSets.Bind(i, 13, nrmGBufferInfo),
				descriptorSets.Bind(i, 14, matUVGBufferInfo),
				descriptorSets.Bind(i, 15, motionGBufferInfo),
				descriptorSets.Bind(i, 16, posGBufferInfo),
				descriptorSets.Bind(i, 17, nrmGBufferInfo),
				descriptorSets.Bind(i, 18, initialoldSampleBufferInfo)
			};

			descriptorSets.UpdateDescriptors(descriptorWrites);
		}//end for

		xvk_pipelineLayout.reset(new XVKPipelineLayout(device, xvk_descriptorSetManager->GetDescriptorSetLayout()));

		//create shader modules
		const XVKShaderModule rayMissShader(device, SHADER_DIR"HLSL/ReSTIRGI/rmiss.spv");
		const XVKShaderModule rayClosetHitShader(device, SHADER_DIR"HLSL/ReSTIRGI/rchit.spv");
		const XVKShaderModule rayAnyHitShader(device, SHADER_DIR"HLSL/ReSTIRGI/rahit.spv");
		const XVKShaderModule rayShadowMissShader(device, SHADER_DIR"HLSL/ReSTIRGI/rmiss_shadow.spv");

		const XVKShaderModule restirGIInitialSampleShader(device, SHADER_DIR"HLSL/ReSTIRGI/rgen_initialSample.spv");
		const XVKShaderModule restirGITemporalReuseShader(device, SHADER_DIR"HLSL/ReSTIRGI/rgen_temporalReuse.spv");
		const XVKShaderModule restirGISpatialReuseShader(device, SHADER_DIR"HLSL/ReSTIRGI/rgen_spatialReuse.spv");
		//const XVKShaderModule restirGIOutputShader(device, SHADER_DIR"HLSL/ReSTIRGI/restirGIOutput.spv");
	
		//define all shader stages (¸üÐÂºó)
		std::vector<VkPipelineShaderStageCreateInfo> initialSampleStages = {
			// RayGen Shader + Ray Miss Shader/Ray Shadow Miss Shader + Ray Closet Hit Shader/Ray Any Hit Shader
			restirGIInitialSampleShader.CreateShaderStage(VK_SHADER_STAGE_RAYGEN_BIT_KHR),
			rayMissShader.CreateShaderStage(VK_SHADER_STAGE_MISS_BIT_KHR),       
			rayShadowMissShader.CreateShaderStage(VK_SHADER_STAGE_MISS_BIT_KHR),  
			rayClosetHitShader.CreateShaderStage(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR),    
			rayAnyHitShader.CreateShaderStage(VK_SHADER_STAGE_ANY_HIT_BIT_KHR)     
		};

		std::vector<VkPipelineShaderStageCreateInfo> temporalReuseStages = {
			// RayGen Shader + Ray Miss Shader/Ray Shadow Miss Shader + Ray Closet Hit Shader/Ray Any Hit Shader
			restirGITemporalReuseShader.CreateShaderStage(VK_SHADER_STAGE_RAYGEN_BIT_KHR),
			rayMissShader.CreateShaderStage(VK_SHADER_STAGE_MISS_BIT_KHR),       
			rayShadowMissShader.CreateShaderStage(VK_SHADER_STAGE_MISS_BIT_KHR),  
			rayClosetHitShader.CreateShaderStage(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR),    
			rayAnyHitShader.CreateShaderStage(VK_SHADER_STAGE_ANY_HIT_BIT_KHR)     
		};

		std::vector<VkPipelineShaderStageCreateInfo> spatialReuseStages = {
			// RayGen Shader + Ray Miss Shader/Ray Shadow Miss Shader + Ray Closet Hit Shader/Ray Any Hit Shader
			restirGISpatialReuseShader.CreateShaderStage(VK_SHADER_STAGE_RAYGEN_BIT_KHR),
			rayMissShader.CreateShaderStage(VK_SHADER_STAGE_MISS_BIT_KHR),       
			rayShadowMissShader.CreateShaderStage(VK_SHADER_STAGE_MISS_BIT_KHR),  
			rayClosetHitShader.CreateShaderStage(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR),    
			rayAnyHitShader.CreateShaderStage(VK_SHADER_STAGE_ANY_HIT_BIT_KHR)     
		};

		//define shader groups 
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> initialGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> temporalGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> spatialGroups;

		//intial sample shader group
		VkRayTracingShaderGroupCreateInfoKHR initialSampleGroup = {};
		initialSampleGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		initialSampleGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		initialSampleGroup.pNext = nullptr;
		initialSampleGroup.generalShader = 0;
		initialSampleGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		initialSampleGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		initialSampleGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		initialGroups.push_back(initialSampleGroup);
		restirgiInitialSampleIndex = 0;

		//temporal reuse shader group
		VkRayTracingShaderGroupCreateInfoKHR temporalReuseGroup = {};
		temporalReuseGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		temporalReuseGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		temporalReuseGroup.pNext = nullptr;
		temporalReuseGroup.generalShader = 0;
		temporalReuseGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		temporalReuseGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		temporalReuseGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		temporalGroups.push_back(temporalReuseGroup);
		restirgiTemporalReuseIndex = 0;

		//spatial reuse shader group
		VkRayTracingShaderGroupCreateInfoKHR spatialReuseGroup = {};
		spatialReuseGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		spatialReuseGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		spatialReuseGroup.pNext = nullptr;
		spatialReuseGroup.generalShader = 0;
		spatialReuseGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		spatialReuseGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		spatialReuseGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		spatialGroups.push_back(spatialReuseGroup);
		restirgiSpatialReuseIndex = 0;

		//miss/hit shader group
		VkRayTracingShaderGroupCreateInfoKHR missGroupInfo = {};
		missGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		missGroupInfo.pNext = nullptr;
		missGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		missGroupInfo.generalShader = 1;
		missGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		missGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		missGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		initialGroups.push_back(missGroupInfo);
		temporalGroups.push_back(missGroupInfo);
		spatialGroups.push_back(missGroupInfo);
		rayMissIndex = 1;

		VkRayTracingShaderGroupCreateInfoKHR shadowMissGroup = {};
		shadowMissGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shadowMissGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shadowMissGroup.generalShader = 2;
		shadowMissGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		shadowMissGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shadowMissGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		initialGroups.push_back(shadowMissGroup);
		temporalGroups.push_back(shadowMissGroup);
		spatialGroups.push_back(shadowMissGroup);
		rayShadowMissIndex = 2;

		VkRayTracingShaderGroupCreateInfoKHR triangleHitGroupInfo = {};
		triangleHitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		triangleHitGroupInfo.pNext = nullptr;
		triangleHitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		triangleHitGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
		triangleHitGroupInfo.closestHitShader = 3;
		triangleHitGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		triangleHitGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		initialGroups.push_back(triangleHitGroupInfo);
		temporalGroups.push_back(triangleHitGroupInfo);
		spatialGroups.push_back(triangleHitGroupInfo);
		rayTriangleHitGroupIndex = 3;

		VkRayTracingShaderGroupCreateInfoKHR anyHitGroup = {};
		anyHitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		anyHitGroup.pNext = nullptr;
		anyHitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		anyHitGroup.generalShader = VK_SHADER_UNUSED_KHR;
		anyHitGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		anyHitGroup.anyHitShader = 4;
		anyHitGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		initialGroups.push_back(anyHitGroup);
		temporalGroups.push_back(anyHitGroup);
		spatialGroups.push_back(anyHitGroup);
		rayAnyHitIndex = 4;

		//create pipeline
		//initial sample pipeline
		VkRayTracingPipelineCreateInfoKHR initialSamplepipelineInfo = {};
		initialSamplepipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		initialSamplepipelineInfo.pNext = nullptr;
		initialSamplepipelineInfo.flags = 0;
		initialSamplepipelineInfo.stageCount = static_cast<uint32_t>(initialSampleStages.size());
		initialSamplepipelineInfo.pStages = initialSampleStages.data();
		initialSamplepipelineInfo.groupCount = static_cast<uint32_t>(initialGroups.size());
		initialSamplepipelineInfo.pGroups = initialGroups.data();
		initialSamplepipelineInfo.maxPipelineRayRecursionDepth = 2;
		initialSamplepipelineInfo.layout = xvk_pipelineLayout->Handle();
		initialSamplepipelineInfo.basePipelineHandle = nullptr;
		initialSamplepipelineInfo.basePipelineIndex = 0;
		VULKAN_RUNTIME_CHECK(deviceFunc.vkCreateRayTracingPipelinesKHR(device.Handle(), nullptr,
			nullptr, 1, &initialSamplepipelineInfo, nullptr, &vk_restirGI_initialSamplePipeline), "create initial sample pipeline");
		//temporal reuse pipeline
		VkRayTracingPipelineCreateInfoKHR temporalReusePipelineInfo = {};
		temporalReusePipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		temporalReusePipelineInfo.pNext = nullptr;
		temporalReusePipelineInfo.flags = 0;
		temporalReusePipelineInfo.stageCount = static_cast<uint32_t>(temporalReuseStages.size());
		temporalReusePipelineInfo.pStages = temporalReuseStages.data();
		temporalReusePipelineInfo.groupCount = static_cast<uint32_t>(temporalGroups.size());
		temporalReusePipelineInfo.pGroups = temporalGroups.data();
		temporalReusePipelineInfo.maxPipelineRayRecursionDepth = 1;
		temporalReusePipelineInfo.layout = xvk_pipelineLayout->Handle();
		temporalReusePipelineInfo.basePipelineHandle = nullptr;
		temporalReusePipelineInfo.basePipelineIndex = 0;
		VULKAN_RUNTIME_CHECK(deviceFunc.vkCreateRayTracingPipelinesKHR(device.Handle(), nullptr,
			nullptr, 1, &temporalReusePipelineInfo, nullptr, &vk_restirGI_temporalReusePipeline), "create temporal reuse pipeline");
		//spatial reuse pipeline
		VkRayTracingPipelineCreateInfoKHR spatialReusePipelineInfo = {};
		spatialReusePipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		spatialReusePipelineInfo.pNext = nullptr;
		spatialReusePipelineInfo.flags = 0;
		spatialReusePipelineInfo.stageCount = static_cast<uint32_t>(spatialReuseStages.size());
		spatialReusePipelineInfo.pStages = spatialReuseStages.data();
		spatialReusePipelineInfo.groupCount = static_cast<uint32_t>(spatialGroups.size());
		spatialReusePipelineInfo.pGroups = spatialGroups.data();
		spatialReusePipelineInfo.maxPipelineRayRecursionDepth = 1;
		spatialReusePipelineInfo.layout = xvk_pipelineLayout->Handle();
		spatialReusePipelineInfo.basePipelineHandle = nullptr;
		spatialReusePipelineInfo.basePipelineIndex = 0;
		VULKAN_RUNTIME_CHECK(deviceFunc.vkCreateRayTracingPipelinesKHR(device.Handle(), nullptr,
			nullptr, 1, &spatialReusePipelineInfo, nullptr, &vk_restirGI_spatialReusePipeline), "create spatial reuse pipeline");
	
		//output pipeline
		/*VkPipelineShaderStageCreateInfo ouptputStage = restirGIOutputShader.CreateShaderStage(VK_SHADER_STAGE_COMPUTE_BIT);
		VkComputePipelineCreateInfo outputPipelineInfo = {};
		outputPipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		outputPipelineInfo.stage = ouptputStage;
		outputPipelineInfo.layout = xvk_pipelineLayout->Handle(); 
		outputPipelineInfo.flags = 0;
		outputPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		outputPipelineInfo.basePipelineIndex = -1;
		VULKAN_RUNTIME_CHECK(vkCreateComputePipelines(device.Handle(), VK_NULL_HANDLE, 1,
			&outputPipelineInfo, nullptr, &vk_restirGI_outputPipeline), "create output compute pipeline");*/
	}

	XVKReSTIRGIPipeline::~XVKReSTIRGIPipeline()
	{
		if (vk_restirGI_initialSamplePipeline != nullptr)
		{
			vkDestroyPipeline(xvk_swapChain.GetDevice().Handle(), vk_restirGI_initialSamplePipeline, nullptr);
			vk_restirGI_initialSamplePipeline = nullptr;
		}
		if (vk_restirGI_temporalReusePipeline != nullptr)
		{
			vkDestroyPipeline(xvk_swapChain.GetDevice().Handle(), vk_restirGI_temporalReusePipeline, nullptr);
			vk_restirGI_temporalReusePipeline = nullptr;
		}
		if (vk_restirGI_spatialReusePipeline != nullptr)
		{
			vkDestroyPipeline(xvk_swapChain.GetDevice().Handle(), vk_restirGI_spatialReusePipeline, nullptr);
			vk_restirGI_spatialReusePipeline = nullptr;
		}
		
		xvk_descriptorSetManager.reset();
		xvk_pipelineLayout.reset();
	}

	VkDescriptorSet XVKReSTIRGIPipeline::GetDescriptorSet(uint32_t index) const
	{
		return xvk_descriptorSetManager->GetDescriptorSets().Handle(index);
	}
}