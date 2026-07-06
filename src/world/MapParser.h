#pragma once
#include "world/MapTypes.h"

#include <string>

namespace adventure::world
{
	struct MapParseResult
	{
		MapData data;
		bool ok = false;
		std::string error;
	};

	// Parse Valve 220 .map text into map-space data. Tolerant: returns ok=false + error on structural
	// failure, never throws. Coordinate conversion happens later in BrushGeometry, not here.
	MapParseResult parseMap(const std::string& text);
} // namespace adventure::world
