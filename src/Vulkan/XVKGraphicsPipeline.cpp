#include "XVKGraphicsPipeline.h"
#include "XVKDevice.h"
#include "XVKSwapChain.h"
#include "XVKRenderPass.h"
#include "XVKPipelineLayout.h"
#include "XVKDescriptorSetManager.h"
#include "XVKDescriptorSetLayout.h"
#include "XVKDescriptorSet.h"
#include "XVKPipelineLayout.h"
#include "XVKBuffer.h"
#include "XVKShaderModule.h"
#include "Asset/Scene.h"
#include "Asset/UniformBuffer.h"
#include "Asset/Vertex.h"

namespace xvk
{
	XVKGraphicsPipeline::XVKGraphicsPipeline(const XVKDevice& device,
		const XVKSwapChain& swapChain,
		const XVKDepthBuffer& depthBuffer,
		const std::vector<vkAsset::AUniformBuffer>& uniformBuffers,
		const vkAsset::AScene& scene)
		:xvk_swapChain(swapChain)
	{
		const auto& xvk_device = swapChain.GetDevice();
		const auto bindingDescription = vkAsset::AVertex::getBindingDescription();
		const auto attributeDescription = vkAsset::AVertex::getAttributeDescriptions();
		
		//shader
		XVKShaderModule vertShaderMoudle(device, SHADER_DIR"/HLSL/simpleRZ.vert.spv");
		XVKShaderModule fragShaderModule(device, SHADER_DIR"/HLSL/simpleRZ.frag.spv");
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderMoudle.CreateShaderStage(VK_SHADER_STAGE_VERTEX_BIT),
			fragShaderModule.CreateShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT)};

		//vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = attributeDescription.size();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		//viewport
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain.GetExtent().width);
		viewport.height = static_cast<float>(swapChain.GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = swapChain.GetExtent();

		VkPipelineViewportStateCreateInfo viewportInfo = {};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = &viewport;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = &scissor;

		//rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		//multi sample
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		//depth stencil
		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		//color
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		std::vector<DescriptorBinding> descriptorBindings;
		descriptorBindings.emplace_back( 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		descriptorBindings.emplace_back( 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT );
		descriptorBindings.emplace_back( 2, static_cast<uint32_t>(scene.GetTextureSamplers().size()), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT );
		
		//uniformBuffer.size() equals the number of swap chain images
		xvk_descriptorSetManager.reset(new XVKDescriptorSetManager(device, descriptorBindings, uniformBuffers.size()));
	
		XVKDescriptorSets& descriptorSets = xvk_descriptorSetManager->GetDescriptorSets();
		//bind descriptor set
		for (uint32_t i = 0; i < swapChain.GetImages().size(); i++)
		{
			//uniform buffer
			VkDescriptorBufferInfo uniformBufferInfo = {};
			uniformBufferInfo.buffer = uniformBuffers[i].GetBuffer().Handle();
			uniformBufferInfo.range = VK_WHOLE_SIZE;//?

			//materail buffer
			VkDescriptorBufferInfo materialBufferInfo = {};
			materialBufferInfo.buffer = scene.GetMaterialBuffer().Handle();
			materialBufferInfo.range = VK_WHOLE_SIZE;

			//image
			std::vector<VkDescriptorImageInfo> imageInfos(scene.GetTextureSamplers().size());
			for (uint32_t t = 0; t < imageInfos.size(); t++)
			{
				imageInfos[t].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfos[t].imageView = scene.GetTextureImageViews()[t];
				imageInfos[t].sampler = scene.GetTextureSamplers()[t];
			}

			std::vector<VkWriteDescriptorSet> descriptorSetWrites;
			//each descriptorset contains multiple descriptors, each descriptor has its own binding index
			descriptorSetWrites.emplace_back(descriptorSets.Bind(i, 0, uniformBufferInfo));
			descriptorSetWrites.emplace_back(descriptorSets.Bind(i, 1, materialBufferInfo));
			descriptorSetWrites.emplace_back(descriptorSets.Bind(i, 2, *imageInfos.data(), static_cast<uint32_t>(imageInfos.size())));
		
			descriptorSets.UpdateDescriptors(descriptorSetWrites);
		}

		xvk_pipelineLayout.reset(new XVKPipelineLayout(device, xvk_descriptorSetManager->GetDescriptorSetLayout()));
		xvk_renderPass.reset(new XVKRenderPass(device, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR,
			depthBuffer, swapChain));
		//finally create graphics pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.basePipelineHandle = nullptr; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
		pipelineInfo.layout = xvk_pipelineLayout->Handle();
		pipelineInfo.renderPass = xvk_renderPass->Handle();
		pipelineInfo.subpass = 0;

		VULKAN_RUNTIME_CHECK(vkCreateGraphicsPipelines(device.Handle(), nullptr, 1, &pipelineInfo,
			nullptr, &vk_graphicsPipeline), "create graphics pipeline");
	}

	XVKGraphicsPipeline::~XVKGraphicsPipeline()
	{
		if (vk_graphicsPipeline != nullptr)
		{
			vkDestroyPipeline(xvk_swapChain.GetDevice().Handle(), vk_graphicsPipeline, nullptr);
			vk_graphicsPipeline = nullptr;
		}

		xvk_renderPass.reset();
		xvk_pipelineLayout.reset();
		xvk_descriptorSetManager.reset();
	}

	VkDescriptorSet XVKGraphicsPipeline::GetDescriptorSet(size_t index) const 
	{ return xvk_descriptorSetManager->GetDescriptorSets().Handle(index); }
}