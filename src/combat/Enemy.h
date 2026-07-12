#pragma once
#include "raylib.h"

namespace adventure
{
	enum class EnemyState
	{
		Approach,
		Windup,  // committed telegraph before a strike (immobile; interruptible by stagger)
		Recover, // brief cooldown after swinging
		Stagger,
		Dead
	};

	// How an enemy is drawn — the swap seam: billboard sprite now, animated 3D model later (render-only change).
	enum class RenderKind
	{
		Box,       // debug placeholder cube
		Billboard, // Y-facing sprite (Doom look)
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
		float burn = 0.0f; // fire DoT time remaining
		float slow = 0.0f; // frost slow time remaining
		RenderKind render = RenderKind::Billboard;
		bool active = true;  // false once despawned
		bool scored = false; // skill point already awarded for this kill
	};
} // namespace adventure
