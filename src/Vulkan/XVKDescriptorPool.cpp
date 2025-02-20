#include "XVKDescriptorPool.h"
#include "XVKDescriptorSetLayout.h"
#include "XVKDevice.h"

namespace xvk
{
	XVKDescriptorPool::XVKDescriptorPool(const XVKDevice& device, const std::vector<DescriptorBinding>& descriptorBindings, size_t maxSets)
		:xvk_device(device)
	{
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (const auto& binding : descriptorBindings)
		{
			VkDescriptorPoolSize poolSize = {};
			poolSize.type = binding.descriptorType;
			poolSize.descriptorCount = binding.descriptorCount * maxSets;//*maxSets?
			poolSizes.push_back(poolSize);
		}

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = maxSets;

		VULKAN_RUNTIME_CHECK(vkCreateDescriptorPool(device.Handle(), &poolInfo, nullptr, &vk_descriptorPool), "create descriptor pool");
	}
	XVKDescriptorPool::~XVKDescriptorPool()
	{
		if (vk_descriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(xvk_device.Handle(), vk_descriptorPool, nullptr);
			vk_descriptorPool = VK_NULL_HANDLE;
		}
	}
}