#include "XVKSampler.h"
#include "XVKDevice.h"

namespace xvk
{
	XVKSampler::XVKSampler(const XVKDevice& device, const SamplerConfig& config)
		:xvk_device(device)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = config.MagFilter;
		samplerInfo.minFilter = config.MinFilter;
		samplerInfo.addressModeU = config.AddressModeU;
		samplerInfo.addressModeV = config.AddressModeV;
		samplerInfo.addressModeW = config.AddressModeW;
		samplerInfo.anisotropyEnable = config.AnisotropyEnable;
		samplerInfo.maxAnisotropy = config.MaxAnisotropy;
		samplerInfo.borderColor = config.BorderColor;
		samplerInfo.unnormalizedCoordinates = config.UnnormalizedCoordinates;
		samplerInfo.compareEnable = config.CompareEnable;
		samplerInfo.compareOp = config.CompareOp;
		samplerInfo.mipmapMode = config.MipmapMode;
		samplerInfo.mipLodBias = config.MipLodBias;
		samplerInfo.minLod = config.MinLod;
		samplerInfo.maxLod = config.MaxLod;

		VULKAN_RUNTIME_CHECK(vkCreateSampler(xvk_device.Handle(), &samplerInfo, nullptr, &vk_sampler),
			"create sampler");
	}

	XVKSampler::~XVKSampler()
	{
		if (vk_sampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(xvk_device.Handle(), vk_sampler, nullptr);
			vk_sampler = VK_NULL_HANDLE;
		}
	}
}