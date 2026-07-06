#pragma once

// Global compile-time configuration. Gameplay tuning that changes often lives in
// Lua (scripts/tuning.lua); this holds only structural constants.
namespace adventure::config
{
	constexpr int kWindowW = 1280;
	constexpr int kWindowH = 720;

	// Internal render resolution — deliberately low. The scene is rendered here then
	// point-filter upscaled to the window for the GRAVEN/boomer-shooter pixelation.
	constexpr int kLowW = 480;
	constexpr int kLowH = 270;

	// Fixed gameplay timestep (physics/combat run here; rendering runs per display frame).
	constexpr float kFixedDt = 1.0f / 60.0f;
} // namespace adventure::config
