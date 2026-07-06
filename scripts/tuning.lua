-- Movement / combat tuning. Hot-reloadable — C++ reads these values so feel can be
-- iterated without recompiling. M0: proves script data loads under the sandbox.
tuning = {
	-- movement (Quake accel model, used from M1)
	moveSpeed = 8.0,
	accel     = 10.0,
	airAccel  = 1.2,
	friction  = 6.0,
	stopSpeed = 2.0,
	gravity   = 22.0,
	jumpSpeed = 8.0,

	-- combat (used from M2)
	kickReach   = 1.6,
	kickImpulse = 14.0,
}

print(string.format("tuning loaded: moveSpeed=%.1f accel=%.1f gravity=%.1f",
	tuning.moveSpeed, tuning.accel, tuning.gravity))

if host then host.log("host bridge reachable from sandbox") end
