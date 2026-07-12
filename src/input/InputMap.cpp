#include "input/InputMap.h"

#include <sstream>

namespace adventure
{
	namespace
	{
		// raylib keycodes used as defaults (avoids including raylib in this pure TU).
		enum
		{
			K_SPACE = 32,
			K_D = 68,
			K_E = 69,
			K_F = 70,
			K_S = 83,
			K_W = 87,
			K_A = 65,
			K_Q = 81,
			K_K = 75,
			K_R = 82,
			K_X = 88,
			K_LSHIFT = 340,
			K_LCTRL = 341,
		};

		const char* kNames[kActionCount] = {
		    "MoveForward",
		    "MoveBack",
		    "MoveLeft",
		    "MoveRight",
		    "Jump",
		    "Crouch",
		    "Sprint",
		    "Attack",
		    "Block",
		    "Kick",
		    "Interact",
		    "NextWeapon",
		    "SkillMenu",
		    "Cast",
		    "NextSpell",
		};
	} // namespace

	InputMap defaultBindings()
	{
		InputMap m;
		m.codes[(int)Action::MoveForward] = K_W;
		m.codes[(int)Action::MoveBack] = K_S;
		m.codes[(int)Action::MoveLeft] = K_A;
		m.codes[(int)Action::MoveRight] = K_D;
		m.codes[(int)Action::Jump] = K_SPACE;
		m.codes[(int)Action::Crouch] = K_LCTRL;
		m.codes[(int)Action::Sprint] = K_LSHIFT;
		m.codes[(int)Action::Attack] = mouseCode(0); // left mouse
		m.codes[(int)Action::Block] = mouseCode(1);  // right mouse
		m.codes[(int)Action::Kick] = K_F;
		m.codes[(int)Action::Interact] = K_E;
		m.codes[(int)Action::NextWeapon] = K_Q;
		m.codes[(int)Action::SkillMenu] = K_K;
		m.codes[(int)Action::Cast] = K_R;      // cast the selected spell
		m.codes[(int)Action::NextSpell] = K_X; // cycle the spellbook
		return m;
	}

	void bindAction(InputMap& m, Action a, int code)
	{
		m.codes[(int)a] = code;
	}

	int actionCode(const InputMap& m, Action a)
	{
		return m.codes[(int)a];
	}

	const char* actionName(Action a)
	{
		const int i = (int)a;
		return (i >= 0 && i < kActionCount) ? kNames[i] : "?";
	}

	std::string codeName(int code)
	{
		if (isMouseCode(code))
		{
			switch (mouseButton(code))
			{
			case 0:
				return "L-Mouse";
			case 1:
				return "R-Mouse";
			case 2:
				return "M-Mouse";
			default:
				return "Mouse" + std::to_string(mouseButton(code));
			}
		}
		if (code >= 'A' && code <= 'Z')
			return std::string(1, (char)code); // letters map to their ASCII char
		if (code >= '0' && code <= '9')
			return std::string(1, (char)code);
		switch (code)
		{
		case 32:
			return "Space";
		case 340:
			return "L-Shift";
		case 341:
			return "L-Ctrl";
		case 257:
			return "Enter";
		case 256:
			return "Esc";
		case 258:
			return "Tab";
		default:
			return "Key" + std::to_string(code);
		}
	}

	std::string saveBindings(const InputMap& m)
	{
		std::string out = "# Adventure keybindings — <Action> <code>. Codes: raylib key codes, or 1000+button for mouse.\n";
		for (int i = 0; i < kActionCount; ++i)
		{
			out += kNames[i];
			out += ' ';
			out += std::to_string(m.codes[i]);
			out += '\n';
		}
		return out;
	}

	InputMap loadBindings(const std::string& text)
	{
		InputMap m = defaultBindings();
		std::istringstream in(text);
		std::string name;
		int code = 0;
		std::string line;
		while (std::getline(in, line))
		{
			if (line.empty() || line[0] == '#')
				continue;
			std::istringstream ls(line);
			if (!(ls >> name >> code))
				continue;
			for (int i = 0; i < kActionCount; ++i)
				if (name == kNames[i])
				{
					m.codes[i] = code;
					break;
				}
		}
		return m;
	}
} // namespace adventure
