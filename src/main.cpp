#include <Vulkan/XVKBuffer.h>
#include "Vulkan/XVKInstance.h"
#include "Vulkan/XVKWindow.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKSwapChain.h"
#include "Vulkan/XVKRenderPass.h"
#include "Vulkan/XVKDescriptorSetLayout.h"
#include "Vulkan/XVKCommandPool.h"
#include "Vulkan/XVKCommandBuffers.h"
#include "Vulkan/XVKDepthBuffer.h"
#include "Vulkan/XVKDescriptorSetManager.h"
#include "Vulkan/XVKDescriptorSet.h"
#include "Vulkan/XVKShaderModule.h"
#include "Vulkan/XVKPipelineLayout.h"
#include "Vulkan/XVKFrameBuffer.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <cstdlib>
#include "Vulkan/XVKSemaphore.h"
#include "Vulkan/XVKFence.h"
#include "Vulkan/XVKImage.h"
#include "Vulkan/XVKImageView.h"
#include "Asset/TextureImage.h"
#include "Asset/Texture.h"
#include "Asset/Scene.h"
#include "Asset/Model.h"
#include "Vulkan/XVKDeviceMemory.h"
#include "Utility/BufferUtil.h"
#include "pathTracer.h"
#include "Editor/UserSettings.h"
#include "Editor/Camera.h"
struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

