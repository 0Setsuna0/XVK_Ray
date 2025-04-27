#include "Vulkan/XVKCommon.h"
#include "glm/glm.hpp"

#define ALIGN16 alignas(16)

namespace xvk::ray
{
	struct ALIGN16 ReSTIRGISample
	{
		glm::vec3 x_view;    float pad0 = 0.0f; // 16 bytes
		glm::vec3 n_view;    float pad1 = 0.0f; // 16 bytes
		glm::vec3 x_sample;  float pad2 = 0.0f; // 16 bytes
		glm::vec3 n_sample;  float pad3 = 0.0f; // 16 bytes
		glm::vec3 L_out;     float pad4 = 0.0f; // 16 bytes
		glm::vec3 f;         float pad5 = 0.0f; // 16 bytes

		uint32_t materialID; float p_q;
		float pad6[2] = { 0.0f, 0.0f };         // -> 16 bytes

		// Total size: 7 * 16 = 112 bytes
	};

	struct ALIGN16 ReSTIRGIReservoir
	{
		float w_sum;
		float W;
		uint32_t m;
		uint32_t pad;        // align to 16 bytes

		ReSTIRGISample s;    // 112 bytes

		// Total: 16 + 112 = 128 bytes
	};
}

