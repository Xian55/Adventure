#pragma once
#include "raylib.h"

namespace adventure
{
	enum class EnemyState
	{
		Approach,
		Stagger,
		Dead
	};

	// One melee enemy (skeleton). Rendered as a placeholder box until sprites exist (RenderKind seam).
	struct Enemy
	{
		Vector3 position{0, 0, 0}; // AABB center, engine space
		Vector3 velocity{0, 0, 0}; // knockback
		float yaw = 0.0f;
		float health = 50.0f;
		float maxHealth = 50.0f;
		EnemyState state = EnemyState::Approach;
		float stateTimer = 0.0f;
		float radius = 0.4f;
		float height = 1.8f;
		bool active = true; // false once despawned
	};
} // namespace adventure
