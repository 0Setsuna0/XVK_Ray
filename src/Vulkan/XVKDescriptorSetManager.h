#include "XVKCommon.h"
#include <memory>

namespace xvk
{
	class XVKDevice;
	class XVKDescriptorPool;
	class XVKDescriptorSets;
	class XVKDescriptorSetLayout;
	struct DescriptorBinding;

	class XVKDescriptorSetManager
	{
	public:
		XVKDescriptorSetManager(const XVKDevice& device, const std::vector<DescriptorBinding>& descriptorBindings, size_t maxSets);
		~XVKDescriptorSetManager();

		const XVKDescriptorSetLayout& GetDescriptorSetLayout() const { return *xvk_descriptorSetLayout; }
		XVKDescriptorSets& GetDescriptorSets() { return *xvk_descriptorSets; }
	private:
		std::unique_ptr<XVKDescriptorPool> xvk_descriptorPool;
		std::unique_ptr<XVKDescriptorSets> xvk_descriptorSets;
		std::unique_ptr<XVKDescriptorSetLayout> xvk_descriptorSetLayout;
	};
}