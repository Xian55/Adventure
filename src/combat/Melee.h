#pragma once

// Melee swing state machine + weapon data. Pure logic (no raylib) -> headless-testable. See src/combat.
namespace adventure
{
	// Weapon numbers (loaded from Lua, e.g. scripts/weapons/sword.lua). Timings in seconds.
	struct WeaponDef
	{
		float windup = 0.12f;
		float active = 0.09f;
		float recovery = 0.22f;
		float reach = 1.9f; // hitbox forward distance (engine units)
		float arc = 1.4f;   // radians the hitbox sweeps across Active
		float damage = 25.0f;
		float knockback = 6.0f;
	};

	enum class MeleePhase
	{
		Idle,
		Windup,
		Active,
		Recovery
	};

	struct MeleeState
	{
		MeleePhase phase = MeleePhase::Idle;
		float timer = 0.0f;
		int comboStep = 0;
		bool wantSwing = false;    // buffered input (set on attack press, consumed by the state machine)
		bool hitThisSwing = false; // combat resolution debounce; cleared at each swing start
	};

	// Buffer an attack input (consumed in Idle, or as a combo chain during Recovery).
	void requestSwing(MeleeState& s);

	// Advance one step: Idle -> Windup -> Active -> Recovery -> Idle, with Recovery->Windup combo chaining.
	void updateMelee(MeleeState& s, const WeaponDef& def, float dt);

	// The sweeping hitbox is live only during Active.
	bool hitboxActive(const MeleeState& s);

	// 0..1 progress through the current phase (for the hitbox sweep + viewmodel animation).
	float phaseProgress(const MeleeState& s, const WeaponDef& def);
} // namespace adventure
