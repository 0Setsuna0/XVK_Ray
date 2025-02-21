#include "XVKShaderModule.h"
#include "XVKDevice.h"
#include <fstream>

namespace xvk
{
	XVKShaderModule::XVKShaderModule(const XVKDevice& device, const std::string filePath)
		:XVKShaderModule(device, ReadFile(filePath))
	{

	}

	XVKShaderModule::XVKShaderModule(const XVKDevice& device, const std::vector<char>& code)
		:xvk_device(device)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VULKAN_RUNTIME_CHECK(vkCreateShaderModule(xvk_device.Handle(), &createInfo, nullptr, &vk_shaderModule),
			"create shader module");

	}

	XVKShaderModule::~XVKShaderModule()
	{
		if (vk_shaderModule != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(xvk_device.Handle(), vk_shaderModule, nullptr);
			vk_shaderModule = VK_NULL_HANDLE;
		}
	}

	std::vector<char> XVKShaderModule::ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			RUNTIME_ERROR("failed to open file '" + filename + "'");
		}

		const auto fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	VkPipelineShaderStageCreateInfo XVKShaderModule::CreateShaderStage(VkShaderStageFlagBits stage) const
	{
		VkPipelineShaderStageCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.module = vk_shaderModule;
		createInfo.stage = stage;
		createInfo.pName = "main";

		return createInfo;
	}
}