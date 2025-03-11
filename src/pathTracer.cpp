#include "pathTracer.h"

#include "Vulkan/XVKInstance.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKSwapChain.h"
#include "Vulkan/XVKWindow.h"
#include "Vulkan/XVKFrameBuffer.h"

const bool EnableValidationLayers =
#ifdef NDEBUG
	false;
#else
	true;
#endif

PathTracer::PathTracer(const editor::UserSettings& userSettings, const xvk::WindowState& windowState,
	VkPresentModeKHR presentMode)
	:ApplicationRT(windowState, presentMode, EnableValidationLayers),
	m_userSettings(userSettings)
{

}