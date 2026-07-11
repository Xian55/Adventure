-- Movement / combat / view feel. Hot-reloadable: C++ reads these so feel is tuned here, not in code.
tuning = {
	-- movement (Quake accel). moveSpeed = walk; hold Shift for sprintSpeed.
	moveSpeed   = 4.5,
	sprintSpeed = 8.0,
	accel       = 12.0,
	airAccel    = 1.0,   -- low air control -> committed jumps
	friction    = 9.0,   -- higher -> less slide on landing
	stopSpeed   = 3.0,
	gravity     = 25.0,  -- snappier jump arc
	jumpSpeed   = 7.5,
	stepHeight  = 0.5,   -- auto-climb stairs up to this (no jump needed)

	-- view feel
	bobFreq   = 9.0,   -- weapon/head bob cadence
	weaponBob = 0.02,  -- viewmodel sway amplitude
	headBob   = 0.035, -- camera bob amplitude

	-- combat (used from M2)
	kickReach   = 1.6,
	kickImpulse = 14.0,

	-- shield (hold RMB): a facing block absorbs blockReduction of the hit over the front cone blockArc (rad).
	blockArc       = 1.2,
	blockReduction = 0.8,

	-- enemy (skeleton): approach, telegraph (windup), then strike for attackDamage if still in reach.
	enemyMoveSpeed    = 3.0,
	enemyAttackRange  = 1.6,
	enemyAttackWindup = 0.5,
	enemyAttackRecover = 0.6,
	enemyAttackReach  = 1.9,
	enemyAttackDamage = 12.0,
	enemyStaggerTime  = 0.45,

	-- rage -> berserk: landed melee builds the meter; maxed flips to a timed berserk (drains as it runs).
	rageMax          = 100.0,
	rageGainPerHit   = 16.0,
	rageGainPerKill  = 30.0,
	rageDecayPerSec  = 12.0,
	rageDecayDelay   = 2.5,
	berserkDuration  = 6.0,
	berserkDamageMul = 1.6,
	berserkSpeedMul  = 1.5,
}

print(string.format("tuning loaded: walk=%.1f sprint=%.1f friction=%.1f gravity=%.1f",
	tuning.moveSpeed, tuning.sprintSpeed, tuning.friction, tuning.gravity))
