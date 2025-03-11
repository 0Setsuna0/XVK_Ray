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
		int spp;
		int numberOfBounces;

		//camera
		float fov;

		inline const static float fovMin = 10.0f;
		inline const static float fovMax = 100.0f;
	};
}

