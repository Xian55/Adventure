#include <doctest/doctest.h>
#include "combat/Projectile.h"

using namespace adventure;

TEST_CASE("a bolt flies, drops under gravity, and despawns on lifetime")
{
	std::vector<Projectile> shots(1);
	shots[0].position = {0, 2, 0};
	shots[0].velocity = {10, 0, 0};
	shots[0].life = 0.2f;

	updateProjectiles(shots, 20.0f, 0.1f);
	CHECK(shots[0].position.x == doctest::Approx(1.0)); // moved along +x
	CHECK(shots[0].position.y < 2.0f);                  // dropped
	CHECK(shots[0].active);

	updateProjectiles(shots, 20.0f, 0.15f); // life now < 0
	CHECK_FALSE(shots[0].active);
}

TEST_CASE("a bolt damages the enemy it hits, then is spent")
{
	EnemyTuning t;
	std::vector<Enemy> es(1);
	es[0].position = {1, 1, 0};
	es[0].health = 50.0f;
	std::vector<Projectile> shots(1);
	shots[0].position = {1, 1, 0}; // overlapping the enemy
	shots[0].damage = 20.0f;

	int kills = resolveProjectileHits(shots, es, t);
	CHECK(kills == 0);
	CHECK(es[0].health == doctest::Approx(30.0));
	CHECK(es[0].state == EnemyState::Stagger);
	CHECK_FALSE(shots[0].active); // spent
}

TEST_CASE("a lethal bolt kills; a missing bolt does nothing")
{
	EnemyTuning t;
	std::vector<Enemy> es(1);
	es[0].position = {0, 1, 0};
	es[0].health = 15.0f;
	std::vector<Projectile> shots{
	    Projectile{{0, 1, 0}, {0, 0, 0}, 20.0f, 3.0f, true}, // hits
	    Projectile{{9, 9, 9}, {0, 0, 0}, 20.0f, 3.0f, true}, // far miss
	};
	int kills = resolveProjectileHits(shots, es, t);
	CHECK(kills == 1);
	CHECK(es[0].state == EnemyState::Dead);
	CHECK_FALSE(shots[0].active);
	CHECK(shots[1].active); // missed -> still flying
}
