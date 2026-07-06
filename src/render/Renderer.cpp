#include "render/Renderer.h"
#include "render/Shaders.h"

using namespace adventure;

void Renderer::init(int winW, int winH, int lowW, int lowH)
{
	m_winW = winW; m_winH = winH; m_lowW = lowW; m_lowH = lowH;

	m_scene = LoadRenderTexture(lowW, lowH);
	SetTextureFilter(m_scene.texture, TEXTURE_FILTER_POINT);
	m_post = LoadRenderTexture(lowW, lowH);
	SetTextureFilter(m_post.texture, TEXTURE_FILTER_POINT);

	m_postShader = LoadShaderFromMemory(nullptr, shaders::kPostFS);
	m_locLowRes  = GetShaderLocation(m_postShader, "uLowRes");
	m_locLevels  = GetShaderLocation(m_postShader, "uLevels");
	m_ready = true;
}

void Renderer::shutdown()
{
	if (!m_ready) return;
	UnloadShader(m_postShader);
	UnloadRenderTexture(m_scene);
	UnloadRenderTexture(m_post);
	m_ready = false;
}

void Renderer::beginScene(Color clear)
{
	BeginTextureMode(m_scene);
	ClearBackground(clear);
}

void Renderer::endScene()
{
	EndTextureMode();
}

// RenderTexture source rects use a negative height to correct the GL bottom-up flip.
// scene -> post keeps orientation (one flip); post -> window flips once more to land upright.
void Renderer::postProcess()
{
	const Rectangle lowSrc = { 0.0f, 0.0f, (float)m_lowW, -(float)m_lowH };

	float lowRes[2] = { (float)m_lowW, (float)m_lowH };
	float levels    = 24.0f;
	SetShaderValue(m_postShader, m_locLowRes, lowRes, SHADER_UNIFORM_VEC2);
	SetShaderValue(m_postShader, m_locLevels, &levels, SHADER_UNIFORM_FLOAT);

	// Apply the post shader at low resolution so the dither stays coarse.
	BeginTextureMode(m_post);
		ClearBackground(BLACK);
		BeginShaderMode(m_postShader);
			DrawTexturePro(m_scene.texture, lowSrc,
				Rectangle{ 0, 0, (float)m_lowW, (float)m_lowH }, Vector2{ 0, 0 }, 0.0f, WHITE);
		EndShaderMode();
	EndTextureMode();
}

void Renderer::blit()
{
	// Point-filter upscale to fill the real framebuffer (GetRenderWidth/Height account for
	// display DPI scaling; the logical window size would leave black borders).
	const Rectangle lowSrc = { 0.0f, 0.0f, (float)m_lowW, -(float)m_lowH };
	const float dstW = (float)GetRenderWidth();
	const float dstH = (float)GetRenderHeight();
	DrawTexturePro(m_post.texture, lowSrc,
		Rectangle{ 0, 0, dstW, dstH }, Vector2{ 0, 0 }, 0.0f, WHITE);
}
