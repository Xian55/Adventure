#pragma once
#include "raylib.h"
#include "combat/Enemy.h"

#include <vector>

// Y-facing enemy sprites (Doom-style upright billboards). The RenderKind seam: this replaces the debug
// cubes; a 3D-model path can slot in later without touching gameplay.
namespace adventure
{
	// Back-to-front draw order (farthest enemy first) so nearer sprites blend over them. Fills `order` with
	// the indices of active, Billboard-kind enemies, sorted by distance to the camera descending. Pure.
	void depthSortEnemies(std::vector<int>& order, const std::vector<Enemy>& enemies, Vector3 camPos);

	// Owns the skeleton sprite texture. Real art (assets/sprites/skeleton.png) is used if present, else a
	// generated placeholder — assets are local/optional, same as WorldRenderer's checker fallback.
	class EnemyBillboards
	{
	public:
		void init();     // needs a GL context (call after the window/renderer is up)
		void shutdown(); // frees the texture

		// Draw every Billboard-kind enemy, depth-sorted + state-tinted. Must run inside BeginMode3D(cam).
		void draw(Camera3D cam, const std::vector<Enemy>& enemies) const;

	private:
		Texture2D m_sprite{};
		bool m_ready = false;
	};
} // namespace adventure
