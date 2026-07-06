#pragma once
#include "raylib.h"

namespace adventure
{
	// Low-res render pipeline: scene -> low-res RT -> post (palette+dither) -> point-upscale.
	class Renderer
	{
	public:
		void init(int winW, int winH, int lowW, int lowH);
		void shutdown();

		void beginScene(Color clear); // BeginTextureMode(sceneRT) + clear
		void endScene();              // EndTextureMode
		void postProcess();           // scene RT -> post RT (palette+dither), low-res
		void blit();                  // post RT -> window, upscaled; call inside BeginDrawing()

		int lowW() const { return m_lowW; }
		int lowH() const { return m_lowH; }

	private:
		int m_winW = 0, m_winH = 0, m_lowW = 0, m_lowH = 0;
		RenderTexture2D m_scene{};
		RenderTexture2D m_post{};
		Shader m_postShader{};
		int m_locLowRes = -1;
		int m_locLevels = -1;
		bool m_ready = false;
	};
} // namespace adventure
