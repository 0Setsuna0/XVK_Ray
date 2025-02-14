#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKDevice;

	struct DescriptorBinding
	{
		uint32_t binding;
		uint32_t descriptorCount;
		VkDescriptorType descriptorType;
		VkShaderStageFlags stageFlags;
	};

	class XVKDescriptorSetLayout
	{
	public:
		XVKDescriptorSetLayout(const XVKDevice& device, const std::vector<DescriptorBinding>& descriptorBindings);
		~XVKDescriptorSetLayout();

		VkDescriptorSetLayout Handle() const { return vk_descriptorSetLayout; }
		
	private:
		VkDescriptorSetLayout vk_descriptorSetLayout;
		
		const XVKDevice& xvk_device;
	};
}