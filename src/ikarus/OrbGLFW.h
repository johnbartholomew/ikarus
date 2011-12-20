#ifndef ORB_GLFW_H
#define ORB_GLFW_H

#include "OrbInput.h"

class OrbWindow
{
public:
	explicit OrbWindow();
	~OrbWindow();

	bool processEvents();
	void open(const wchar_t *title, int width, int height);
	void flipGL();

	OrbInput input;
private:
	void handleKey(int key, int action);
	void handleChar(int character, int action);
	void handleMouseButton(int button, int action);
	void handleMousePos(int x, int y);
	void handleMouseWheel(int pos);
	void handleWindowSize(int x, int y);

	int mMouseWheelPos;

	static void dispatchKeyEvent(int key, int action);
	static void dispatchCharEvent(int character, int action);
	static void dispatchMouseButtonEvent(int button, int action);
	static void dispatchMousePosEvent(int x, int y);
	static void dispatchMouseWheelEvent(int pos);
	static void dispatchWindowSizeEvent(int width, int height);
};


#endif
