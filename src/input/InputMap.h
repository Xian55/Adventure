#pragma once
#include <string>

// Named input actions mapped to input codes (keyboard keys or mouse buttons), so keys are rebindable.
// Pure data + logic (no raylib window) -> headless-testable. Live querying is in InputQuery (raylib).
namespace adventure
{
	enum class Action
	{
		MoveForward,
		MoveBack,
		MoveLeft,
		MoveRight,
		Jump,
		Crouch,
		Sprint,
		Attack,
		Block,
		Kick,
		Interact,
		Count,
	};
	constexpr int kActionCount = (int)Action::Count;

	// Input codes: a raylib key code as-is, or a mouse button encoded as kMouseBase + button.
	constexpr int kMouseBase = 1000;
	inline int mouseCode(int button)
	{
		return kMouseBase + button;
	}
	inline bool isMouseCode(int code)
	{
		return code >= kMouseBase;
	}
	inline int mouseButton(int code)
	{
		return code - kMouseBase;
	}

	struct InputMap
	{
		int codes[kActionCount] = {};
	};

	InputMap defaultBindings();
	void bindAction(InputMap& m, Action a, int code);
	int actionCode(const InputMap& m, Action a);

	const char* actionName(Action a); // stable id used in the config file + UI label
	std::string codeName(int code);   // human label for a key/mouse code (for the UI)

	// Config file (one "action code" per line). Unknown/blank lines ignored; missing actions keep defaults.
	std::string saveBindings(const InputMap& m);
	InputMap loadBindings(const std::string& text); // starts from defaults, applies parsed overrides
} // namespace adventure
