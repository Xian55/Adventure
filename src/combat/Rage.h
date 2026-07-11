#pragma once

// Rage → berserk meter (Dark Messiah combat resource). Landed melee builds rage; maxed out flips the player
// into a temporary berserk (bonus damage + faster swings). Pure logic (no raylib) -> headless-testable.
namespace adventure
{
	struct RageTuning
	{
		float max = 100.0f;           // full meter
		float gainPerHit = 16.0f;     // rage per enemy a swing connects with
		float gainPerKill = 30.0f;    // extra rage when that hit kills
		float decayPerSec = 12.0f;    // bleeds off once you stop fighting
		float decayDelay = 2.5f;      // grace after the last hit before decay starts
		float berserkDuration = 6.0f; // how long a berserk lasts (meter drains across it)
		float damageMul = 1.6f;       // melee damage multiplier while berserk
		float speedMul = 1.5f;        // swing-speed multiplier while berserk
	};

	struct RageState
	{
		float rage = 0.0f;         // current meter, 0..max
		float sinceHit = 0.0f;     // seconds since the last landed hit (gates decay)
		bool berserk = false;      // in the berserk window
		float berserkTimer = 0.0f; // time left in the berserk window
	};

	// Feed the meter from a resolved swing (hits/kills this frame). Reaching max triggers berserk. No-op while
	// already berserk (you can't stack it) and when nothing connected.
	void addRage(RageState& s, const RageTuning& t, int hits, int kills);

	// One fixed step: run the berserk timer (draining the meter across it) or bleed rage after the grace delay.
	void updateRage(RageState& s, const RageTuning& t, float dt);

	float rageFraction(const RageState& s, const RageTuning& t);  // 0..1 for the HUD meter
	float rageDamageMul(const RageState& s, const RageTuning& t); // damage multiplier (1.0 unless berserk)
	float rageSpeedMul(const RageState& s, const RageTuning& t);  // swing-speed multiplier (1.0 unless berserk)
} // namespace adventure
