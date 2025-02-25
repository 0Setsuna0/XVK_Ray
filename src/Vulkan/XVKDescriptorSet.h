#include "XVKCommon.h"
#include <map>

namespace xvk
{
	class XVKDescriptorSetLayout;
	class XVKDescriptorPool;

	class XVKDescriptorSets
	{
	public:
		XVKDescriptorSets(const XVKDescriptorPool& descriptorPool,
			const XVKDescriptorSetLayout& layout,
			std::map<uint32_t, VkDescriptorType> bindingTypes,
			size_t size);
		~XVKDescriptorSets();

		const VkDescriptorSet& Handle(size_t index) const { return vk_descriptorSets[index]; }
		const XVKDescriptorPool& GetDescriptorPool() const { return xvk_descriptorPool; }


		VkWriteDescriptorSet Bind(uint32_t index, uint32_t binding, const VkDescriptorBufferInfo& bufferInfo, uint32_t count = 1) const;
		VkWriteDescriptorSet Bind(uint32_t index, uint32_t binding, const VkDescriptorImageInfo& imageInfo, uint32_t count = 1) const;
		VkWriteDescriptorSet Bind(uint32_t index, uint32_t binding, const VkWriteDescriptorSetAccelerationStructureKHR& structureInfo, uint32_t count = 1) const;

		void UpdateDescriptors(uint32_t index, const std::vector<VkWriteDescriptorSet>& descriptorWrites);

	private:
		std::vector<VkDescriptorSet> vk_descriptorSets;
		
		const XVKDescriptorPool& xvk_descriptorPool;
		const std::map<uint32_t, VkDescriptorType> vk_bindingTypes;
	};
}