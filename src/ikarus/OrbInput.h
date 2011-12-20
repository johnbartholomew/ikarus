#ifndef ORB_INPUT_H
#define ORB_INPUT_H

#include "SDL_keysym.h"

// nb: these are chosen to match the windows virtual key codes
SCOPED_ENUM(KeyCode)
{
	Invalid    = 0,

	Escape     = SDLK_ESCAPE,

	Return     = SDLK_RETURN,
	Backspace  = SDLK_BACKSPACE,
	Tab        = SDLK_TAB,
	Space      = SDLK_SPACE,

	LeftShift  = SDLK_LSHIFT,
	RightShift = SDLK_RSHIFT,
	LeftCtrl   = SDLK_LCTRL,
	RightCtrl  = SDLK_RCTRL,
	LeftWin    = SDLK_LMETA,
	RightWin   = SDLK_RMETA,
	LeftMenu   = SDLK_LSUPER,
	RightMenu  = SDLK_RSUPER,
	Screenshot = SDLK_PRINT,
	Pause      = SDLK_PAUSE,

	Key0       = '0',
	Key1       = '1',
	Key2       = '2',
	Key3       = '3',
	Key4       = '4',
	Key5       = '5',
	Key6       = '6',
	Key7       = '7',
	Key8       = '8',
	Key9       = '9',

	A          = 'A',
	B          = 'B',
	C          = 'C',
	D          = 'D',
	E          = 'E',
	F          = 'F',
	G          = 'G',
	H          = 'H',
	I          = 'I',
	J          = 'J',
	K          = 'K',
	L          = 'L',
	M          = 'M',
	N          = 'N',
	O          = 'O',
	P          = 'P',
	Q          = 'Q',
	R          = 'R',
	S          = 'S',
	T          = 'T',
	U          = 'U',
	V          = 'V',
	W          = 'W',
	X          = 'X',
	Y          = 'Y',
	Z          = 'Z',

	NumPad0    = SDLK_KP0,
	NumPad1    = SDLK_KP1,
	NumPad2    = SDLK_KP2,
	NumPad3    = SDLK_KP3,
	NumPad4    = SDLK_KP4,
	NumPad5    = SDLK_KP5,
	NumPad6    = SDLK_KP6,
	NumPad7    = SDLK_KP7,
	NumPad8    = SDLK_KP8,
	NumPad9    = SDLK_KP9,

	NumPadDot      = SDLK_KP_PERIOD,
	NumPadEnter    = SDLK_KP_ENTER,
	NumPadAdd      = SDLK_KP_PLUS,
	NumPadSubtract = SDLK_KP_MINUS,
	NumPadMultiply = SDLK_KP_MULTIPLY,
	NumPadDivide   = SDLK_KP_DIVIDE,
	
	ArrowLeft  = SDLK_LEFT,
	ArrowRight = SDLK_RIGHT,
	ArrowUp    = SDLK_UP,
	ArrowDown  = SDLK_DOWN,
	
	Insert     = SDLK_INSERT,
	Delete     = SDLK_DELETE,
	Home       = SDLK_HOME,
	End        = SDLK_END,
	PageUp     = SDLK_PAGEUP,
	PageDown   = SDLK_PAGEDOWN,

	F1         = SDLK_F1,
	F2         = SDLK_F2,
	F3         = SDLK_F3,
	F4         = SDLK_F4,
	F5         = SDLK_F5,
	F6         = SDLK_F6,
	F7         = SDLK_F7,
	F8         = SDLK_F8,
	F9         = SDLK_F9,
	F10        = SDLK_F10,
	F11        = SDLK_F11,
	F12        = SDLK_F12,

	NumLock    = SDLK_NUMLOCK,
	ScrollLock = SDLK_SCROLLOCK,
	CapsLock   = SDLK_CAPSLOCK,

	// mouse button key codes
	MouseL  = SDLK_LAST,
	MouseR,
	MouseM,
	MouseX1,
	MouseX2,

	// not a real KeyCode
	KEY_CODE_COUNT
};

SCOPED_ENUM(MouseButton)
{
	Left    = 0,
	Right   = 1,
	Middle  = 2,
	X1      = 3, // extra buttons; typically used for, e.g., browser back & forward
	X2      = 4,

	MOUSE_BUTTON_COUNT
};

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
	{ mouseRelease(button, mMousePos.x, mMousePos.y); }
	void mouseRelease(int button)
	{ mousePress(button, mMousePos.x, mMousePos.y); }

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
	{ assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT); return (mKeyState[key] & Down); }
	bool wasKeyPressed(int key) const
	{ assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT); return (mKeyState[key] == Pressed); }
	bool wasKeyReleased(int key) const
	{ assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT); return (mKeyState[key] == Released); }

	int buttonToKeyCode(int button) const
	{
		switch (button)
		{
		case MouseButton::Left:   return KeyCode::MouseL;
		case MouseButton::Right:  return KeyCode::MouseR;
		case MouseButton::Middle: return KeyCode::MouseM;
		case MouseButton::X1:     return KeyCode::MouseX1;
		case MouseButton::X2:     return KeyCode::MouseX2;
		default: assert(0); return KeyCode::Invalid;
		}
	}

private:
	enum KeyState
	{
		Up       = 0,
		Down     = 1,
		Released = 2,
		Pressed  = 3
	};

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
