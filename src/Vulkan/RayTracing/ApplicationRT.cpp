#include "ApplicationRT.h"

namespace xvk::ray
{
	ApplicationRT::ApplicationRT(const WindowState& windowState, VkPresentModeKHR presentMode,
		bool enableValidationLayer)
		:Application(windowState, presentMode, enableValidationLayer)
	{
	}
}