#include "Global.h"
#include "OrbGLFW.h"
#include <GL/glfw.h>

static OrbWindow *g_window = 0;

static int GLFW_KEY_TO_ORB_KEY[GLFW_KEY_LAST + 1 - GLFW_KEY_SPECIAL];

static void init_glfw_key_map()
{
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_SPECIAL] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_ESC] = KeyCode::Escape;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F1] = KeyCode::F1;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F2] = KeyCode::F2;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F3] = KeyCode::F3;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F4] = KeyCode::F4;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F5] = KeyCode::F5;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F6] = KeyCode::F6;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F7] = KeyCode::F7;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F8] = KeyCode::F8;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F9] = KeyCode::F9;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F10] = KeyCode::F10;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F11] = KeyCode::F11;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F12] = KeyCode::F12;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F13] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F14] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F15] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F16] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F17] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F18] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F19] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F20] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F21] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F22] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F23] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F24] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_F25] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_UP] = KeyCode::ArrowUp;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_DOWN] = KeyCode::ArrowDown;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_LEFT] = KeyCode::ArrowLeft;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_RIGHT] = KeyCode::ArrowRight;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_LSHIFT] = KeyCode::ShiftL;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_RSHIFT] = KeyCode::ShiftR;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_LCTRL] = KeyCode::CtrlL;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_RCTRL] = KeyCode::CtrlR;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_LALT] = KeyCode::AltL;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_RALT] = KeyCode::AltR;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_TAB] = KeyCode::Tab;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_ENTER] = KeyCode::Return;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_BACKSPACE] = KeyCode::Backspace;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_INSERT] = KeyCode::Insert;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_DEL] = KeyCode::Delete;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_PAGEUP] = KeyCode::PageUp;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_PAGEDOWN] = KeyCode::PageDown;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_HOME] = KeyCode::Home;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_END] = KeyCode::End;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_0] = KeyCode::NumPad0;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_1] = KeyCode::NumPad1;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_2] = KeyCode::NumPad2;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_3] = KeyCode::NumPad3;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_4] = KeyCode::NumPad4;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_5] = KeyCode::NumPad5;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_6] = KeyCode::NumPad6;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_7] = KeyCode::NumPad7;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_8] = KeyCode::NumPad8;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_9] = KeyCode::NumPad9;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_DIVIDE] = KeyCode::NumPadDivide;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_MULTIPLY] = KeyCode::NumPadMultiply;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_SUBTRACT] = KeyCode::NumPadSubtract;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_ADD] = KeyCode::NumPadAdd;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_DECIMAL] = KeyCode::NumPadDecimal;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_EQUAL] = KeyCode::INVALID;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_ENTER] = KeyCode::NumPadEnter;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_KP_NUM_LOCK] = KeyCode::NumLock;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_CAPS_LOCK] = KeyCode::CapsLock;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_SCROLL_LOCK] = KeyCode::ScrollLock;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_PAUSE] = KeyCode::Pause;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_LSUPER] = KeyCode::SuperL;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_RSUPER] = KeyCode::SuperR;
	GLFW_KEY_TO_ORB_KEY[GLFW_KEY_MENU] = KeyCode::INVALID;
}

static int glfw_key_to_orb_key(int key)
{
	static bool initialised = false;
	if (!initialised)
	{
		init_glfw_key_map();
		initialised = true;
	}
	if (key == GLFW_KEY_UNKNOWN) return 0;
	if (key < GLFW_KEY_SPECIAL) return key;
	if (key > GLFW_KEY_LAST) return 0;
	return GLFW_KEY_TO_ORB_KEY[key];
}

OrbWindow::OrbWindow()
{
	assert(!g_window);
	if (glfwInit() != GL_TRUE)
		throw std::runtime_error("failed to initialise GLFW");
	g_window = this;
}

OrbWindow::~OrbWindow()
{
	assert(g_window == this);
	glfwTerminate();
	g_window = 0;
}

bool OrbWindow::processEvents()
{
	glfwPollEvents();
	// callbacks should be fired from inside glfwPollEvents()
	return (glfwGetWindowParam(GLFW_OPENED) == GL_TRUE);
}

void OrbWindow::open(const wchar_t *title, int width, int height)
{
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_FALSE);
	int ret = glfwOpenWindow(width, height, 8, 8, 8, 0, 16, 0, GLFW_WINDOW);
	if (ret != GL_TRUE)
		throw std::runtime_error("failed to create OpenGL window");
	glfwDisable(GLFW_AUTO_POLL_EVENTS);
	//glfwSetWindowTitle(title);
	glfwSetKeyCallback(&OrbWindow::dispatchKeyEvent);
	glfwSetCharCallback(&OrbWindow::dispatchCharEvent);
	glfwSetMouseButtonCallback(&OrbWindow::dispatchMouseButtonEvent);
	glfwSetMousePosCallback(&OrbWindow::dispatchMousePosEvent);
	glfwSetMouseWheelCallback(&OrbWindow::dispatchMouseWheelEvent);
	glfwSetWindowSizeCallback(&OrbWindow::dispatchWindowSizeEvent);

	glfwGetWindowSize(&width, &height);
	input.windowResize(width, height);
	mMouseWheelPos = glfwGetMouseWheel();
}

void OrbWindow::flipGL()
{
	glfwSwapBuffers();
}

void OrbWindow::handleKey(int key, int action)
{
	key = glfw_key_to_orb_key(key);
	if (action == GLFW_PRESS)
		input.keyPress(key);
	else
		input.keyRelease(key);
}

void OrbWindow::handleChar(int character, int action)
{
	assert(character >= 32 && character < 127); // printable ASCII
	if (action == GLFW_PRESS)
		input.keyPress(character);
	else
		input.keyRelease(character);
}

void OrbWindow::handleMouseButton(int button, int action)
{
	if (action == GLFW_PRESS)
		input.mousePress(button);
	else
		input.mouseRelease(button);
}

void OrbWindow::handleMousePos(int x, int y)
{
	input.mouseMove(x, y);
}

void OrbWindow::handleMouseWheel(int pos)
{
	input.mouseScroll(pos - mMouseWheelPos);
	mMouseWheelPos = pos;
}

void OrbWindow::handleWindowSize(int x, int y)
{
	input.windowResize(x, y);
	glViewport(0, 0, x, y);
}

void OrbWindow::dispatchKeyEvent(int key, int action)
{
	assert(g_window);
	g_window->handleKey(key, action);
}

void OrbWindow::dispatchCharEvent(int character, int action)
{
	assert(g_window);
	g_window->handleChar(character, action);
}

void OrbWindow::dispatchMouseButtonEvent(int button, int action)
{
	assert(g_window);
	g_window->handleMouseButton(button, action);
}

void OrbWindow::dispatchMousePosEvent(int x, int y)
{
	assert(g_window);
	g_window->handleMousePos(x, y);
}

void OrbWindow::dispatchMouseWheelEvent(int pos)
{
	assert(g_window);
	g_window->handleMouseWheel(pos);
}

void OrbWindow::dispatchWindowSizeEvent(int width, int height)
{
	assert(g_window);
	g_window->handleWindowSize(width, height);
}
