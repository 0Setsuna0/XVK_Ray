#include "Vulkan/XVKCommon.h"
#include "glm/glm.hpp"

namespace xvk::ray
{
	struct ReSTIRGISample
	{
		glm::vec3 x_view;
		glm::vec3 n_view;
		glm::vec3 x_sample;
		glm::vec3 n_sample;
		glm::vec3 L_out;
		glm::vec3 f;
		uint32_t materialID;
		float p_q;
	};

	struct ReSTIRGIReservoir
	{
		float w_sum;
		float W;
		uint32_t m;
		uint32_t pad;
		ReSTIRGISample s;
	};
}

