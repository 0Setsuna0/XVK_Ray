#pragma once

namespace editor
{
	struct UserSettings
	{
		//asset
		int sceneIndex;

		//renderer
		bool enableRayTracing;
		bool enableRayAccumulation;
		uint32_t spp;
		uint32_t numberOfBounces;
		uint32_t maxSpp;

		//camera
		float fov;

		inline const static float fovMin = 10.0f;
		inline const static float fovMax = 100.0f;
	};
}

