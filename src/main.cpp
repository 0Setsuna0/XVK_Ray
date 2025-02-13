#include <Vulkan/XVKBuffer.h>
#include "Vulkan/XVKInstance.h"
#include "Vulkan/XVKWindow.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKSwapChain.h"
int main()
{
	VkBuffer buffer;
	int* ptr = new int{ 8 };

	xvk::XVKWindow window({ "test", 600, 800, true, true });
	try
	{
		std::vector<const char*> testlayer{ "VK_LAYER_KHRONOS_validation" };
		xvk::XVKInstance instance(window, testlayer);
		std::vector<const char*> testextension{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		xvk::XVKDevice device(instance, testextension);
		xvk::XVKSwapChain swapChain(device, VK_PRESENT_MODE_MAILBOX_KHR);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}
