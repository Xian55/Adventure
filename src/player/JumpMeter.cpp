#include "player/JumpMeter.h"

#include <cmath>

namespace adventure
{
	float JumpMeter::horizontalSpeed(const Player& p)
	{
		return std::sqrt(p.velocity.x * p.velocity.x + p.velocity.z * p.velocity.z);
	}

	void JumpMeter::update(const Player& p, float dt)
	{
		const float hs = horizontalSpeed(p);

		if (!p.onGround)
		{
			if (!m_airborne) // takeoff
			{
				m_airborne = true;
				m_launch = p.position;
				m_apex = 0.0f;
				m_airtime = 0.0f;
				m_peakSpeed = hs;
			}
			m_airtime += dt;
			m_apex = std::fmax(m_apex, p.position.y - m_launch.y);
			m_peakSpeed = std::fmax(m_peakSpeed, hs);
		}
		else if (m_airborne) // landing
		{
			m_airborne = false;
			const float dx = p.position.x - m_launch.x;
			const float dz = p.position.z - m_launch.z;
			m_last.distance = std::sqrt(dx * dx + dz * dz);
			m_last.height = m_apex;
			m_last.airtime = m_airtime;
			m_last.peakSpeed = m_peakSpeed;
			m_maxDist = std::fmax(m_maxDist, m_last.distance);
			m_maxHeight = std::fmax(m_maxHeight, m_last.height);
		}
	}
} // namespace adventure
