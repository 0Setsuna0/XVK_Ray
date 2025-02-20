#include "XVKPipelineLayout.h"
#include "XVKDevice.h"
#include "XVKDescriptorSetLayout.h"

namespace xvk
{
	XVKPipelineLayout::XVKPipelineLayout(const XVKDevice& device, const XVKDescriptorSetLayout& descriptorSetLayout)
		:xvk_device(device)
	{
		VkDescriptorSetLayout descriptorSetLayouts[] = { descriptorSetLayout.Handle() };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		VULKAN_RUNTIME_CHECK(vkCreatePipelineLayout(device.Handle(), &pipelineLayoutInfo, nullptr, &vk_pipelineLayout),
			"create pipeline layout");
	}

	XVKPipelineLayout::~XVKPipelineLayout()
	{
		if (vk_pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(xvk_device.Handle(), vk_pipelineLayout, nullptr);
			vk_pipelineLayout = VK_NULL_HANDLE;
		}
	}
}