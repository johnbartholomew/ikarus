#include "Global.h"
#include "OrbInput.h"

OrbInput::OrbInput()
:	mMousePos(0, 0),
	mMouseDelta(0, 0),
	mWheelPos(0),
	mWheelDelta(0)
{
	for (int i = 0; i < MouseButton::MOUSE_BUTTON_COUNT; ++i)
		mMouseClickPos[i] = vec2i(0, 0);
	for (int i = 0; i < KeyCode::KEY_CODE_COUNT; ++i)
		mKeyState[i] = 0;
}

OrbInput::~OrbInput()
{
}

void OrbInput::beginFrame()
{
	mMouseDelta = vec2i(0, 0);
	mWheelDelta = 0;

	for (int i = 0; i < KeyCode::KEY_CODE_COUNT; ++i)
	{
		// reset the changed flag
		mKeyState[i] &= Down;
	}
}

void OrbInput::mousePress(int button, int x, int y)
{
	assert(button >= 0 && button < MouseButton::MOUSE_BUTTON_COUNT);

	mouseMove(x, y);
	keyPress(buttonToKeyCode(button));
	mMouseClickPos[button] = vec2i(x, y);
}

void OrbInput::mouseRelease(int button, int x, int y)
{
	assert(button >= 0 && button < MouseButton::MOUSE_BUTTON_COUNT);
	
	mouseMove(x, y);
	keyRelease(buttonToKeyCode(button));
}

void OrbInput::mouseMove(int x, int y)
{
	const vec2i v(x, y);
	mMouseDelta += v - mMousePos;
	mMousePos = v;
}

void OrbInput::mouseScroll(int delta)
{
	mWheelPos += delta;
	mWheelDelta += delta;
}

void OrbInput::keyPress(int key)
{
	assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT);
	mKeyState[key] = Pressed;
}

void OrbInput::keyRelease(int key)
{
	assert(key >= 0 && key < KeyCode::KEY_CODE_COUNT);
	mKeyState[key] = Released;
}
