#pragma once

struct MouseEvent
{
	enum class EventType
	{ MousePress, MouseDoubleClick, MouseRelease, MouseMove };
	EventType type;

	int x = 0, y = 0; // Position relative to the window

	enum { LeftButton = 1, RightButton = 2, MiddleButton = 4 };
	enum { ShiftModifier = 1, ControlModifier = 2, AltModifier = 4 };

	unsigned char button = 0; // The button at the origin of this event (0 for a Move event)
	unsigned char buttons = 0; // Combinaison of all buttons pressed when the event was generated (even for Move)
	unsigned char modifiers = 0; // Keyboard modifiers that existed before the event occurred

	int width, height; // Dimensions of the canvas
};
