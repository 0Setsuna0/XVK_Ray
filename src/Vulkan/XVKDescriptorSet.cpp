#include "XVKDescriptorSet.h"
#include "XVKDescriptorSetLayout.h"
#include "XVKDescriptorPool.h"
#include "XVKDevice.h"

namespace xvk
{
	XVKDescriptorSets::XVKDescriptorSets(const XVKDescriptorPool& descriptorPool,
		const XVKDescriptorSetLayout& layout,
		std::map<uint32_t, VkDescriptorType> bindingTypes,
		size_t size)
		: xvk_descriptorPool(descriptorPool), vk_bindingTypes(bindingTypes)
	{
		std::vector<VkDescriptorSetLayout> layouts(size, layout.Handle());

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool.Handle();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
		allocInfo.pSetLayouts = layouts.data();
		vk_descriptorSets.resize(size);

		VULKAN_RUNTIME_CHECK(vkAllocateDescriptorSets(descriptorPool.GetDevice().Handle(), &allocInfo, vk_descriptorSets.data()), "allocate descriptor sets");
	}

	XVKDescriptorSets::~XVKDescriptorSets()
	{
	}

	//bind specific buffer info to descriptor set
	VkWriteDescriptorSet XVKDescriptorSets::Bind(uint32_t index, uint32_t binding, const VkDescriptorBufferInfo& bufferInfo, uint32_t count) const
	{
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = vk_descriptorSets[index];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = vk_bindingTypes.at(binding);
		descriptorWrite.descriptorCount = count;
		descriptorWrite.pBufferInfo = &bufferInfo;

		return descriptorWrite;
	}

	VkWriteDescriptorSet XVKDescriptorSets::Bind(uint32_t index, uint32_t binding, const VkDescriptorImageInfo& imageInfo, uint32_t count) const
	{
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = vk_descriptorSets[index];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = vk_bindingTypes.at(binding);
		descriptorWrite.descriptorCount = count;
		descriptorWrite.pImageInfo = &imageInfo;

		return descriptorWrite;
	}

	VkWriteDescriptorSet XVKDescriptorSets::Bind(uint32_t index, uint32_t binding, const VkWriteDescriptorSetAccelerationStructureKHR& structureInfo, uint32_t count) const
	{
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = vk_descriptorSets[index];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = vk_bindingTypes.at(binding);
		descriptorWrite.descriptorCount = count;
		descriptorWrite.pNext = &structureInfo;

		return descriptorWrite;
	}

	void XVKDescriptorSets::UpdateDescriptors(uint32_t index, const std::vector<VkWriteDescriptorSet>& descriptorWrites)
	{
		vkUpdateDescriptorSets(xvk_descriptorPool.GetDevice().Handle(),
			static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}