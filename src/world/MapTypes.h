#pragma once
#include "raylib.h"

#include <map>
#include <string>
#include <vector>

// Data types for TrenchBroom/Quake .map loading. See src/world/CLAUDE.md.
namespace adventure::world
{
	// One brush face: 3 plane-defining points (raw map space, Z-up) + Valve 220 texture projection.
	struct Face
	{
		Vector3 p[3];
		std::string texture;
		Vector4 uAxis; // xyz = axis, w = offset (texels)
		Vector4 vAxis;
		float rotation = 0.0f;
		float scaleX = 1.0f;
		float scaleY = 1.0f;
	};

	struct Brush
	{
		std::vector<Face> faces;
	};

	struct Entity
	{
		std::string classname;
		std::map<std::string, std::string> keys;
		std::vector<Brush> brushes;

		bool has(const std::string& k) const { return keys.count(k) != 0; }
		std::string str(const std::string& k, const std::string& def = "") const;
		float number(const std::string& k, float def = 0.0f) const;
		Vector3 vec3(const std::string& k, Vector3 def = {0, 0, 0}) const; // raw map space
	};

	struct MapData
	{
		std::vector<Entity> entities;
		const Entity* first(const std::string& classname) const;
	};

	// Render geometry for one texture, engine space (Y-up). Parallel arrays, not interleaved.
	struct MeshData
	{
		std::string texture;
		std::vector<float> positions;      // 3 per vertex
		std::vector<float> normals;        // 3 per vertex
		std::vector<float> uvs;            // 2 per vertex (texel space; /texSize at upload)
		std::vector<unsigned char> colors; // 4 per vertex (baked vertex light, RGBA)
		std::vector<unsigned short> indices;
		int vertexCount() const { return (int)(positions.size() / 3); }
	};

	// Convex collision volume: outward face planes (xyz = unit normal, w = d) + AABB, engine space.
	struct CollisionBrush
	{
		std::vector<Vector4> planes;
		Vector3 min;
		Vector3 max;
	};

	struct WorldGeometry
	{
		std::vector<MeshData> meshes;          // grouped by texture
		std::vector<CollisionBrush> collision; // one per solid brush
	};

	// Map space (Z-up) -> engine space (Y-up) with map scale. For placing entities read from a map.
	Vector3 mapToEngine(Vector3 mapPoint);
} // namespace adventure::world
