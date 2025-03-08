#include "XVKGraphicsPipeline.h"
#include "XVKDevice.h"
#include "XVKSwapChain.h"
#include "XVKRenderPass.h"
#include "XVKPipelineLayout.h"
namespace xvk
{
	XVKGraphicsPipeline::XVKGraphicsPipeline(const XVKDevice& device, const XVKSwapChain& swapChain, const VkRenderPass renderPass)
		:xvk_swapChain(swapChain)
	{

	}

	XVKGraphicsPipeline::~XVKGraphicsPipeline()
	{

	}


}