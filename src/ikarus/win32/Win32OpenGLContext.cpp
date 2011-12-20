#include "Global.h"
#include "Win32OpenGLContext.h"
#include "Win32Error.h"


/// --- OpenGLContext ---------------------------------------------------------

OpenGLContext::OpenGLContext(): mTarget(0), mContext(0)
{
}

OpenGLContext::~OpenGLContext()
{
	close();
}

void OpenGLContext::open(RenderTarget *t)
{
	mTarget = t;
	open(mTarget->getDevice());
	bind(mTarget);
}

void OpenGLContext::open(HDC baseDevice)
{
	mContext = wglCreateContext(baseDevice);
}

void OpenGLContext::close()
{
	wglMakeCurrent(0, 0);
	mTarget = 0;
	if (mContext)
	{
		wglDeleteContext(mContext);
		mContext = 0;
	}
}

void OpenGLContext::bind()
{
	wglMakeCurrent(mTarget->getDevice(), mContext);
}

void OpenGLContext::bind(RenderTarget *t)
{
	mTarget = t;
	bind();
}

void OpenGLContext::swapBuffers() const
{
	SwapBuffers(mTarget->getDevice());
}

void OpenGLContext::Share(OpenGLContext &a, OpenGLContext &b)
{
	wglShareLists(a.mContext, b.mContext);
}

/// --- RenderTarget ----------------------------------------------------------

RenderTarget::RenderTarget()
{
}

RenderTarget::~RenderTarget()
{
}

/// --- WindowRenderTarget ----------------------------------------------------

WindowRenderTarget::WindowRenderTarget()
{
}

WindowRenderTarget::~WindowRenderTarget()
{
}

void WindowRenderTarget::open(HWND wnd, int depthBufferBitDepth)
{
	mWindow = wnd;
	mDevice = GetDC(wnd);

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1, // version of PIXELFORMATDESCRIPTOR
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24, // 24-bit colour (excluding alpha)
		0, 0, 0, 0, 0, 0, 0, 0, // details of the RGBA format are ignored
		0, 0, 0, 0, 0, // no accumulation buffer
		depthBufferBitDepth,
		0,  // no stencil buffer
		0,  // no aux. buffers (they aren't supported anyway)
		PFD_MAIN_PLANE,
		0, 0, 0, 0 // ignored or reserved items
	};

	int format_id = ChoosePixelFormat(mDevice, &pfd);
	if (format_id == 0)
		throw Win32Error("Could not find pixel format with our requirements.");
	if (SetPixelFormat(mDevice, format_id, &pfd) == FALSE)
		throw Win32Error("Could not set pixel format.");

#if 0
	const int fmt_attribs[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, 1,
		WGL_SUPPORT_OPENGL_ARB, 1,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_RED_BITS_ARB, 8,
		WGL_GREEN_BITS_ARB, 8,
		WGL_BLUE_BITS_ARB, 8,
		WGL_ALPHA_BITS_ARB, 8,
		0, 0
	};

	int fmt;
	UINT num_fmts;

	wglChoosePixelFormatARB(device, fmt_attribs, 0, 1, &fmt, &num_fmts);
	SetPixelFormat(device, fmt, 0);
#endif
}

void WindowRenderTarget::close()
{
	if (mDevice)
	{
		ReleaseDC(mWindow, mDevice);
		mDevice = 0;
	}
	mWindow = 0;
}

HDC WindowRenderTarget::getDevice() const
{
	return mDevice;
}
