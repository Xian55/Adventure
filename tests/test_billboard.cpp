#include <doctest/doctest.h>
#include "render/Billboard.h"

using namespace adventure;

namespace
{
	Enemy at(float x, float z, RenderKind kind = RenderKind::Billboard, bool active = true)
	{
		Enemy e;
		e.position = {x, 0.0f, z};
		e.render = kind;
		e.active = active;
		return e;
	}
} // namespace

TEST_CASE("billboards draw back-to-front (farthest enemy first)")
{
	std::vector<Enemy> es{at(0, -1.0f), at(0, -10.0f), at(0, -5.0f)};
	std::vector<int> order;
	depthSortEnemies(order, es, Vector3{0, 0, 0});
	REQUIRE(order.size() == 3);
	CHECK(order[0] == 1); // z=-10, farthest
	CHECK(order[1] == 2); // z=-5
	CHECK(order[2] == 0); // z=-1, nearest
}

TEST_CASE("only active, Billboard-kind enemies are drawn")
{
	std::vector<Enemy> es{
	    at(0, -1.0f, RenderKind::Billboard, true),
	    at(0, -2.0f, RenderKind::Box, true),        // cube path, not a billboard
	    at(0, -3.0f, RenderKind::Billboard, false), // despawned
	};
	std::vector<int> order;
	depthSortEnemies(order, es, Vector3{0, 0, 0});
	REQUIRE(order.size() == 1);
	CHECK(order[0] == 0);
}

TEST_CASE("empty input yields empty order")
{
	std::vector<Enemy> es;
	std::vector<int> order{7, 8, 9}; // must be cleared
	depthSortEnemies(order, es, Vector3{0, 0, 0});
	CHECK(order.empty());
}
