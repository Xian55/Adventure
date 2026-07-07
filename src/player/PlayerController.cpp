#include "player/PlayerController.h"

#include "raymath.h"

#include <cmath>

namespace adventure
{
	namespace
	{
		Vector3 halfExtents(const Player& p, const MoveTuning& t)
		{
			float h = p.crouched ? t.crouchHeight : t.height;
			return Vector3{t.radius, h * 0.5f, t.radius};
		}

		void applyFriction(Vector3& vel, const MoveTuning& t, float dt)
		{
			float speed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
			if (speed < 0.001f)
			{
				vel.x = 0.0f;
				vel.z = 0.0f;
				return;
			}
			float drop = fmaxf(speed, t.stopSpeed) * t.friction * dt;
			float scale = fmaxf(speed - drop, 0.0f) / speed;
			vel.x *= scale;
			vel.z *= scale;
		}

		void accelerate(Vector3& vel, Vector3 wishDir, float wishSpeed, float accel, float dt)
		{
			float current = vel.x * wishDir.x + vel.z * wishDir.z;
			float add = wishSpeed - current;
			if (add <= 0.0f)
				return;
			float step = fminf(accel * wishSpeed * dt, add);
			vel.x += wishDir.x * step;
			vel.z += wishDir.z * step;
		}

		// Try to move the center along one axis; revert if it lands in a solid. Returns true if blocked.
		bool moveAxis(Player& p, Vector3 h, const world::CollisionWorld& w, int axis, float delta)
		{
			Vector3 c = p.position;
			if (axis == 0)
				c.x += delta;
			else if (axis == 1)
				c.y += delta;
			else
				c.z += delta;
			if (w.overlaps(c, h))
				return true;
			p.position = c;
			return false;
		}
	} // namespace

	void updatePlayer(Player& p, const MoveInput& in, const world::CollisionWorld& world, const MoveTuning& t, float dt)
	{
		// Crouch toggles the collider height while keeping the feet planted (shift the center), and
		// standing up requires headroom — otherwise growing the AABB around a fixed center would push it
		// into the floor/ceiling and jam every axis (the "stuck after crouch" bug).
		if (in.crouch != p.crouched)
		{
			const float dh = (t.height - t.crouchHeight) * 0.5f;
			if (in.crouch)
			{
				p.position.y -= dh; // shrink from the top: lower center, feet stay
				p.crouched = true;
			}
			else
			{
				// Inset the test box slightly so merely resting on the floor (touching == overlap)
				// doesn't read as blocked; this checks real headroom (a low ceiling) only.
				Vector3 fullHalf = {t.radius - 0.02f, t.height * 0.5f - 0.03f, t.radius - 0.02f};
				Vector3 stand = {p.position.x, p.position.y + dh, p.position.z};
				if (!world.overlaps(stand, fullHalf)) // only stand if there's headroom
				{
					p.position = stand;
					p.crouched = false;
				}
			}
		}
		Vector3 h = halfExtents(p, t);

		// Yaw-relative wish direction (horizontal). yaw 0 => forward -Z, right +X.
		Vector3 fwd = {sinf(p.yaw), 0.0f, -cosf(p.yaw)};
		Vector3 rightv = {cosf(p.yaw), 0.0f, sinf(p.yaw)};
		Vector3 wish = {fwd.x * in.forward + rightv.x * in.right, 0.0f, fwd.z * in.forward + rightv.z * in.right};
		float wl = std::sqrt(wish.x * wish.x + wish.z * wish.z);
		if (wl > 0.001f)
		{
			wish.x /= wl;
			wish.z /= wl;
		}

		const float wishSpeed = in.sprint ? t.sprintSpeed : t.moveSpeed;

		if (p.onGround)
		{
			applyFriction(p.velocity, t, dt);
			accelerate(p.velocity, wish, wishSpeed, t.accel, dt);
			if (in.jump)
			{
				p.velocity.y = t.jumpSpeed;
				p.onGround = false;
			}
		}
		else
		{
			accelerate(p.velocity, wish, wishSpeed, t.airAccel, dt);
		}

		p.velocity.y -= t.gravity * dt;

		// Axis-separated horizontal integration: X, then Z (walls slide).
		const float velX = p.velocity.x, velZ = p.velocity.z;
		const Vector3 preH = p.position;
		bool bx = moveAxis(p, h, world, 0, velX * dt);
		bool bz = moveAxis(p, h, world, 2, velZ * dt);

		// Stair step-up: if a grounded move was blocked, retry lifted by stepHeight and settle onto the
		// step. Genuine walls (taller than stepHeight) fail the retry and keep the wall-slide behaviour.
		bool stepped = false;
		if ((bx || bz) && p.onGround && (velX != 0.0f || velZ != 0.0f))
		{
			const Vector3 flat = p.position;
			const Vector3 lifted = {preH.x, preH.y + t.stepHeight, preH.z};
			if (!world.overlaps(lifted, h))
			{
				p.position = lifted;
				moveAxis(p, h, world, 0, velX * dt);
				moveAxis(p, h, world, 2, velZ * dt);
				const bool advanced = std::fabs(p.position.x - flat.x) > 0.001f ||
				                      std::fabs(p.position.z - flat.z) > 0.001f;
				float dropped = 0.0f;
				bool landed = false;
				while (dropped <= t.stepHeight + 0.001f)
				{
					if (moveAxis(p, h, world, 1, -0.04f)) // hit the step surface
					{
						landed = true;
						break;
					}
					dropped += 0.04f;
				}
				stepped = advanced && landed;
			}
			if (!stepped)
				p.position = flat;
		}
		if (!stepped)
		{
			if (bx)
				p.velocity.x = 0.0f;
			if (bz)
				p.velocity.z = 0.0f;
		}

		bool blockedY = moveAxis(p, h, world, 1, p.velocity.y * dt);
		if (blockedY)
		{
			if (p.velocity.y < 0.0f)
				p.onGround = true;
			p.velocity.y = 0.0f;
		}
		else
		{
			p.onGround = false;
		}
	}
} // namespace adventure
