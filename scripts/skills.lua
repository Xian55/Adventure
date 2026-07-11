-- Skill-tree data: per-rank point costs + effect magnitudes. Hot-reloadable (F5).
-- The tree shape (which node does what, prerequisites) lives in C++; these are the tunable numbers.
skills = {
	costs = {
		toughness   = { 1, 2, 4 },
		power       = { 2, 4 },
		adrenaline  = { 6 },
		endurance   = { 1, 3 },
		lockpicking = { 4 },
	},
	effects = {
		healthPerRank = 20.0, -- Toughness: +max HP per rank
		damagePerRank = 0.15, -- Power: +melee damage fraction per rank
		rageBonus     = 0.5,  -- Adrenaline: +rage-build fraction
		speedPerRank  = 0.08, -- Endurance: +move speed fraction per rank
	},
}
