#pragma once
#include "raylib.h"
#include "world/MapTypes.h"

#include <string>
#include <vector>

namespace adventure
{
	// Uploads world::WorldGeometry to the GPU and draws it with the fog + vertex-light shader.
	class WorldRenderer
	{
	public:
		void load(const world::WorldGeometry& geo);
		void draw(Vector3 cameraPos) const; // call inside BeginMode3D
		void unload();
		void setFog(Color color, float density);

	private:
		struct Batch
		{
			Mesh mesh;
			Material material;
		};
		Texture2D loadTextureFor(const std::string& name);

		std::vector<Batch> m_batches;
		std::vector<Texture2D> m_ownedTextures;
		Texture2D m_default{};
		Shader m_shader{};
		int m_locCameraPos = -1;
		int m_locFogColor = -1;
		int m_locFogDensity = -1;
		Color m_fogColor{26, 28, 40, 255};
		float m_fogDensity = 0.15f;
		bool m_ready = false;
	};
} // namespace adventure
