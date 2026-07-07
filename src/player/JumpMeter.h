#pragma once
#include "player/Player.h"

namespace adventure
{
	struct JumpStats
	{
		float distance = 0.0f;  // horizontal launch->land
		float height = 0.0f;    // apex above launch
		float airtime = 0.0f;   // seconds airborne
		float peakSpeed = 0.0f; // max horizontal speed in the air
	};

	// Measures jumps for tuning/level design: detects takeoff/landing from Player.onGround and records the
	// last jump + session maxima. Pure (no raylib) -> headless-testable. Feed it each fixed step.
	class JumpMeter
	{
	public:
		void update(const Player& p, float dt);

		const JumpStats& last() const { return m_last; }
		float maxDistance() const { return m_maxDist; }
		float maxHeight() const { return m_maxHeight; }
		bool airborne() const { return m_airborne; }

		static float horizontalSpeed(const Player& p);

	private:
		bool m_airborne = false;
		Vector3 m_launch{0, 0, 0};
		float m_apex = 0.0f;
		float m_airtime = 0.0f;
		float m_peakSpeed = 0.0f;
		JumpStats m_last{};
		float m_maxDist = 0.0f;
		float m_maxHeight = 0.0f;
	};
} // namespace adventure
