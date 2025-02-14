#include "XVKDescriptorSetLayout.h"
#include "XVKDevice.h"

namespace xvk
{
	XVKDescriptorSetLayout::XVKDescriptorSetLayout(const XVKDevice& device, const std::vector<DescriptorBinding>& descriptorBindings)
		:xvk_device(device)
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

		for (const auto& binding : descriptorBindings)
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = binding.binding;
			layoutBinding.descriptorCount = binding.descriptorCount;
			layoutBinding.descriptorType = binding.descriptorType;
			layoutBinding.stageFlags = binding.stageFlags;

			layoutBindings.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		VULKAN_RUNTIME_CHECK(vkCreateDescriptorSetLayout(xvk_device.Handle(), &layoutInfo, nullptr, &vk_descriptorSetLayout), "create descriptor set layout");
	}

	XVKDescriptorSetLayout::~XVKDescriptorSetLayout()
	{
		if (vk_descriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(xvk_device.Handle(), vk_descriptorSetLayout, nullptr);
			vk_descriptorSetLayout = VK_NULL_HANDLE;
		}
	}
}