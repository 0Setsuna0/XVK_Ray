#include "UniformBuffer.h"
#include "Vulkan/XVKBuffer.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKDeviceMemory.h"
namespace vkAsset
{

	AUniformBuffer::AUniformBuffer(const xvk::XVKDevice& device)
	{
		const auto bufferSize = sizeof(UniformBufferObject);
		buffer.reset(new xvk::XVKBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
		memory.reset(new xvk::XVKDeviceMemory(buffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)));
	}

	AUniformBuffer::AUniformBuffer(AUniformBuffer&& other) noexcept
		:buffer(other.buffer.release()), memory(other.memory.release())
	{

	}

	AUniformBuffer::~AUniformBuffer()
	{
		buffer.reset();
		memory.reset();
	}

	void AUniformBuffer::SetValue(const UniformBufferObject& ubo)
	{
		void* data = memory->MapMemory(0, sizeof(UniformBufferObject));
		std::memcpy(data, &ubo, sizeof(ubo));
		memory->UnmapMemory();
	}
}