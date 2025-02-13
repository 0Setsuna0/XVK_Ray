#pragma once

#include "XVKCommon.h"
#include "XVKInstance.h"

namespace xvk
{
	struct QueueFamilyIndices 
	{
		uint32_t graphicsFamily = -1;
		uint32_t computeFamily = -1;
		uint32_t presentFamily = -1;

		bool IsCompelete()
		{
			return graphicsFamily >= 0 && presentFamily >= 0;
		}

		bool SupportCompute()
		{
			return computeFamily >= 0;
		}
	};

	class XVKDevice
	{
	public:
		XVKDevice(const XVKInstance& instance, 
			const std::vector<const char*>& requiredExtensions);

		~XVKDevice();

		void CreateLogicalDevice();

		VkPhysicalDevice PhysicalHandle() const { return vk_physicalDevice; }
		VkDevice Handle() const { return vk_logicalDevice; }

		const QueueFamilyIndices& GetQueueFamilyIndices() const { return queueFamilyIndices; }
		uint32_t GraphicsQueueIndex() const { return queueFamilyIndices.graphicsFamily; }
		uint32_t ComputeQueueIndex() const { return queueFamilyIndices.computeFamily; }
		uint32_t PresentQueueIndex() const { return queueFamilyIndices.presentFamily; }

		VkQueue GraphicsQueue() const { return vk_graphicsQueue; }
		VkQueue ComputeQueue() const { return vk_computeQueue; }
		VkQueue PresentQueue() const { return vk_presentQueue; }

		const VkSurfaceKHR Surface() const { return vk_surface; }
		const XVKInstance& Instance() const { return xvk_instance; }

		void WaitIdle() const;

	private:
		void GetPhysicalDevices();
		void PickPhysicalDevice();

		bool IsDeviceSuitable(VkPhysicalDevice physicalDevice);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);

		std::vector<VkPhysicalDevice> physicalDevices;

		VkPhysicalDevice vk_physicalDevice;
		VkDevice		 vk_logicalDevice;

		const XVKInstance&	xvk_instance;
		const VkSurfaceKHR	vk_surface;

		QueueFamilyIndices queueFamilyIndices;

		VkQueue vk_graphicsQueue;
		VkQueue vk_computeQueue;
		VkQueue vk_presentQueue;

		std::vector<const char*> requiredExtensions;
	};
}