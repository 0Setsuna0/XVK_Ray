#include "XVKInstance.h"
#include "XVKWindow.h"
namespace xvk
{
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallback, void* pUserdata)
	{
		std::cerr << "[VALIDATION LAYER]: " << pCallback->pMessage << std::endl;

		return VK_FALSE;
	}

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}


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
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
		appInfo.pEngineName = "Test";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		if (enableValidationLayer)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}
		VULKAN_RUNTIME_CHECK(vkCreateInstance(&createInfo, nullptr, &vk_instance),"create instance");
		std::cout << "Succeed to create Vulkan instance" << std::endl;

		if (enableValidationLayer)
		{
			SetupDebugMessenger();
		}

		GetVkExtensions();
		GetVkLayers();
		CreateSurface();
	}

	XVKInstance::~XVKInstance()
	{
		if (vk_instance != nullptr)
		{
			if (enableValidationLayer)
			{
				DestroyDebugUtilsMessengerEXT(vk_instance, debugMessenger, nullptr);
			}
			vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
			vkDestroyInstance(vk_instance, nullptr);
			vk_instance = nullptr;
			vk_surface = nullptr;
		}
	}

	void XVKInstance::SetupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		PopulateDebugMessengerCreateInfo(debugCreateInfo);

		if (CreateDebugUtilsMessengerEXT(vk_instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up debug messenger!");
		}
		else
		{
			std::cout << "Succeed to set up debug messenger" << std::endl;
		}
	}

	bool XVKInstance::CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
	{
		std::vector<VkLayerProperties> availableLayers = GetEnumerateVector(vkEnumerateInstanceLayerProperties);
		
		for (auto layer : validationLayers)
		{
			bool bSupport = false;

			for (auto& availableLayer : availableLayers)
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