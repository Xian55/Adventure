#include "render/WorldRenderer.h"
#include "render/Shaders.h"

#include "raymath.h"

#include <cstring>

using namespace adventure;

namespace
{
	// world::MeshData (parallel arrays) -> raylib Mesh. Texel-space UVs are divided by the texture
	// size so they tile correctly under wrap-repeat.
	Mesh toMesh(const world::MeshData& md, float texW, float texH)
	{
		Mesh m = {0};
		m.vertexCount = md.vertexCount();
		m.triangleCount = (int)(md.indices.size() / 3);

		m.vertices = (float*)MemAlloc(sizeof(float) * 3 * m.vertexCount);
		std::memcpy(m.vertices, md.positions.data(), sizeof(float) * 3 * m.vertexCount);

		m.normals = (float*)MemAlloc(sizeof(float) * 3 * m.vertexCount);
		std::memcpy(m.normals, md.normals.data(), sizeof(float) * 3 * m.vertexCount);

		m.texcoords = (float*)MemAlloc(sizeof(float) * 2 * m.vertexCount);
		for (int i = 0; i < m.vertexCount; ++i)
		{
			m.texcoords[i * 2 + 0] = md.uvs[i * 2 + 0] / texW;
			m.texcoords[i * 2 + 1] = md.uvs[i * 2 + 1] / texH;
		}

		m.colors = (unsigned char*)MemAlloc(sizeof(unsigned char) * 4 * m.vertexCount);
		std::memcpy(m.colors, md.colors.data(), sizeof(unsigned char) * 4 * m.vertexCount);

		m.indices = (unsigned short*)MemAlloc(sizeof(unsigned short) * md.indices.size());
		std::memcpy(m.indices, md.indices.data(), sizeof(unsigned short) * md.indices.size());

		return m;
	}
} // namespace

void WorldRenderer::setFog(Color color, float density)
{
	m_fogColor = color;
	m_fogDensity = density;
}

Texture2D WorldRenderer::loadTextureFor(const std::string& name)
{
	std::string path = "assets/textures/" + name + ".png";
	if (FileExists(path.c_str()))
	{
		Texture2D t = LoadTexture(path.c_str());
		if (t.id != 0)
		{
			SetTextureFilter(t, TEXTURE_FILTER_POINT);
			SetTextureWrap(t, TEXTURE_WRAP_REPEAT);
			m_ownedTextures.push_back(t);
			return t;
		}
	}
	return m_default; // placeholder checker (assets are local/optional)
}

void WorldRenderer::load(const world::WorldGeometry& geo)
{
	Image chk = GenImageChecked(64, 64, 8, 8, Color{70, 72, 86, 255}, Color{54, 56, 68, 255});
	m_default = LoadTextureFromImage(chk);
	UnloadImage(chk);
	SetTextureFilter(m_default, TEXTURE_FILTER_POINT);
	SetTextureWrap(m_default, TEXTURE_WRAP_REPEAT);

	m_shader = LoadShaderFromMemory(shaders::kWorldVS, shaders::kWorldFS);
	m_shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(m_shader, "matModel");
	m_locCameraPos = GetShaderLocation(m_shader, "uCameraPos");
	m_locFogColor = GetShaderLocation(m_shader, "uFogColor");
	m_locFogDensity = GetShaderLocation(m_shader, "uFogDensity");

	for (const auto& md : geo.meshes)
	{
		Texture2D tex = loadTextureFor(md.texture);
		Mesh mesh = toMesh(md, (float)tex.width, (float)tex.height);
		UploadMesh(&mesh, false);
		Material mat = LoadMaterialDefault();
		mat.shader = m_shader;
		mat.maps[MATERIAL_MAP_DIFFUSE].texture = tex;
		m_batches.push_back({mesh, mat});
	}
	m_ready = true;
}

void WorldRenderer::draw(Vector3 cameraPos) const
{
	if (!m_ready)
		return;

	Vector3 cam = cameraPos;
	float fog[3] = {m_fogColor.r / 255.0f, m_fogColor.g / 255.0f, m_fogColor.b / 255.0f};
	float density = m_fogDensity;
	SetShaderValue(m_shader, m_locCameraPos, &cam, SHADER_UNIFORM_VEC3);
	SetShaderValue(m_shader, m_locFogColor, fog, SHADER_UNIFORM_VEC3);
	SetShaderValue(m_shader, m_locFogDensity, &density, SHADER_UNIFORM_FLOAT);

	for (const auto& b : m_batches)
		DrawMesh(b.mesh, b.material, MatrixIdentity());
}

void WorldRenderer::unload()
{
	if (!m_ready)
		return;
	for (auto& b : m_batches)
		UnloadMesh(b.mesh); // materials share one shader + shared textures; freed below
	m_batches.clear();
	for (auto& t : m_ownedTextures)
		UnloadTexture(t);
	m_ownedTextures.clear();
	UnloadTexture(m_default);
	UnloadShader(m_shader);
	m_ready = false;
}
