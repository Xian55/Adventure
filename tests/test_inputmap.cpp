#include <doctest/doctest.h>
#include "input/InputMap.h"

using namespace adventure;

TEST_CASE("default bindings cover every action")
{
	InputMap m = defaultBindings();
	for (int i = 0; i < kActionCount; ++i)
		CHECK(m.codes[i] != 0); // every action bound
	CHECK(actionCode(m, Action::Interact) == 'E');
	CHECK(isMouseCode(actionCode(m, Action::Attack)));
	CHECK(mouseButton(actionCode(m, Action::Attack)) == 0);
}

TEST_CASE("mouse codes round-trip and stay clear of key codes")
{
	CHECK(isMouseCode(mouseCode(1)));
	CHECK(mouseButton(mouseCode(1)) == 1);
	CHECK_FALSE(isMouseCode('W')); // a letter is a key code, not a mouse code
}

TEST_CASE("rebinding an action changes only that action")
{
	InputMap m = defaultBindings();
	bindAction(m, Action::Interact, 'F');
	CHECK(actionCode(m, Action::Interact) == 'F');
	CHECK(actionCode(m, Action::Kick) == 'F'); // Kick default was F, untouched
	CHECK(actionCode(m, Action::Jump) == 32);
}

TEST_CASE("save then load round-trips the bindings")
{
	InputMap m = defaultBindings();
	bindAction(m, Action::Interact, 'Q');
	bindAction(m, Action::Kick, mouseCode(2));
	const std::string text = saveBindings(m);

	InputMap back = loadBindings(text);
	CHECK(actionCode(back, Action::Interact) == 'Q');
	CHECK(actionCode(back, Action::Kick) == mouseCode(2));
	CHECK(actionCode(back, Action::Jump) == 32); // unchanged
}

TEST_CASE("loading tolerates junk and keeps defaults for missing actions")
{
	const std::string text =
	    "# a comment\n"
	    "\n"
	    "Interact 81\n"        // Q
	    "Bogus 5\n"            // unknown action -> ignored
	    "MoveForward notnum\n" // bad code -> ignored, keeps default
	    "garbage line here\n";
	InputMap m = loadBindings(text);
	CHECK(actionCode(m, Action::Interact) == 81);
	CHECK(actionCode(m, Action::MoveForward) == 'W'); // default kept
}

TEST_CASE("code names are human-readable")
{
	CHECK(codeName('W') == "W");
	CHECK(codeName(32) == "Space");
	CHECK(codeName(mouseCode(0)) == "L-Mouse");
	CHECK(codeName(mouseCode(1)) == "R-Mouse");
}
