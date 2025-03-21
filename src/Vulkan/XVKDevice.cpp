#include "XVKDevice.h"
#include <set>

namespace xvk
{
	XVKDevice::XVKDevice(const XVKInstance& instance, 
		const std::vector<const char*>& requiredExtensions)
		:xvk_instance(instance), vk_surface(instance.GetVulkanSurface())
	{
		this->requiredExtensions = requiredExtensions;
		GetPhysicalDevices();
		PickPhysicalDevice();
		CreateLogicalDevice();
		vkGetDeviceQueue(vk_logicalDevice, queueFamilyIndices.graphicsFamily, 0, &vk_graphicsQueue);
		vkGetDeviceQueue(vk_logicalDevice, queueFamilyIndices.computeFamily, 0, &vk_computeQueue);
		vkGetDeviceQueue(vk_logicalDevice, queueFamilyIndices.presentFamily, 0, &vk_presentQueue);
	}

	XVKDevice::~XVKDevice()
	{
		if (vk_logicalDevice != VK_NULL_HANDLE)
		{
			vkDestroyDevice(vk_logicalDevice, nullptr);
			vk_logicalDevice = VK_NULL_HANDLE;
		}
	}

	void XVKDevice::CreateLogicalDevice()
	{
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
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndex;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueInfo);
		}

		// Required device features.
		VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
		physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
		physicalDeviceFeatures.shaderInt64 = true;
		//--ray tracing features--
		VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {};
		bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
		bufferDeviceAddressFeatures.pNext = nullptr;
		bufferDeviceAddressFeatures.bufferDeviceAddress = true;

		VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures = {};
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexingFeatures.pNext = &bufferDeviceAddressFeatures;
		indexingFeatures.runtimeDescriptorArray = true;
		indexingFeatures.shaderSampledImageArrayNonUniformIndexing = true;

		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {};
		accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		accelerationStructureFeatures.pNext = &indexingFeatures;
		accelerationStructureFeatures.accelerationStructure = true;

		VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = {};
		rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
		rayQueryFeatures.rayQuery = VK_TRUE;
		rayQueryFeatures.pNext = &accelerationStructureFeatures;

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures = {};
		rayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		rayTracingFeatures.pNext = &rayQueryFeatures;
		rayTracingFeatures.rayTracingPipeline = true;
		//--

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = queueCreateInfos.size();
		createInfo.pEnabledFeatures = &physicalDeviceFeatures;
		createInfo.pNext = &rayTracingFeatures;

		createInfo.enabledExtensionCount = requiredExtensions.size();
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		if (xvk_instance.enableValidationLayer)
		{
			createInfo.enabledLayerCount = xvk_instance.GetValidationLayers().size();
			createInfo.ppEnabledLayerNames = xvk_instance.GetValidationLayers().data();
		}
		else
		{
			createInfo.enabledExtensionCount = 0;
		}

		VULKAN_RUNTIME_CHECK(vkCreateDevice(vk_physicalDevice, &createInfo, nullptr, &vk_logicalDevice), "Fail to create logical device");
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

	void XVKDevice::WaitIdle() const
	{
		vkDeviceWaitIdle(vk_logicalDevice);
	}
}