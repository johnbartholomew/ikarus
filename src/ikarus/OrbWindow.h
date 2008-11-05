#ifndef ORB_WINDOW_H
#define ORB_WINDOW_H

#include "Window.h"
#include "OpenGLContext.h"
#include "OrbInput.h"

#define NOMINMAX
#include <ShellApi.h>

class OrbWindow : public MainWindow
{
public:
	explicit OrbWindow();
	~OrbWindow();

	void open(const wchar_t *title, int width, int height);
	void flipGL();

	vec2i getSize() const
	{ return mSize; }

	OrbInput input;
protected:
	virtual LRESULT handleMessage(UINT msg, WPARAM wparam, LPARAM lparam);

private:
	void initViewport();
	void handleDropFiles(HDROP dropHandle);

	vec2i mSize;

	OpenGLContext mOpenGLContext;
	WindowRenderTarget mOpenGLTarget;
};


#endif
