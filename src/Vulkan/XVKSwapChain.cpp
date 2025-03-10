#include "XVKSwapChain.h"
#include "XVKDevice.h"
#include "XVKInstance.h"
#include "XVKWindow.h"
#include "XVKImageView.h"
#include <algorithm>
#include <limits>

namespace xvk
{
	XVKSwapChain::XVKSwapChain(const XVKDevice& device, VkPresentModeKHR presentMode)
		:xvk_device(device), vk_presentMode(presentMode)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(xvk_device.PhysicalHandle());
		if (swapChainSupport.formats.empty() || swapChainSupport.presetnModes.empty())
		{
			RUNTIME_ERROR("Swap chain support details are empty");
		}

		const auto& surface = device.Surface();
		const auto& window = device.Instance().GetWindow();

		const auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		const auto realPresentMode = ChooseSwapPresentMode(swapChainSupport.presetnModes, presentMode);
		const auto extent = ChooseSwapExtent(swapChainSupport.capabilities);
		const auto imageCount = ChooseImageCount(swapChainSupport.capabilities);

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = nullptr;

		if (xvk_device.GraphicsQueueIndex() != xvk_device.PresentQueueIndex())
		{
			uint32_t queueFamilyIndices[] = { xvk_device.GraphicsQueueIndex(), xvk_device.PresentQueueIndex() };
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		VULKAN_RUNTIME_CHECK(vkCreateSwapchainKHR(xvk_device.Handle(), &createInfo, nullptr, &vk_swapChain), "create swap chain");
	
		minImageCount = imageCount;
		vk_presentMode = realPresentMode;
		vk_swapChainImageFormat = surfaceFormat.format;
		vk_swapChainExtent = extent;
		GetEnumerateVector(xvk_device.Handle(), vk_swapChain, vkGetSwapchainImagesKHR, vk_swapChainImages);
		xvk_imageViews.reserve(vk_swapChainImages.size());

		for (const auto& image : vk_swapChainImages)
		{
			xvk_imageViews.emplace_back(std::make_unique<XVKImageView>(xvk_device, image, 
				vk_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT));
		}
	}

	XVKSwapChain::~XVKSwapChain()
	{
		xvk_imageViews.clear();
		if (vk_swapChain != nullptr)
		{
			vkDestroySwapchainKHR(xvk_device.Handle(), vk_swapChain, nullptr);
			vk_swapChain = nullptr;
		}
	}

	SwapChainSupportDetails XVKSwapChain::QuerySwapChainSupport(VkPhysicalDevice physicalDevice) const
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, xvk_device.Surface(), &details.capabilities);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, xvk_device.Surface(), &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, xvk_device.Surface(), &formatCount, details.formats.data());
		}
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, xvk_device.Surface(), &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presetnModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, xvk_device.Surface(), &presentModeCount, details.presetnModes.data());
		}
		return details;
	}

	VkSurfaceFormatKHR XVKSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		if(availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& format : availableFormats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		RUNTIME_ERROR("Fail to choose swap surface format");
	}

	VkPresentModeKHR XVKSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR presentMode)
	{
		switch (presentMode)
		{
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
		case VK_PRESENT_MODE_MAILBOX_KHR:
		case VK_PRESENT_MODE_FIFO_KHR:
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			if (std::find(availablePresentModes.begin(), availablePresentModes.end(), presentMode) != availablePresentModes.end())
			{
				return presentMode;
			}
			break;
		default:
			RUNTIME_ERROR("Fail to choose swap present mode");
			break;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D XVKSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		auto extent = xvk_device.Instance().GetWindow().FramebufferSize();

		extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
		extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));

		return extent;
	}

	uint32_t XVKSwapChain::ChooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}
		return imageCount;
	}
}