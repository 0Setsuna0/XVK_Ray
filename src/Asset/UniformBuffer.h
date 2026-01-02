#pragma once

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <memory>

namespace xvk
{
	class XVKBuffer;
	class XVKDevice;
	class XVKDeviceMemory;
}

namespace vkAsset
{
	class UniformBufferObject
	{
	public:
		glm::mat4 view;
		glm::mat4 viewPrev;
		glm::mat4 projection;
		glm::mat4 projectionPrev;
		glm::mat4 viewInverse;
		glm::mat4 projectionInverse;
		uint32_t spp;
		uint32_t totalNumberOfSamples;
		uint32_t numberOfBounces;
		uint32_t randomSeed;
		uint32_t hasSkyBox;
		uint32_t reuse;
		uint32_t currentFrame;
	};

	class AUniformBuffer
	{
	public:

		AUniformBuffer(const xvk::XVKDevice& device);
		AUniformBuffer(AUniformBuffer&& other) noexcept;
		~AUniformBuffer();

		const xvk::XVKBuffer& GetBuffer() const { return *buffer; }

		void SetValue(const UniformBufferObject& ubo);

	private:
		std::unique_ptr<xvk::XVKBuffer> buffer;
		std::unique_ptr<xvk::XVKDeviceMemory> memory;
	};
}


