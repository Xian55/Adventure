-- Melee weapon: sword. Dark Messiah-style: hold to wind up, release to strike; direction from WASD.
-- Timings in seconds; tuned for feel (hot-reload with F5).
sword = {
	active           = 0.09,
	recovery         = 0.22,
	reach            = 1.9,  -- hitbox forward reach (engine units)
	arc              = 1.4,  -- radians the hitbox spans
	damage           = 25.0,
	knockback        = 6.0,
	chargeMax        = 0.5,  -- seconds of hold for a full charge
	chargeDamageMul  = 0.6,  -- extra damage fraction at full charge
}
