#include "VkInstance.h"
#include "VkWindow.h"
namespace xvk
{
	XVKInstance::XVKInstance(const XVKWindow& window, const std::vector<const char*>& validationLayers)
		:xvk_window(window), validationLayers(validationLayers)
	{
		auto extensions = xvk_window.GetRequiredInstanceExtensions();

		//check validation layer support
		enableValidationLayer = validationLayers.size() != 0;
		bool bSupportValidationLayer = CheckValidationLayerSupport(validationLayers);
		if (enableValidationLayer && !bSupportValidationLayer)
		{
			RUNTIME_ERROR("Fail to enable validation layer");
		}
		
		if (!validationLayers.empty())
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "ReSTIR_PT";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Test";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_MAKE_VERSION(1, 3, 0);

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();

		VULKAN_RUNTIME_CHECK(vkCreateInstance(&createInfo, nullptr, &vk_instance),"create instance");
		std::cout << "Succeed to create Vulkan instance" << std::endl;

		GetVkExtensions();
		GetVkLayers();
		CreateSurface();
	}

	XVKInstance::~XVKInstance()
	{
		if (vk_instance != nullptr)
		{
			vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
			vkDestroyInstance(vk_instance, nullptr);
			vk_instance = nullptr;
		}
	}

	bool XVKInstance::CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
	{
		std::vector<VkLayerProperties> availableLayers = GetEnumerateVector(vkEnumerateInstanceLayerProperties);
		
		for (auto layer : validationLayers)
		{
			bool bSupport = false;

			for (auto availableLayer : availableLayers)
			{
				if (strcmp(availableLayer.layerName, layer) == 0)
				{
					bSupport = true;
					break;
				}
			}
			if (!bSupport)
			{
				return false;
			}
		}

		return true;
	}

	void XVKInstance::CreateSurface()
	{
		VULKAN_RUNTIME_CHECK(glfwCreateWindowSurface(vk_instance, xvk_window.Handle(), nullptr, &vk_surface),
			"create surface");
	}

	void XVKInstance::GetVkExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		vk_extensions.resize(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vk_extensions.data());
	}

	void XVKInstance::GetVkLayers()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		vk_layers.resize(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, vk_layers.data());
	}
}