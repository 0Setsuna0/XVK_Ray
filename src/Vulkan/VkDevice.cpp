#include "VkDevice.h"
#include <set>

namespace xvk
{
	XVKDevice::XVKDevice(const XVKInstance& instance, 
		const std::vector<const char*>& requiredExtensions)
		:xvk_instance(instance), vk_surface(instance.GetVulkanSurface())
	{
		GetPhysicalDevices();
		PickPhysicalDevice();

		queueFamilyIndices = FindQueueFamilies(vk_physicalDevice);

		const std::set<uint32_t> uniqueQueueFamilies =
		{
			queueFamilyIndices.graphicsFamily,
			queueFamilyIndices.computeFamily,
			queueFamilyIndices.presentFamily
		};

		float queuePriority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		for (uint32_t queueFamilyIndex : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueInfo;
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndex;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueInfo);
		}

		VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
		physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = queueCreateInfos.size();

		createInfo.pEnabledFeatures = &physicalDeviceFeatures;

		createInfo.enabledExtensionCount = requiredExtensions.size();
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		createInfo.enabledLayerCount = instance.GetValidationLayers().size();
		createInfo.ppEnabledLayerNames = instance.GetValidationLayers().data();
		vkCreateDevice(vk_physicalDevice, &createInfo, nullptr, &vk_logicalDevice);
		//VULKAN_RUNTIME_CHECK(vkCreateDevice(vk_physicalDevice, &createInfo, nullptr, &vk_logicalDevice), "Fail to create logical device");
	}

	XVKDevice::~XVKDevice()
	{
		//vkDestroyDevice(vk_logicalDevice, nullptr);
	}

	void XVKDevice::GetPhysicalDevices()
	{
		uint32_t physicalDevicesCount = 0;
		vkEnumeratePhysicalDevices(xvk_instance.Handle(), &physicalDevicesCount, nullptr);
		physicalDevices.resize(physicalDevicesCount);
		vkEnumeratePhysicalDevices(xvk_instance.Handle(), &physicalDevicesCount, physicalDevices.data());
		//GetEnumerateVector(xvk_instance.Handle(), vkEnumeratePhysicalDevices, physicalDevices);
	}

	void XVKDevice::PickPhysicalDevice()
	{
		for (const auto& device : physicalDevices)
		{
			if (IsDeviceSuitable(device))
			{
				vk_physicalDevice = device;
				break;
			}
		}

		if (vk_physicalDevice == VK_NULL_HANDLE)
		{
			RUNTIME_ERROR("Fail to find a suitable GPU");
		}
		else
		{
			VkPhysicalDeviceProperties  deviceProperties;
			vkGetPhysicalDeviceProperties(vk_physicalDevice, &deviceProperties);
			LOG(deviceProperties.deviceName);
		}
	}

	QueueFamilyIndices XVKDevice::FindQueueFamilies(VkPhysicalDevice physicalDevice)
	{
		QueueFamilyIndices indices;

		std::vector<VkQueueFamilyProperties> queueFamilies;
		GetEnumerateVector(physicalDevice, vkGetPhysicalDeviceQueueFamilyProperties, queueFamilies);

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.computeFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, vk_surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport)
			{
				indices.presentFamily = i;
			}
		}
		return indices;
	}

	bool XVKDevice::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
	{
		const std::vector<const char*> deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool XVKDevice::IsDeviceSuitable(VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceProperties  deviceProperties;
		VkPhysicalDeviceFeatures    deviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
		bool extensionSupported = CheckDeviceExtensionSupport(physicalDevice);

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && indices.IsCompelete()
			&& extensionSupported;
	}
}