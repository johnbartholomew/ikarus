#ifndef ORB_INPUT_H
#define ORB_INPUT_H

#define NOMINMAX
#include <Windows.h>

// nb: these are chosen to match the windows virtual key codes
SCOPED_ENUM(KeyCode)
{
	Invalid    = 0,

	Escape     = VK_ESCAPE,

	Return     = VK_RETURN,
	Backspace  = VK_BACK,
	Tab        = VK_TAB,
	Space      = VK_SPACE,

	LeftShift  = VK_LSHIFT,
	RightShift = VK_RSHIFT,
	LeftCtrl   = VK_LCONTROL,
	RightCtrl  = VK_RCONTROL,
	LeftWin    = VK_LWIN,
	RightWin   = VK_RWIN,
	LeftMenu   = VK_LMENU,
	RightMenu  = VK_RMENU,
	Screenshot = VK_SNAPSHOT,
	Pause      = VK_PAUSE,

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

	NumPad0    = VK_NUMPAD0,
	NumPad1    = VK_NUMPAD1,
	NumPad2    = VK_NUMPAD2,
	NumPad3    = VK_NUMPAD3,
	NumPad4    = VK_NUMPAD4,
	NumPad5    = VK_NUMPAD5,
	NumPad6    = VK_NUMPAD6,
	NumPad7    = VK_NUMPAD7,
	NumPad8    = VK_NUMPAD8,
	NumPad9    = VK_NUMPAD9,

	NumPadDot      = VK_DECIMAL,
	NumPadEnter    = VK_SEPARATOR,
	NumPadAdd      = VK_ADD,
	NumPadSubtract = VK_SUBTRACT,
	NumPadMultiply = VK_MULTIPLY,
	NumPadDivide   = VK_DIVIDE,
	
	ArrowLeft  = VK_LEFT,
	ArrowRight = VK_RIGHT,
	ArrowUp    = VK_UP,
	ArrowDown  = VK_DOWN,
	
	Insert     = VK_INSERT,
	Delete     = VK_DELETE,
	Home       = VK_HOME,
	End        = VK_END,
	PageUp     = VK_PRIOR,
	PageDown   = VK_NEXT,

	F1         = VK_F1,
	F2         = VK_F2,
	F3         = VK_F3,
	F4         = VK_F4,
	F5         = VK_F5,
	F6         = VK_F6,
	F7         = VK_F7,
	F8         = VK_F8,
	F9         = VK_F9,
	F10        = VK_F10,
	F11        = VK_F11,
	F12        = VK_F12,

	NumLock    = VK_NUMLOCK,
	ScrollLock = VK_SCROLL,
	CapsLock   = VK_CAPITAL,

	// mouse button key codes
	MouseL  = VK_LBUTTON,
	MouseR  = VK_RBUTTON,
	MouseM  = VK_MBUTTON,
	MouseX1 = VK_XBUTTON1,
	MouseX2 = VK_XBUTTON2,

	// not a real KeyCode
	KEY_CODE_COUNT = 0xff
};

SCOPED_ENUM(MouseButton)
{
	Left    = 0,
	Right   = 1,
	Middle  = 2,
	X1      = 3, // extra buttons; typically used for, e.g., browser back & forward
	X2      = 4,

	MOUSE_BUTTON_COUNT = 5
};

class OrbInput
{
public:
	explicit OrbInput();
	~OrbInput();

	// ==== input event methods ====

	// beginFrame gives an opportunity to reset deltas
	void beginFrame();

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

	bool isMouseButtonDown(int button) const
	{ return isKeyDown(buttonToKeyCode(button)); }
	bool wasMouseButtonPressed(int button) const
	{ return wasKeyPressed(buttonToKeyCode(button)); }
	bool wasMouseButtonReleased(int button) const
	{ return wasKeyReleased(buttonToKeyCode(button)); }

	bool isKeyDown(int key) const
	{ assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT); return (mKeyState[key] & Down); }
	bool wasKeyPressed(int key) const
	{ assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT);return (mKeyState[key] == Pressed); }
	bool wasKeyReleased(int key) const
	{ assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT);return (mKeyState[key] == Released); }

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
