#ifndef ORB_WIN32_H
#define ORB_WIN32_H

#include "Win32Window.h"
#include "Win32OpenGLContext.h"
#include "OrbInput.h"

#define NOMINMAX
#include <ShellApi.h>

class OrbWindow : public MainWindow
{
public:
	explicit OrbWindow();
	~OrbWindow();

	bool processEvents();
	void open(const wchar_t *title, int width, int height);
	void flipGL();

	OrbInput input;
protected:
	virtual LRESULT handleMessage(UINT msg, WPARAM wparam, LPARAM lparam);

private:
	void handleSize(int x, int y);
	void handleDropFiles(HDROP dropHandle);

	OpenGLContext mOpenGLContext;
	WindowRenderTarget mOpenGLTarget;
};


#endif
