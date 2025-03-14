#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkAsset
{
	struct alignas(16) AMaterial
	{
		static AMaterial Lambertian(const glm::vec3& diffuse, const int32_t textureId = -1)
		{
			return AMaterial{ glm::vec4(diffuse, 1), textureId, 0.0f, 0.0f, Enum::Lambertian };
		}

		static AMaterial Metallic(const glm::vec3& diffuse, const float fuzziness, const int32_t textureId = -1)
		{
			return AMaterial{ glm::vec4(diffuse, 1), textureId, fuzziness, 0.0f, Enum::Metallic };
		}

		static AMaterial Dielectric(const float refractionIndex, const int32_t textureId = -1)
		{
			return AMaterial{ glm::vec4(0.7f, 0.7f, 1.0f, 1), textureId,  0.0f, refractionIndex, Enum::Dielectric };
		}

		static AMaterial Isotropic(const glm::vec3& diffuse, const int32_t textureId = -1)
		{
			return AMaterial{ glm::vec4(diffuse, 1), textureId, 0.0f, 0.0f, Enum::Isotropic };
		}

		static AMaterial DiffuseLight(const glm::vec3& diffuse, const int32_t textureId = -1)
		{
			return AMaterial{ glm::vec4(diffuse, 1), textureId, 0.0f, 0.0f, Enum::DiffuseLight };
		}

		enum class Enum : uint32_t
		{
			Lambertian = 0,
			Metallic = 1,
			Dielectric = 2,
			Isotropic = 3,
			DiffuseLight = 4
		};

		// Note: vec3 and vec4 gets aligned on 16 bytes in Vulkan shaders. 

		// Base material
		glm::vec4 baseColor;
		int32_t baseColorTextureIndex;

		// Metal fuzziness
		float fuzziness;

		// Dielectric refraction index
		float refractionIndex;

		// Which material are we dealing with
		Enum materialModel;
	};
}