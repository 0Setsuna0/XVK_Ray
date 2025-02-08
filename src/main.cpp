#include <Vulkan/VkBuffer.h>
#include "Vulkan/VkInstance.h"
#include "Vulkan/VkWindow.h"
#include "Vulkan/VkDevice.h"
int main()
{
	VkBuffer buffer;
	int* ptr = new int{ 8 };

	xvk::XVKWindow window({ "test", 600, 800, true, true });
	try
	{
		std::vector<const char*> testlayer{ "VK_LAYER_KHRONOS_validation" };
		xvk::XVKInstance instance(window, testlayer);
		xvk::XVKDevice device(instance, { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}
