#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKDevice;
	class XVKDescriptorSetLayout;

	class XVKPipelineLayout
	{
	public:
		XVKPipelineLayout(const XVKDevice& device, const XVKDescriptorSetLayout& descriptorSetLayout);
		~XVKPipelineLayout();

	private:
		VkPipelineLayout vk_pipelineLayout;
		const XVKDevice& xvk_device;
	};
}