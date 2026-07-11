#include "render/Billboard.h"

#include <algorithm>

namespace adventure
{
	namespace
	{
		float distSq(Vector3 a, Vector3 b)
		{
			const float dx = a.x - b.x;
			const float dy = a.y - b.y;
			const float dz = a.z - b.z;
			return dx * dx + dy * dy + dz * dz;
		}

		// A tiny bone-white skeleton on a transparent field — placeholder until real art lands in
		// assets/sprites/skeleton.png. Point-filtered + fogged by the scene, so it reads suitably crunchy.
		Image genPlaceholderSkeleton()
		{
			const Color bone = Color{230, 226, 208, 255};
			const Color dark = Color{35, 32, 30, 255};
			Image img = GenImageColor(32, 48, BLANK);

			ImageDrawRectangle(&img, 11, 3, 10, 8, bone);  // skull
			ImageDrawRectangle(&img, 13, 6, 2, 2, dark);   // eye
			ImageDrawRectangle(&img, 17, 6, 2, 2, dark);   // eye
			ImageDrawRectangle(&img, 15, 8, 2, 2, dark);   // nose
			ImageDrawRectangle(&img, 12, 11, 8, 2, bone);  // jaw
			ImageDrawRectangle(&img, 15, 13, 2, 13, bone); // spine
			ImageDrawRectangle(&img, 10, 15, 12, 1, bone); // ribs
			ImageDrawRectangle(&img, 10, 17, 12, 1, bone);
			ImageDrawRectangle(&img, 11, 19, 10, 1, bone);
			ImageDrawRectangle(&img, 12, 21, 8, 1, bone);
			ImageDrawRectangle(&img, 8, 14, 2, 7, bone);   // left upper arm
			ImageDrawRectangle(&img, 22, 14, 2, 7, bone);  // right upper arm
			ImageDrawRectangle(&img, 7, 21, 2, 7, bone);   // left forearm
			ImageDrawRectangle(&img, 23, 21, 2, 7, bone);  // right forearm
			ImageDrawRectangle(&img, 12, 26, 8, 3, bone);  // pelvis
			ImageDrawRectangle(&img, 13, 29, 2, 15, bone); // left leg
			ImageDrawRectangle(&img, 17, 29, 2, 15, bone); // right leg
			ImageDrawRectangle(&img, 10, 44, 5, 3, bone);  // left foot (to the texture bottom -> no gap)
			ImageDrawRectangle(&img, 17, 44, 5, 3, bone);  // right foot
			return img;
		}

		Color tintFor(EnemyState s)
		{
			switch (s)
			{
			case EnemyState::Windup:
				return Color{255, 225, 95, 255}; // telegraph
			case EnemyState::Stagger:
				return Color{235, 120, 110, 255}; // hit
			case EnemyState::Recover:
				return Color{230, 205, 180, 255};
			case EnemyState::Dead:
				return Color{120, 100, 95, 255};
			case EnemyState::Approach:
			default:
				return Color{225, 222, 205, 255}; // bone
			}
		}
	} // namespace

	void depthSortEnemies(std::vector<int>& order, const std::vector<Enemy>& enemies, Vector3 camPos)
	{
		order.clear();
		for (int i = 0; i < (int)enemies.size(); ++i)
		{
			const Enemy& e = enemies[i];
			if (e.active && e.render == RenderKind::Billboard)
				order.push_back(i);
		}
		std::sort(order.begin(), order.end(), [&](int a, int b) {
			return distSq(enemies[a].position, camPos) > distSq(enemies[b].position, camPos); // far first
		});
	}

	void EnemyBillboards::init()
	{
		const char* path = "assets/sprites/skeleton.png";
		if (FileExists(path))
			m_sprite = LoadTexture(path);
		if (m_sprite.id == 0)
		{
			Image img = genPlaceholderSkeleton();
			m_sprite = LoadTextureFromImage(img);
			UnloadImage(img);
		}
		SetTextureFilter(m_sprite, TEXTURE_FILTER_POINT);
		m_ready = true;
	}

	void EnemyBillboards::shutdown()
	{
		if (!m_ready)
			return;
		UnloadTexture(m_sprite);
		m_ready = false;
	}

	void EnemyBillboards::draw(Camera3D cam, const std::vector<Enemy>& enemies) const
	{
		if (!m_ready)
			return;

		std::vector<int> order;
		depthSortEnemies(order, enemies, cam.position);

		const Rectangle src = {0.0f, 0.0f, (float)m_sprite.width, (float)m_sprite.height};
		const float aspect = (float)m_sprite.width / (float)m_sprite.height;

		for (int i : order)
		{
			const Enemy& e = enemies[i];
			const float h = e.state == EnemyState::Dead ? e.height * 0.45f : e.height; // collapse the corpse
			const Vector2 size = {h * aspect, h};
			// raylib anchors the billboard at `position` as its bottom-left corner (extends +right, +up), so
			// place it at the feet and shift the origin half a width to stand the sprite centered on the enemy.
			const Vector3 feet = {e.position.x, e.position.y - e.height * 0.5f, e.position.z};
			// Contact shadow: a flat dark disc on the floor so the sprite reads as grounded (billboards cast none).
			const float sr = e.radius * 1.3f;
			DrawCylinderEx(Vector3{feet.x, feet.y + 0.02f, feet.z}, Vector3{feet.x, feet.y + 0.03f, feet.z}, sr, sr, 14, Color{0, 0, 0, 90});
			DrawBillboardPro(cam, m_sprite, src, feet, Vector3{0, 1, 0}, size, Vector2{size.x * 0.5f, 0.0f}, 0.0f, tintFor(e.state));
		}
	}
} // namespace adventure
