#pragma once

// Melee swing state machine (Dark Messiah–style directional attacks). Pure logic -> headless-testable.
// Hold attack to wind up (Charge), the movement key picks the direction, release to strike.
namespace adventure
{
	struct WeaponDef
	{
		float active = 0.09f;
		float recovery = 0.22f;
		float reach = 1.9f; // hitbox forward distance (engine units)
		float arc = 1.4f;   // radians the hitbox spans
		float damage = 25.0f;
		float knockback = 6.0f;
		float chargeMax = 0.5f;       // seconds of hold for a full charge
		float chargeDamageMul = 0.6f; // extra damage fraction at full charge (hold longer -> stronger)
	};

	// Chosen by the held movement key during the charge. Neutral (no key) alternates Left/Right per swing.
	enum class SwingDir
	{
		Neutral,
		Left,     // A
		Right,    // D
		Forward,  // W (thrust)
		Overhead, // S (top-down chop)
	};

	enum class MeleePhase
	{
		Idle,
		Charge, // holding the attack button (player-timed windup); direction selectable
		Active,
		Recovery
	};

	struct MeleeState
	{
		MeleePhase phase = MeleePhase::Idle;
		float timer = 0.0f;      // time in Active/Recovery
		float chargeTime = 0.0f; // time held in Charge (drives charge damage bonus)
		SwingDir dir = SwingDir::Neutral;
		SwingDir resolved = SwingDir::Neutral; // effective dir for the current swing (Neutral -> alternating)
		bool neutralLeft = true;               // side the next Neutral swing takes (alternates)
		bool hitThisSwing = false;             // hit-resolution debounce; cleared at swing start
	};

	void beginCharge(MeleeState& s);             // attack pressed: Idle -> Charge
	void setSwingDir(MeleeState& s, SwingDir d); // while charging, pick the direction
	void releaseSwing(MeleeState& s);            // attack released: Charge -> Active (resolves Neutral)
	void updateMelee(MeleeState& s, const WeaponDef& def, float dt);

	bool hitboxActive(const MeleeState& s);                          // Active only
	float phaseProgress(const MeleeState& s, const WeaponDef& def);  // 0..1 within Active/Recovery
	float chargeFraction(const MeleeState& s, const WeaponDef& def); // 0..1 charge, for damage + visuals
} // namespace adventure
