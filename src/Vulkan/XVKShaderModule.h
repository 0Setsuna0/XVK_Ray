#include "XVKCommon.h"
#include <string>
namespace xvk
{
	class XVKDevice;

	class XVKShaderModule
	{
	public:
		XVKShaderModule(const XVKDevice& device, const std::string filePath);
		XVKShaderModule(const XVKDevice& device, const std::vector<char>& code);
		~XVKShaderModule();

		VkShaderModule Handle() const { return vk_shaderModule; }

		const XVKDevice& GetDevice() const { return xvk_device; }

		VkPipelineShaderStageCreateInfo CreateShaderStage(VkShaderStageFlagBits stage) const;

	private:
		static std::vector<char> ReadFile(const std::string& filename);

		VkShaderModule vk_shaderModule;

		const XVKDevice& xvk_device;
	};
}