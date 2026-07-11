#include "input/InputQuery.h"

#include "raylib.h"

namespace adventure
{
	bool actionDown(const InputMap& m, Action a)
	{
		const int code = m.codes[(int)a];
		return isMouseCode(code) ? IsMouseButtonDown(mouseButton(code)) : IsKeyDown(code);
	}

	bool actionPressed(const InputMap& m, Action a)
	{
		const int code = m.codes[(int)a];
		return isMouseCode(code) ? IsMouseButtonPressed(mouseButton(code)) : IsKeyPressed(code);
	}

	bool actionReleased(const InputMap& m, Action a)
	{
		const int code = m.codes[(int)a];
		return isMouseCode(code) ? IsMouseButtonReleased(mouseButton(code)) : IsKeyReleased(code);
	}
} // namespace adventure
