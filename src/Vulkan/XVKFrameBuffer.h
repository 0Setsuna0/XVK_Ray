#pragma once

#include "XVKCommon.h"

namespace xvk
{
	class XVKImageView;
	class XVKRenderPass;

	class XVKFrameBuffer
	{
	public:
		XVKFrameBuffer(const XVKImageView& imageView, const XVKRenderPass& renderPass);
		XVKFrameBuffer(XVKFrameBuffer&& other) noexcept;
		~XVKFrameBuffer();

		VkFramebuffer Handle() const { return vk_frameBuffer; }
		const XVKImageView& GetImageView() const { return xvk_imageView; }
		const XVKRenderPass& GetRenderPass() const { return xvk_renderPass; }
	private:
		VkFramebuffer vk_frameBuffer;

		const XVKImageView& xvk_imageView;
		const XVKRenderPass& xvk_renderPass;
	};
}