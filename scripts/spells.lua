-- Spell data (numbers). Hot-reloadable (F5). The spell set, schools, and cast kinds live in C++;
-- these are the tunable numbers (mana / power / range).
spells = {
	telekinesis = { mana = 25.0, power = 18.0, range = 6.0 }, -- Force: shove enemies (into hazards)
	fireball    = { mana = 20.0, power = 30.0, range = 0.0 }, -- Fire projectile: burns on hit
	frostbolt   = { mana = 18.0, power = 22.0, range = 0.0 }, -- Frost projectile: slows on hit
	lightning   = { mana = 30.0, power = 45.0, range = 0.0 }, -- Lightning projectile: fast + hard
	quake       = { mana = 35.0, power = 35.0, range = 3.0 }, -- Earth melee cone: damage + knockback
	heal        = { mana = 40.0, power = 40.0, range = 0.0 }, -- Holy: restore health
}
