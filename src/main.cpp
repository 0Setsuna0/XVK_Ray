#include <Vulkan/XVKBuffer.h>
#include "Vulkan/XVKInstance.h"
#include "Vulkan/XVKWindow.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKSwapChain.h"
#include "Vulkan/XVKRenderPass.h"
#include "Vulkan/XVKDescriptorSetLayout.h"
#include "Vulkan/XVKCommandPool.h"
#include "Vulkan/XVKCommandBuffers.h"
#include "Vulkan/XVKDepthBuffer.h"
int main()
{
	VkBuffer buffer;
	int* ptr = new int{ 8 };

	xvk::XVKWindow window({ "test", 1920, 1080, true, false });
	try
	{
		std::vector<const char*> testlayer{ "VK_LAYER_KHRONOS_validation" };
		xvk::XVKInstance instance(window, testlayer);
		std::vector<const char*> testextension{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		xvk::XVKDevice device(instance, testextension);
		xvk::XVKSwapChain swapChain(device, VK_PRESENT_MODE_MAILBOX_KHR);
		xvk::XVKCommandPool commandPool(device, device.GraphicsQueueIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		xvk::XVKCommandBuffers commandBuffers(commandPool, swapChain.GetMinImageCount());
		xvk::XVKDepthBuffer depthBuffer(commandPool, swapChain.GetExtent());
		xvk::XVKRenderPass renderPass(device, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR,
			depthBuffer, swapChain);
		xvk::DescriptorBinding binding{ 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT };
		xvk::XVKDescriptorSetLayout descriptor(device, {binding});
		
		window.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}
