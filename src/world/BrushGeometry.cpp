#include "world/BrushGeometry.h"
#include "core/Config.h"

#include "raymath.h"

#include <cmath>

namespace adventure::world
{
	namespace
	{
		constexpr float kBig = 1.0e5f; // seed-quad half-extent in map units
		constexpr float kEps = 1.0e-3f;

		float planeDist(Vector4 pl, Vector3 p)
		{
			return pl.x * p.x + pl.y * p.y + pl.z * p.z + pl.w;
		}

		Vector4 planeFromPoints(Vector3 a, Vector3 b, Vector3 c)
		{
			Vector3 n = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a)));
			return Vector4{n.x, n.y, n.z, -Vector3DotProduct(n, a)};
		}

		// Map space (Z-up) -> engine space (Y-up), with map scale applied.
		Vector3 toEngine(Vector3 m)
		{
			return Vector3{m.x * config::kMapScale, m.z * config::kMapScale, -m.y * config::kMapScale};
		}

		// Same axis swap for a direction (scale is uniform; caller normalizes).
		Vector3 toEngineNormal(Vector3 m)
		{
			return Vector3{m.x, m.z, -m.y};
		}

		// A large quad lying on the plane, used as the seed for clipping.
		std::vector<Vector3> seedPolygon(Vector4 pl)
		{
			Vector3 n = {pl.x, pl.y, pl.z};
			Vector3 center = Vector3Scale(n, -pl.w);
			Vector3 axis = (std::fabs(n.x) <= std::fabs(n.y) && std::fabs(n.x) <= std::fabs(n.z))
			                   ? Vector3{1, 0, 0}
			               : (std::fabs(n.y) <= std::fabs(n.z)) ? Vector3{0, 1, 0}
			                                                    : Vector3{0, 0, 1};
			Vector3 u = Vector3Scale(Vector3Normalize(Vector3CrossProduct(axis, n)), kBig);
			Vector3 v = Vector3Scale(Vector3CrossProduct(n, Vector3Normalize(u)), kBig);
			return {
			    Vector3Add(Vector3Add(center, u), v),
			    Vector3Add(Vector3Subtract(center, u), v),
			    Vector3Subtract(Vector3Subtract(center, u), v),
			    Vector3Subtract(Vector3Add(center, u), v),
			};
		}

		// Sutherland-Hodgman: keep the part of poly on the inside (dist <= eps) of the plane.
		std::vector<Vector3> clip(const std::vector<Vector3>& poly, Vector4 pl)
		{
			std::vector<Vector3> out;
			const int m = (int)poly.size();
			for (int i = 0; i < m; ++i)
			{
				Vector3 a = poly[i];
				Vector3 b = poly[(i + 1) % m];
				float da = planeDist(pl, a);
				float db = planeDist(pl, b);
				bool ina = da <= kEps;
				bool inb = db <= kEps;
				if (ina)
					out.push_back(a);
				if (ina != inb)
				{
					float t = da / (da - db);
					out.push_back(Vector3Lerp(a, b, t));
				}
			}
			return out;
		}

		// Fake directional light baked into vertex color so faces read in 3D before real lighting.
		unsigned char faceShade(Vector3 nEngine)
		{
			Vector3 key = Vector3Normalize(Vector3{0.35f, 1.0f, 0.25f});
			float l = 0.55f + 0.45f * fmaxf(0.0f, Vector3DotProduct(nEngine, key));
			return (unsigned char)(fminf(1.0f, l) * 255.0f);
		}

		MeshData& meshFor(WorldGeometry& w, const std::string& tex)
		{
			for (auto& m : w.meshes)
				if (m.texture == tex)
					return m;
			w.meshes.push_back(MeshData{});
			w.meshes.back().texture = tex;
			return w.meshes.back();
		}

		void buildBrush(const Brush& brush, WorldGeometry& world)
		{
			const int nf = (int)brush.faces.size();
			if (nf < 4)
				return;

			// Interior guess = average of all defining points; used to orient normals outward.
			Vector3 interior = {0, 0, 0};
			for (const auto& f : brush.faces)
				for (int k = 0; k < 3; ++k)
					interior = Vector3Add(interior, f.p[k]);
			interior = Vector3Scale(interior, 1.0f / (nf * 3));

			std::vector<Vector4> mapPlanes(nf);
			for (int i = 0; i < nf; ++i)
			{
				Vector4 pl = planeFromPoints(brush.faces[i].p[0], brush.faces[i].p[1], brush.faces[i].p[2]);
				if (planeDist(pl, interior) > 0.0f) // interior must be on the inside
					pl = Vector4{-pl.x, -pl.y, -pl.z, -pl.w};
				mapPlanes[i] = pl;
			}

			CollisionBrush cb;
			cb.min = Vector3{1e30f, 1e30f, 1e30f};
			cb.max = Vector3{-1e30f, -1e30f, -1e30f};
			bool anyVerts = false;

			for (int i = 0; i < nf; ++i)
			{
				std::vector<Vector3> poly = seedPolygon(mapPlanes[i]);
				for (int j = 0; j < nf && poly.size() >= 3; ++j)
					if (j != i)
						poly = clip(poly, mapPlanes[j]);
				if (poly.size() < 3)
					continue;

				const Face& face = brush.faces[i];
				Vector3 nMap = {mapPlanes[i].x, mapPlanes[i].y, mapPlanes[i].z};
				Vector3 nEngine = Vector3Normalize(toEngineNormal(nMap));
				unsigned char shade = faceShade(nEngine);

				MeshData& mesh = meshFor(world, face.texture);
				unsigned short base = (unsigned short)mesh.vertexCount();

				for (const Vector3& mp : poly)
				{
					Vector3 ep = toEngine(mp);
					mesh.positions.insert(mesh.positions.end(), {ep.x, ep.y, ep.z});
					mesh.normals.insert(mesh.normals.end(), {nEngine.x, nEngine.y, nEngine.z});
					float uu = Vector3DotProduct(mp, Vector3{face.uAxis.x, face.uAxis.y, face.uAxis.z}) /
					               (face.scaleX != 0 ? face.scaleX : 1.0f) +
					           face.uAxis.w;
					float vv = Vector3DotProduct(mp, Vector3{face.vAxis.x, face.vAxis.y, face.vAxis.z}) /
					               (face.scaleY != 0 ? face.scaleY : 1.0f) +
					           face.vAxis.w;
					mesh.uvs.insert(mesh.uvs.end(), {uu, vv});
					mesh.colors.insert(mesh.colors.end(), {shade, shade, shade, 255});

					cb.min = Vector3Min(cb.min, ep);
					cb.max = Vector3Max(cb.max, ep);
					anyVerts = true;
				}

				const int m = (int)poly.size();
				for (int k = 1; k < m - 1; ++k)
				{
					mesh.indices.push_back(base);
					mesh.indices.push_back((unsigned short)(base + k));
					mesh.indices.push_back((unsigned short)(base + k + 1));
				}

				// Engine-space collision plane, recomputed from converted points, oriented outward.
				Vector4 ePlane = planeFromPoints(toEngine(poly[0]), toEngine(poly[1]), toEngine(poly[2]));
				Vector3 iEngine = toEngine(interior);
				if (planeDist(ePlane, iEngine) > 0.0f)
					ePlane = Vector4{-ePlane.x, -ePlane.y, -ePlane.z, -ePlane.w};
				cb.planes.push_back(ePlane);
			}

			if (anyVerts && !cb.planes.empty())
				world.collision.push_back(std::move(cb));
		}
	} // namespace

	WorldGeometry buildWorld(const MapData& map)
	{
		WorldGeometry world;
		for (const auto& e : map.entities)
			for (const auto& b : e.brushes)
				buildBrush(b, world);
		return world;
	}
} // namespace adventure::world