const std::vector<Vertex> vertices = {
	{{0.5f, -0.5f, 0.0f}, {1.0f, 0.3f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

	{{ 0.5f, -0.5f, -0.5f }, {1.0f, 0.3f, 0.0f}, {1.0f, 0.0f} },
	{{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};

const int MAX_FRAMES_IN_FLIGHT = 2;

int main()
{
	xvk::XVKWindow window({ "test", 1920, 1080, true, false });
	try
	{
#pragma region api_test
		//std::vector<const char*> testlayer{ "VK_LAYER_KHRONOS_validation" };
		//xvk::XVKInstance instance(window, testlayer);
		//std::vector<const char*> testextension{ VK_KHR_SWAPCHAIN_EXTENSION_NAME,VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		//VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		//VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME };
		//xvk::XVKDevice device(instance, testextension);
		//xvk::XVKSwapChain swapChain(device, VK_PRESENT_MODE_MAILBOX_KHR);
		//xvk::XVKCommandPool commandPool(device, device.GraphicsQueueIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		//xvk::XVKCommandBuffers commandBuffers(commandPool, swapChain.GetMinImageCount());
		//xvk::XVKDepthBuffer depthBuffer(commandPool, swapChain.GetExtent());
		//xvk::XVKRenderPass renderPass(device, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR,
		//	depthBuffer, swapChain);
		//xvk::DescriptorBinding binding{ 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT };
		//xvk::DescriptorBinding binding1{ 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT };

		//xvk::XVKDescriptorSetManager descriptorManager(device, { binding, binding1 }, swapChain.GetImages().size());
		//VkDescriptorSetLayout descriptorSetLayout = descriptorManager.GetDescriptorSetLayout().Handle();

		//std::vector<xvk::XVKFrameBuffer> frameBuffers;
		//frameBuffers.reserve(swapChain.GetMinImageCount());
		//for (int i = 0; i < swapChain.GetMinImageCount(); i++)
		//{
		//	frameBuffers.emplace_back(*swapChain.GetImageViews()[i], renderPass);
		//}
		//VkPipeline graphicsPipeline;

		//xvk::XVKShaderModule vertShaderModule(device, SHADER_DIR"vert.spv");
		//xvk::XVKShaderModule fragShaderModule(device, SHADER_DIR"frag.spv");
		//VkPipelineShaderStageCreateInfo vertStageInfo = vertShaderModule.CreateShaderStage(VK_SHADER_STAGE_VERTEX_BIT);
		//VkPipelineShaderStageCreateInfo fragStageInfo = fragShaderModule.CreateShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT);
		//VkPipelineShaderStageCreateInfo shaderStages[] = { vertStageInfo, fragStageInfo };

		//VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		//vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		//auto bindingDescription = Vertex::getBindingDescription();
		//auto attributeDescriptions = Vertex::getAttributeDescriptions();

		//vertexInputInfo.vertexBindingDescriptionCount = 1;
		//vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
		//vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		//vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		//VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		//inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		//inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		//inputAssembly.primitiveRestartEnable = VK_FALSE;

		//VkViewport viewport = {};
		//viewport.x = 0.0f;
		//viewport.y = 0.0f;
		//viewport.width = (float)swapChain.GetExtent().width;
		//viewport.height = (float)swapChain.GetExtent().height;
		//viewport.minDepth = 0.0f;
		//viewport.maxDepth = 1.0f;
		//VkRect2D scissor = {};
		//scissor.offset = { 0,0 };
		//scissor.extent = swapChain.GetExtent();
		//VkPipelineViewportStateCreateInfo viewportState = {};
		//viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		//viewportState.viewportCount = 1;
		//viewportState.pViewports = &viewport;
		//viewportState.scissorCount = 1;
		//viewportState.pScissors = &scissor;

		//VkPipelineRasterizationStateCreateInfo rasterizer = {};
		//rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		//rasterizer.depthClampEnable = VK_FALSE;
		//rasterizer.rasterizerDiscardEnable = VK_FALSE;
		//rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		//rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		//rasterizer.lineWidth = 1.0f;
		//rasterizer.depthBiasEnable = VK_FALSE;
		//rasterizer.depthBiasConstantFactor = 0.0f;
		//rasterizer.depthBiasClamp = 0.0f;
		//rasterizer.depthBiasSlopeFactor = 0.0f;

		//VkPipelineMultisampleStateCreateInfo multisampling = {};
		//multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		//multisampling.sampleShadingEnable = VK_FALSE;
		//multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		//multisampling.minSampleShading = 1.0f;
		//multisampling.pSampleMask = nullptr;
		//multisampling.alphaToCoverageEnable = VK_FALSE;
		//multisampling.alphaToOneEnable = VK_FALSE;

		//VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		//colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
		//	| VK_COLOR_COMPONENT_A_BIT;
		//colorBlendAttachment.blendEnable = VK_FALSE;
		//colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		//colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		//colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		//colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		//colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		//colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		//VkPipelineColorBlendStateCreateInfo colorBlending = {};
		//colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		//colorBlending.logicOpEnable = VK_FALSE;
		//colorBlending.logicOp = VK_LOGIC_OP_COPY;
		//colorBlending.attachmentCount = 1;
		//colorBlending.pAttachments = &colorBlendAttachment;
		//colorBlending.blendConstants[0] = 0.0f;
		//colorBlending.blendConstants[1] = 0.0f;
		//colorBlending.blendConstants[2] = 0.0f;
		//colorBlending.blendConstants[3] = 0.0f;

		//std::vector<VkDynamicState> dynamicStates = {
		//	  VK_DYNAMIC_STATE_VIEWPORT,
		//	  VK_DYNAMIC_STATE_SCISSOR
		//};
		//VkPipelineDynamicStateCreateInfo dynamicState = {};
		//dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		//dynamicState.dynamicStateCount = dynamicStates.size();
		//dynamicState.pDynamicStates = dynamicStates.data();

		//xvk::XVKPipelineLayout pipelineLayout(device, descriptorManager.GetDescriptorSetLayout());

		//VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		//depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		//depthStencil.depthTestEnable = VK_TRUE;
		//depthStencil.depthWriteEnable = VK_TRUE;
		//depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		//depthStencil.depthBoundsTestEnable = VK_FALSE;
		//depthStencil.minDepthBounds = 0.0f;
		//depthStencil.maxDepthBounds = 1.0f;
		//depthStencil.stencilTestEnable = VK_FALSE;
		//depthStencil.front = {};
		//depthStencil.back = {};

		//VkGraphicsPipelineCreateInfo pipelineInfo = {};
		//pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		//pipelineInfo.stageCount = 2;
		//pipelineInfo.pStages = shaderStages;
		//pipelineInfo.pVertexInputState = &vertexInputInfo;
		//pipelineInfo.pInputAssemblyState = &inputAssembly;
		//pipelineInfo.pViewportState = &viewportState;
		//pipelineInfo.pRasterizationState = &rasterizer;
		//pipelineInfo.pMultisampleState = &multisampling;
		//pipelineInfo.pDepthStencilState = nullptr;
		//pipelineInfo.pColorBlendState = &colorBlending;
		//pipelineInfo.pDynamicState = &dynamicState;
		//pipelineInfo.layout = pipelineLayout.Handle();
		//pipelineInfo.renderPass = renderPass.Handle();
		//pipelineInfo.subpass = 0;
		//pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		//pipelineInfo.basePipelineIndex = -1;
		//pipelineInfo.pDepthStencilState = &depthStencil;

		//VULKAN_RUNTIME_CHECK(vkCreateGraphicsPipelines(device.Handle(), VK_NULL_HANDLE,
		//	1, &pipelineInfo, nullptr, &graphicsPipeline));

		////prepare buffer

		////--vertex buffer--
		//VkDeviceSize vertBufferSize = sizeof(vertices[0]) * vertices.size();
		//xvk::XVKBuffer stagingBuffer(device, vertBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		//xvk::XVKDeviceMemory stagingBufferMemory = stagingBuffer.AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		//	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//void* data = stagingBufferMemory.MapMemory(0, vertBufferSize);
		//memcpy(data, vertices.data(), vertBufferSize);
		//stagingBufferMemory.UnmapMemory();

		//xvk::XVKBuffer vertexBuffer(device, vertBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		//xvk::XVKDeviceMemory vertexBufferMemory = vertexBuffer.AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		//vertexBuffer.CopyFromBuffer(commandPool, stagingBuffer, vertBufferSize);
		////--vertex end--

		////--index buffer--
		////VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
		////xvk::XVKBuffer indexBuffer(device, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		////xvk::XVKDeviceMemory indexBufferMemory = indexBuffer.AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		////utility::BufferUtility::CopyFromBuffer(commandPool, indexBuffer, indices);

		//std::unique_ptr<xvk::XVKBuffer> indexBuffer;
		//std::unique_ptr<xvk::XVKDeviceMemory> indexBufferMemory;
		//utility::BufferUtility::CreateBuffer(commandPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices, indexBuffer, indexBufferMemory);

		////--index end--

		////--uniform buffer--
		//VkDeviceSize uniformBufferSize = sizeof(UniformBufferObject);
		//std::vector<xvk::XVKBuffer> uniformBuffers;
		//std::vector<xvk::XVKDeviceMemory> uniformMemories;
		//std::vector<void*> uniformMapped;

		//uniformBuffers.reserve(swapChain.GetImages().size());
		//uniformMemories.reserve(swapChain.GetImages().size());
		//uniformMapped.reserve(swapChain.GetImages().size());

		//UniformBufferObject ubo{};
		//ubo.model = glm::rotate(glm::mat4(1.0f), 1 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.proj = glm::perspective(glm::radians(45.0f), (float)swapChain.GetExtent().width / (float)swapChain.GetExtent().height, 0.1f, 10.0f);
		//ubo.proj[1][1] *= -1;

		////提前reserve分配空间，不初始化。否则一个个添加元素会引发容器扩容，导致自动析构原来的元素，这样会导致buffer被反复destroy
		//for (int i = 0; i < swapChain.GetImages().size(); i++)
		//{
		//	uniformBuffers.emplace_back(device, uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		//	auto memory = uniformBuffers[i].AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		//		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//	uniformMemories.push_back(std::move(memory));

		//	uniformMapped.push_back(uniformMemories[i].MapMemory(0, uniformBufferSize));
		//	memcpy(uniformMapped[i], &ubo, uniformBufferSize);
		//	uniformMemories[i].UnmapMemory();
		//}
		////--

		////--image&sampler--
		//xvk::SamplerConfig config = {};
		//config.MagFilter = VK_FILTER_LINEAR;
		//config.MinFilter = VK_FILTER_LINEAR;
		//config.AddressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//config.AddressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//config.AddressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//config.AnisotropyEnable = VK_TRUE;
		//config.MaxAnisotropy = 16;
		//config.BorderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		//config.UnnormalizedCoordinates = VK_FALSE;
		//config.CompareEnable = VK_FALSE;
		//config.CompareOp = VK_COMPARE_OP_ALWAYS;
		//config.MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		//config.MipLodBias = 0.0f;
		//config.MinLod = 0.0f;
		//config.MaxLod = 0.0f;

		//vkAsset::AVulkanTexture testTexture(ASSET_DIR"texture/TestTexture0.png", config);
		//vkAsset::AVulkanTextureImage textureImage(commandPool, testTexture);
		////--

		////--descriptor set--
		//for (int i = 0; i < swapChain.GetImages().size(); i++)
		//{
		//	VkDescriptorBufferInfo bufferInfo = {};
		//	bufferInfo.buffer = uniformBuffers[i].Handle();
		//	bufferInfo.offset = 0;
		//	bufferInfo.range = sizeof(UniformBufferObject);

		//	VkDescriptorImageInfo imageInfo = {};
		//	imageInfo.imageLayout = textureImage.GetTextureImage().GetImageLayout();
		//	imageInfo.imageView = textureImage.GetTextureImageView().Handle();
		//	imageInfo.sampler = textureImage.GetTextureImageSampler().Handle();

		//	auto write = descriptorManager.GetDescriptorSets().Bind(i, 0, bufferInfo, 1);
		//	auto write1 = descriptorManager.GetDescriptorSets().Bind(i, 1, imageInfo, 1);

		//	std::vector<VkWriteDescriptorSet> writes{ write, write1 };
		//	descriptorManager.GetDescriptorSets().UpdateDescriptors(writes);
		//}
		////--

		////synchronization
		//std::vector<xvk::XVKSemaphore> imageAvailableSemaphores;
		//std::vector<xvk::XVKSemaphore> renderFinishedSemaphores;
		//std::vector<xvk::XVKFence> inFlightFences;

		//imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
		//renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
		//inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

		//for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		//{
		//	imageAvailableSemaphores.emplace_back(device);
		//	renderFinishedSemaphores.emplace_back(device);
		//	inFlightFences.emplace_back(device, true);
		//}

		//glfwSetTime(0.0);

		//int currentFrame = 0;
		//auto DrawFrame = [&]() {

		//	vkWaitForFences(device.Handle(), 1, &inFlightFences[currentFrame].Handle(), VK_TRUE,
		//		std::numeric_limits<uint64_t>::max());

		//	uint32_t imageIndex;
		//	VkResult result = vkAcquireNextImageKHR(device.Handle(), swapChain.Handle(), UINT64_MAX,
		//		imageAvailableSemaphores[currentFrame].Handle(), VK_NULL_HANDLE, &imageIndex);

		//	vkResetFences(device.Handle(), 1, &inFlightFences[currentFrame].Handle());

		//	//--record command buffer--
		//	vkResetCommandBuffer(commandBuffers[imageIndex], 0);
		//	auto commandBuffer = commandBuffers.Begin(imageIndex);

		//	std::array<VkClearValue, 2> clearValues = {};
		//	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		//	clearValues[1].depthStencil = { 1.0f, 0 };

		//	VkRenderPassBeginInfo renderPassInfo = {};
		//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//	renderPassInfo.renderPass = renderPass.Handle();
		//	renderPassInfo.framebuffer = frameBuffers[imageIndex].Handle();
		//	renderPassInfo.renderArea.offset = { 0, 0 };
		//	renderPassInfo.renderArea.extent = swapChain.GetExtent();
		//	renderPassInfo.clearValueCount = clearValues.size();
		//	renderPassInfo.pClearValues = clearValues.data();

		//	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		//	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		//	VkBuffer vertexBuffers[] = { vertexBuffer.Handle() };
		//	VkDeviceSize offsets[] = { 0 };
		//	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		//	vkCmdBindIndexBuffer(commandBuffer, indexBuffer->Handle(), 0, VK_INDEX_TYPE_UINT16);

		//	VkViewport viewport{};
		//	viewport.x = 0.0f;
		//	viewport.y = 0.0f;
		//	viewport.width = (float)swapChain.GetExtent().width;
		//	viewport.height = (float)swapChain.GetExtent().height;
		//	viewport.minDepth = 0.0f;
		//	viewport.maxDepth = 1.0f;
		//	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		//	VkRect2D scissor{};
		//	scissor.offset = { 0, 0 };
		//	scissor.extent = swapChain.GetExtent();
		//	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		//	VkDescriptorSet descriptors[] = { descriptorManager.GetDescriptorSets().Handle(imageIndex) };
		//	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.Handle(), 0, 1,
		//		descriptors,
		//		0, nullptr);

		//	vkCmdDrawIndexed(commandBuffer, indices.size(), 1, 0, 0, 0);

		//	vkCmdEndRenderPass(commandBuffer);

		//	commandBuffers.End(imageIndex);

		//	VkSubmitInfo submitInfo = {};
		//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame].Handle() };
		//	VkPipelineStageFlags waitStages[] = {
		//		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		//	};
		//	submitInfo.waitSemaphoreCount = 1;
		//	submitInfo.pWaitSemaphores = waitSemaphores;
		//	submitInfo.pWaitDstStageMask = waitStages;

		//	submitInfo.commandBufferCount = 1;
		//	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		//	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame].Handle() };
		//	submitInfo.signalSemaphoreCount = 1;
		//	submitInfo.pSignalSemaphores = signalSemaphores;

		//	if (vkQueueSubmit(device.GraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame].Handle()) != VK_SUCCESS)
		//	{
		//		throw std::runtime_error("fail to submit draw command buffer!");
		//	}

		//	VkPresentInfoKHR presentInfo = {};
		//	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		//	presentInfo.waitSemaphoreCount = 1;
		//	presentInfo.pWaitSemaphores = signalSemaphores;

		//	VkSwapchainKHR swapChains[] = { swapChain.Handle() };
		//	presentInfo.pSwapchains = swapChains;
		//	presentInfo.pImageIndices = &imageIndex;
		//	presentInfo.swapchainCount = 1;

		//	presentInfo.pResults = nullptr;

		//	result = vkQueuePresentKHR(device.PresentQueue(), &presentInfo);

		//	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		//	};

		//while (!glfwWindowShouldClose(window.Handle()))
		//{
		//	glfwPollEvents();
		//	DrawFrame();
		//}
		//vkDeviceWaitIdle(device.Handle());
		////window.Run();

		//vkDestroyPipeline(device.Handle(), graphicsPipeline, nullptr);
#pragma endregion
		editor::UserSettings userSettings{};
		userSettings.sceneIndex = 0;
		userSettings.enableRayAccumulation = false;
		userSettings.enableRayTracing = false;
		userSettings.fov = 45.0f;
		userSettings.spp = 8;
		userSettings.numberOfBounces = 10;
		xvk::WindowState windowState{ "test", 1920, 1080, true, false };
		
		PathTracer app(userSettings, windowState, VK_PRESENT_MODE_MAILBOX_KHR);

		vkAsset::AglTFModel model;
		model.loadglTFFile(ASSET_DIR"model/cornell_box/scene.gltf");
		std::vector<vkAsset::AglTFModel*> models = { &model };
		vkAsset::AScene scene(app.GetCommandPool(), models);
		std::vector<vkAsset::AScene*> sceneList = { &scene };
		app.PostTracerInit(sceneList);

	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}
