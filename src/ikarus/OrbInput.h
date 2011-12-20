#ifndef ORB_INPUT_H
#define ORB_INPUT_H

#include <SDL/SDL_keysym.h>

// nb: these are chosen to match the windows virtual key codes
namespace KeyCode
{
	enum KeyCodeValues
	{
		INVALID     = 0,

		Backspace   = 8,
		Tab         = 9,
		Return      = 13,
		PRINTABLE_BEGIN = 32,
		Space       = 32,
		PRINTABLE_END = 127,

		SPECIAL_BEGIN = 128,

		Escape = SPECIAL_BEGIN,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,

		PrintScreen,
		Pause,
		ScrollLock,
		CapsLock,
		NumLock,

		Insert,
		Delete,
		Home,
		End,
		PageUp,
		PageDown,

		ArrowLeft  = SDLK_LEFT,
		ArrowRight = SDLK_RIGHT,
		ArrowUp    = SDLK_UP,
		ArrowDown  = SDLK_DOWN,

		ShiftL,
		ShiftR,
		CtrlL,
		CtrlR,
		AltL,
		AltR,
		MetaL,
		MetaR,
		SuperL, // Windows key
		SuperR,

		NumPad0,
		NumPad1,
		NumPad2,
		NumPad3,
		NumPad4,
		NumPad5,
		NumPad6,
		NumPad7,
		NumPad8,
		NumPad9,
		NumPadDivide,
		NumPadMultiply,
		NumPadSubtract,
		NumPadAdd,
		NumPadDecimal,
		NumPadEnter,

		SPECIAL_END,

		MOUSE_BEGIN = SPECIAL_END,
		MouseLeft = MOUSE_BEGIN,
		MouseRight,
		MouseMiddle,
		MouseX1,
		MouseX2,
		MouseX3,
		MouseX4,
		MouseX5,
		MouseX6,
		MouseX7,
		MouseX8,
		MOUSE_END,

		KEY_CODE_COUNT = MOUSE_END
	};
}

namespace MouseButton
{
	enum MouseButtonValues
	{
		Left    = 0,
		Right   = 1,
		Middle  = 2,
		// extra buttons; typically used for, e.g., browser back & forward
		X1, X2, X3, X4, X5, X6, X7, X8,
		MOUSE_BUTTON_COUNT
	};
}

class OrbInput
{
public:
	explicit OrbInput();
	~OrbInput();

	// ==== input event methods ====

	// beginFrame gives an opportunity to reset deltas
	void beginFrame();

	// update the window size
	void windowResize(int x, int y);

	// update the input with a mouse click
	void mousePress(int button, int x, int y);
	void mouseRelease(int button, int x, int y);
	
	// update with a mouse click (no position; assume mouse is at its last known position)
	void mousePress(int button)
	{ mousePress(button, mMousePos.x, mMousePos.y); }
	void mouseRelease(int button)
	{ mouseRelease(button, mMousePos.x, mMousePos.y); }

	// update the input with a mouse move
	void mouseMove(int x, int y);
	// update the input with a mouse scroll
	void mouseScroll(int delta);

	// update the input with a key click
	void keyPress(int key);
	void keyRelease(int key);

	// ==== input state getters ====

	vec2i getWindowSize() const
	{ return mWindowSize; }

	vec2i getMousePos() const
	{ return mMousePos; }
	vec2i getMouseDelta() const
	{ return mMouseDelta; }
	
	vec2i getMouseClickPos(int button) const
	{ assert(button >= 0 && button < MouseButton::MOUSE_BUTTON_COUNT); return mMouseClickPos[button]; }

	int getMouseWheelPos() const
	{ return mWheelPos / WHEEL_DELTA; }
	int getMouseWheelDelta() const
	{ return mWheelDelta / WHEEL_DELTA; }

	bool isMouseDown(int button) const
	{ return isKeyDown(buttonToKeyCode(button)); }
	bool wasMousePressed(int button) const
	{ return wasKeyPressed(buttonToKeyCode(button)); }
	bool wasMouseReleased(int button) const
	{ return wasKeyReleased(buttonToKeyCode(button)); }

	bool isKeyDown(int key) const
	{ assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT); return (mKeyState[key] & 1u); }
	bool wasKeyPressed(int key) const
	{ assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT); return (mKeyState[key] & 2u); }
	bool wasKeyReleased(int key) const
	{ assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT); return (mKeyState[key] & 4u); }

	int buttonToKeyCode(int button) const
	{
		assert(button >= 0 && button < MouseButton::MOUSE_BUTTON_COUNT);
		return KeyCode::MOUSE_BEGIN + button;
	}

private:
	// the window's current size (treated as input because the user can change it)
	vec2i mWindowSize;
	// the mouse position at the end of the frame
	vec2i mMousePos;
	// the mouse delta over the frame
	vec2i mMouseDelta;
	// the mouse position at which the button was pressed (useful for dragging)
	// first entry is for the left button, second is for the right button, third is for the middle button
	vec2i mMouseClickPos[MouseButton::MOUSE_BUTTON_COUNT];

	// the current scroll wheel position (starts at zero on initialization)
	int mWheelPos;
	// the change in scroll wheel position
	int mWheelDelta;

	// nb: key state includes the state of the mouse buttons
	unsigned char mKeyState[KeyCode::KEY_CODE_COUNT];
};


#endif
