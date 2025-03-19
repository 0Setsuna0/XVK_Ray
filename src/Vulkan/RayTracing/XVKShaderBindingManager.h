#pragma once

#include "Vulkan/XVKCommon.h"
#include "Vulkan/XVKBuffer.h"
#include <memory>

namespace xvk
{
	class XVKBuffer;
	class XVKDevice;
	class XVKDeviceMemory;
}

namespace xvk::ray
{
	class XVKRayFuncManager;
	class XVKRayTracingContext;
	class XVKRayTracingPipeline;

	class XVKShaderBindingManager
	{
	public:

		struct Entry
		{
			uint32_t GroupIndex;
			std::vector<unsigned char> InlineData;
		};

		XVKShaderBindingManager(
			const XVKRayFuncManager& deviceFunc,
			const XVKRayTracingPipeline& rayTracingPipeline,
			const XVKRayTracingContext& rayTracingContext,
			const std::vector<Entry>& rayGenPrograms,
			const std::vector<Entry>& rayMissPrograms,
			const std::vector<Entry>& rayHitPrograms
		);

		~XVKShaderBindingManager();

		const XVKBuffer& GetBuffer() const { return *buffer; }

		VkDeviceAddress RayGenDeviceAddress() const { return buffer->GetDeviceAddress() + rayGenOffset; }
		VkDeviceAddress RayMissDeviceAddress() const { return buffer->GetDeviceAddress() + rayMissOffset; }
		VkDeviceAddress RayHitGroupDeviceAddress() const { return buffer->GetDeviceAddress() + rayHitGroupOffset; }
	
		size_t RayGenOffset() const { return rayGenOffset; }
		size_t MissOffset() const { return rayMissOffset; }
		size_t HitGroupOffset() const { return rayHitGroupOffset; }

		size_t RayGenSize() const { return rayGenSize; }
		size_t MissSize() const { return rayMissSize; }
		size_t HitGroupSize() const { return rayHitGroupSize; }

		size_t RayGenEntrySize() const { return rayGenEntrySize; }
		size_t MissEntrySize() const { return rayMissEntrySize; }
		size_t HitGroupEntrySize() const { return rayHitGroupEntrySize; }

	private:

		const size_t rayGenEntrySize;
		const size_t rayMissEntrySize;
		const size_t rayHitGroupEntrySize;

		const size_t rayGenOffset;
		const size_t rayMissOffset;
		const size_t rayHitGroupOffset;

		const size_t rayGenSize;
		const size_t rayMissSize;
		const size_t rayHitGroupSize;

		std::unique_ptr<XVKBuffer> buffer;
		std::unique_ptr<XVKDeviceMemory> bufferMemory;
	};
}