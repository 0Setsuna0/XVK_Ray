#include "XVKCommon.h"

namespace xvk
{
	class XVKDevice;
	struct DescriptorBinding;

	class XVKDescriptorPool
	{
	public:
		XVKDescriptorPool(const XVKDevice& device, const std::vector<DescriptorBinding>& descriptorBindings, size_t maxSets);
		~XVKDescriptorPool();

		VkDescriptorPool Handle() const { return vk_descriptorPool; }
		const XVKDevice& GetDevice() const { return xvk_device; }

	private:
		VkDescriptorPool vk_descriptorPool;
		const XVKDevice& xvk_device;
	};
}