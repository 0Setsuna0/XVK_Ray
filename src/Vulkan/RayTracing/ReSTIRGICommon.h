#include "Vulkan/XVKCommon.h"
#include "glm/glm.hpp"

#define ALIGN16 alignas(16)

namespace xvk::ray
{
	struct ReSTIRGISample
	{
		glm::vec3 x_view;  
		uint32_t materialID;
		glm::vec3 n_view;   
		uint32_t randSeed;
		glm::vec3 x_sample;  
		float p_q;
		glm::vec3 n_sample;  
		uint32_t pad1;
		glm::vec3 L_out;
		uint32_t pad2;
		glm::vec3 f;
		uint32_t pad3;
	};

	struct ReSTIRGIReservoir
	{
		float w_sum;
		float W;
		uint32_t m;
		float pad;

		ReSTIRGISample s;
	};
}

