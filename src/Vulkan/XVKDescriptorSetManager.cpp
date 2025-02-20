#include "XVKDescriptorSetManager.h"
#include "XVKDescriptorPool.h"
#include "XVKDescriptorSet.h"
#include "XVKDescriptorSetLayout.h"
#include "XVKDevice.h"
#include <map>

namespace xvk
{
	XVKDescriptorSetManager::XVKDescriptorSetManager(const XVKDevice& device, const std::vector<DescriptorBinding>& descriptorBindings, size_t maxSets)
	{
		std::map<uint32_t, VkDescriptorType> bindingTypes;

		for (const auto& binding : descriptorBindings)
		{
			if (!bindingTypes.insert(std::make_pair(binding.binding, binding.descriptorType)).second)
			{
				RUNTIME_ERROR("binding collision");
			}
		}

		xvk_descriptorPool.reset(new XVKDescriptorPool(device, descriptorBindings, maxSets));
		xvk_descriptorSetLayout.reset(new XVKDescriptorSetLayout(device, descriptorBindings));
		xvk_descriptorSets.reset(new XVKDescriptorSets(*xvk_descriptorPool, *xvk_descriptorSetLayout, bindingTypes, maxSets));
	}

	XVKDescriptorSetManager::~XVKDescriptorSetManager()
	{
		xvk_descriptorSets.reset();
		xvk_descriptorSetLayout.reset();
		xvk_descriptorPool.reset();
	}
}