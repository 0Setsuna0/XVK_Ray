#include "XVKShaderBindingManager.h"
#include "XVKRayFuncManager.h"
#include "XVKRayTracingCore.h"
#include "XVKRayTracingPipeline.h"
#include "Vulkan/XVKBuffer.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKDeviceMemory.h"
#include <algorithm>

namespace xvk::ray
{
	size_t RoundUp(size_t size, size_t powerOf2Alignment)
	{
		return (size + powerOf2Alignment - 1) & ~(powerOf2Alignment - 1);
	}

	size_t GetEntrySize(const XVKRayTracingContext& rayTracingProperties, const std::vector<XVKShaderBindingManager::Entry>& entries)
	{
		// Find the maximum number of parameters used by a single entry
		size_t maxArgs = 0;

		for (const auto& entry : entries)
		{
			maxArgs = std::max(maxArgs, entry.InlineData.size());
		}

		// A SBT entry is made of a program ID and a set of 4-byte parameters (see shaderRecordEXT).
		// Its size is ShaderGroupHandleSize (plus parameters) and must be aligned to ShaderGroupBaseAlignment.
		return RoundUp(rayTracingProperties.ShaderGroupHandleSize() + maxArgs, rayTracingProperties.ShaderGroupBaseAlignment());
	}

	size_t CopyShaderData(
		uint8_t* const dst,
		const XVKRayTracingContext& rayTracingProperties,
		const std::vector<XVKShaderBindingManager::Entry>& entries,
		const size_t entrySize,
		const uint8_t* const shaderHandleStorage)
	{
		const auto handleSize = rayTracingProperties.ShaderGroupHandleSize();

		uint8_t* pDst = dst;

		for (const auto& entry : entries)
		{
			// Copy the shader identifier that was previously obtained with vkGetRayTracingShaderGroupHandlesKHR.
			std::memcpy(pDst, shaderHandleStorage + entry.GroupIndex * handleSize, handleSize);
			std::memcpy(pDst + handleSize, entry.InlineData.data(), entry.InlineData.size());

			pDst += entrySize;
		}

		return entries.size() * entrySize;
	}

	XVKShaderBindingManager::XVKShaderBindingManager(
		const XVKRayFuncManager& deviceFunc,
		const XVKRayTracingPipeline& rayTracingPipeline,
		const XVKRayTracingContext& rayTracingContext,
		const std::vector<Entry>& rayGenPrograms,
		const std::vector<Entry>& rayMissPrograms,
		const std::vector<Entry>& rayHitPrograms)
		:rayGenEntrySize(GetEntrySize(rayTracingContext, rayGenPrograms)),
		rayMissEntrySize(GetEntrySize(rayTracingContext, rayMissPrograms)),
		rayHitGroupEntrySize(GetEntrySize(rayTracingContext, rayHitPrograms)),
		rayGenOffset(0),
		rayMissOffset(rayGenPrograms.size() * rayGenEntrySize),
		rayHitGroupOffset(rayMissOffset + rayMissPrograms.size() * rayMissEntrySize),
		rayGenSize(rayGenPrograms.size() * rayGenEntrySize),
		rayMissSize(rayMissPrograms.size() * rayMissEntrySize),
		rayHitGroupSize(rayHitPrograms.size() * rayHitGroupEntrySize)
	{
		const size_t totalSize = rayGenPrograms.size() * rayGenEntrySize +
			rayMissPrograms.size() * rayMissEntrySize +
			rayHitPrograms.size() * rayHitGroupEntrySize;

		const auto& device = rayTracingContext.GetDevice();

		buffer.reset(new XVKBuffer(device, totalSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR));
		bufferMemory.reset(new XVKDeviceMemory(buffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)));
	
		//generate table
		const uint32_t handleSize = rayTracingContext.ShaderGroupHandleSize();
		const size_t groupCount = rayGenPrograms.size() + rayMissPrograms.size() + rayHitPrograms.size();
		
		std::vector<uint8_t> shaderHandleStorage(groupCount * handleSize);

		deviceFunc.vkGetRayTracingShaderGroupHandlesKHR(
			device.Handle(),
			rayTracingPipeline.Handle(),
			0,
			groupCount,
			shaderHandleStorage.size(),
			shaderHandleStorage.data());

		auto* pData = static_cast<uint8_t*>(bufferMemory->MapMemory(0, totalSize));

		pData += CopyShaderData(pData, rayTracingContext, rayGenPrograms, rayGenEntrySize, shaderHandleStorage.data());
		pData += CopyShaderData(pData, rayTracingContext, rayMissPrograms, rayMissEntrySize, shaderHandleStorage.data());
		CopyShaderData(pData, rayTracingContext, rayHitPrograms, rayHitGroupEntrySize, shaderHandleStorage.data());

		bufferMemory->UnmapMemory();
	}

	XVKShaderBindingManager::XVKShaderBindingManager(
		const XVKRayFuncManager& deviceFunc,
		VkPipeline rayTracingPipeline,
		const XVKRayTracingContext& rayTracingContext,
		const std::vector<Entry>& rayGenPrograms,
		const std::vector<Entry>& rayMissPrograms,
		const std::vector<Entry>& rayHitPrograms)
		:rayGenEntrySize(GetEntrySize(rayTracingContext, rayGenPrograms)),
		rayMissEntrySize(GetEntrySize(rayTracingContext, rayMissPrograms)),
		rayHitGroupEntrySize(GetEntrySize(rayTracingContext, rayHitPrograms)),
		rayGenOffset(0),
		rayMissOffset(rayGenPrograms.size()* rayGenEntrySize),
		rayHitGroupOffset(rayMissOffset + rayMissPrograms.size() * rayMissEntrySize),
		rayGenSize(rayGenPrograms.size()* rayGenEntrySize),
		rayMissSize(rayMissPrograms.size()* rayMissEntrySize),
		rayHitGroupSize(rayHitPrograms.size()* rayHitGroupEntrySize)
	{
		const size_t totalSize = rayGenPrograms.size() * rayGenEntrySize +
			rayMissPrograms.size() * rayMissEntrySize +
			rayHitPrograms.size() * rayHitGroupEntrySize;

		const auto& device = rayTracingContext.GetDevice();

		buffer.reset(new XVKBuffer(device, totalSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR));
		bufferMemory.reset(new XVKDeviceMemory(buffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)));

		//generate table
		const uint32_t handleSize = rayTracingContext.ShaderGroupHandleSize();
		const size_t groupCount = rayGenPrograms.size() + rayMissPrograms.size() + rayHitPrograms.size();

		std::vector<uint8_t> shaderHandleStorage(groupCount * handleSize);

		deviceFunc.vkGetRayTracingShaderGroupHandlesKHR(
			device.Handle(),
			rayTracingPipeline,
			0,
			groupCount,
			shaderHandleStorage.size(),
			shaderHandleStorage.data());

		auto* pData = static_cast<uint8_t*>(bufferMemory->MapMemory(0, totalSize));

		pData += CopyShaderData(pData, rayTracingContext, rayGenPrograms, rayGenEntrySize, shaderHandleStorage.data());
		pData += CopyShaderData(pData, rayTracingContext, rayMissPrograms, rayMissEntrySize, shaderHandleStorage.data());
		CopyShaderData(pData, rayTracingContext, rayHitPrograms, rayHitGroupEntrySize, shaderHandleStorage.data());

		bufferMemory->UnmapMemory();
	}

	XVKShaderBindingManager::~XVKShaderBindingManager()
	{
		buffer.reset();
		bufferMemory.reset();
	}
}
