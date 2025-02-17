#pragma

#include "XVKCommon.h"

namespace xvk
{
	class XVKInstance;
	class XVKDevice;
	class XVKImageView;

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;//交换链最大最小图像数量，宽度，高度
		std::vector<VkSurfaceFormatKHR> formats;//像素格式，颜色空间
		std::vector<VkPresentModeKHR> presetnModes;//可用的呈现模式
	};

	class XVKSwapChain
	{
	public:
		XVKSwapChain(const XVKDevice& device, VkPresentModeKHR presentMode);
		~XVKSwapChain();

		VkSwapchainKHR Handle() const { return vk_swapChain; }
		
		VkFormat GetImageFormat() const { return vk_swapChainImageFormat; }
		VkExtent2D GetExtent() const { return vk_swapChainExtent; }
		uint32_t GetMinImageCount() const { return minImageCount; }
		VkPresentModeKHR GetPresentMode() const { return vk_presentMode; }
		const XVKDevice& GetDevice() const { return xvk_device; }
		const std::vector<VkImage>& GetImages() const { return vk_swapChainImages; }
		
		
	private:
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice) const;
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR presentMode);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		uint32_t ChooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities);
		
		const XVKDevice& xvk_device;

		VkSwapchainKHR vk_swapChain;

		uint32_t minImageCount;
		VkFormat vk_swapChainImageFormat;
		VkExtent2D vk_swapChainExtent;
		VkPresentModeKHR vk_presentMode;
		std::vector<VkImage> vk_swapChainImages;
		std::vector<XVKImageView> xvk_imageViews;
	};
}