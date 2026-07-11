#pragma once
#include "input/InputMap.h"

// Live input queries for actions — dispatches a bound code to raylib's key/mouse polling. Needs a window,
// so it lives outside the pure InputMap.
namespace adventure
{
	bool actionDown(const InputMap& m, Action a);
	bool actionPressed(const InputMap& m, Action a);
	bool actionReleased(const InputMap& m, Action a);
} // namespace adventure
