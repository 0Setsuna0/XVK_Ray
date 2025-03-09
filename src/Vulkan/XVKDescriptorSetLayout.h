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
		DescriptorBinding(uint32_t inbingding, uint32_t indescriptorCount, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags)
			:binding(inbingding), descriptorCount(indescriptorCount), descriptorType(descriptorType), stageFlags(stageFlags) {}
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