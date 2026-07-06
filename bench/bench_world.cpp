#include "Bench.h"
#include "world/BrushGeometry.h"
#include "world/MapParser.h"

using namespace adventure::world;

namespace
{
	const char* kCube =
	    "{\n\"classname\" \"worldspawn\"\n{\n"
	    "( -64 -64 -64 ) ( -64 -64 64 ) ( -64 64 -64 ) t [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 64 64 64 ) ( 64 -64 64 ) ( 64 64 -64 ) t [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( -64 -64 -64 ) ( 64 -64 -64 ) ( -64 -64 64 ) t [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 64 64 64 ) ( -64 64 64 ) ( 64 64 -64 ) t [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( -64 -64 -64 ) ( -64 64 -64 ) ( 64 -64 -64 ) t [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	    "( 64 64 64 ) ( 64 -64 64 ) ( -64 64 64 ) t [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	    "}\n}\n";
}

// Map load is not per-frame, but it must stay snappy. Parse + build a brush per iteration.
ADV_BENCH(world_parse_and_build, 20000, 1500.0)
{
	MapParseResult r = parseMap(kCube);
	WorldGeometry w = buildWorld(r.data);
	advbench::keep(w.meshes.size());
}
