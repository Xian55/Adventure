#pragma once
#include <cstdio>
#include <string>

// One axis-aligned box brush (Valve 220, map space). Winding-agnostic, so any 3 points on each plane work.
inline std::string boxBrush(float x1, float x2, float y1, float y2, float z1, float z2)
{
	char buf[2048];
	std::snprintf(buf, sizeof(buf), "{\n"
	                                "( %g %g %g ) ( %g %g %g ) ( %g %g %g ) t [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	                                "( %g %g %g ) ( %g %g %g ) ( %g %g %g ) t [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	                                "( %g %g %g ) ( %g %g %g ) ( %g %g %g ) t [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	                                "( %g %g %g ) ( %g %g %g ) ( %g %g %g ) t [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	                                "( %g %g %g ) ( %g %g %g ) ( %g %g %g ) t [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	                                "( %g %g %g ) ( %g %g %g ) ( %g %g %g ) t [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	                                "}\n",
	              x1,
	              y1,
	              z1,
	              x1,
	              y2,
	              z1,
	              x1,
	              y1,
	              z2, // -X
	              x2,
	              y1,
	              z1,
	              x2,
	              y1,
	              z2,
	              x2,
	              y2,
	              z1, // +X
	              x1,
	              y1,
	              z1,
	              x1,
	              y1,
	              z2,
	              x2,
	              y1,
	              z1, // -Y
	              x1,
	              y2,
	              z1,
	              x2,
	              y2,
	              z1,
	              x1,
	              y2,
	              z2, // +Y
	              x1,
	              y1,
	              z1,
	              x2,
	              y1,
	              z1,
	              x1,
	              y2,
	              z1, // -Z
	              x1,
	              y1,
	              z2,
	              x1,
	              y2,
	              z2,
	              x2,
	              y1,
	              z2 // +Z
	);
	return buf;
}

inline std::string worldspawnOf(const std::string& brushes)
{
	return "{\n\"classname\" \"worldspawn\"\n" + brushes + "}\n";
}

// Single-box worldspawn (used by the collision/geometry tests).
inline std::string boxMap(float x1, float x2, float y1, float y2, float z1, float z2)
{
	return worldspawnOf(boxBrush(x1, x2, y1, y2, z1, z2));
}
