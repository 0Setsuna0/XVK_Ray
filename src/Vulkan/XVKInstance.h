#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKWindow;

	class XVKInstance
	{
	public:
		XVKInstance(const XVKWindow& window, const std::vector<const char*>& validationLayers);
		~XVKInstance();

		bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);

		VkInstance Handle() const { return vk_instance; }

		const XVKWindow& GetWindow() const { return xvk_window; }
		VkSurfaceKHR GetVulkanSurface() const { return vk_surface; };
		
		const std::vector<const char*>& GetValidationLayers() const { return validationLayers; }

		void CreateSurface();
		void SetupDebugMessenger();

		bool enableValidationLayer;

	private:
		void GetVkExtensions();
		void GetVkLayers();

		VkInstance vk_instance;

		//validation layer settings

		const std::vector<const char*> validationLayers;
		VkDebugUtilsMessengerEXT debugMessenger;

		const XVKWindow& xvk_window;
		VkSurfaceKHR vk_surface{};

		std::vector<VkExtensionProperties> vk_extensions;
		std::vector<VkLayerProperties> vk_layers;
	};
}