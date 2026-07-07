#pragma once

namespace adventure
{
	// First-person viewmodel: torch (left) + sword (right), drawn over the world with depth-test off,
	// inside the low-res scene RT so it pixelates. Placeholder procedural geometry until sprites/models
	// exist. bobPhase advances with movement; bobAmount scales the sway; flickerTime drives the flame.
	// meleePhase (0=Idle,1=Charge,2=Active,3=Recovery) + meleeProgress (0..1) + swingDir
	// (0=Neutral,1=Left,2=Right,3=Forward,4=Overhead) + charge (0..1) drive the directional sword swing.
	void drawViewmodel(float bobPhase, float bobAmount, float flickerTime, int meleePhase, float meleeProgress, int swingDir, float charge);
} // namespace adventure
