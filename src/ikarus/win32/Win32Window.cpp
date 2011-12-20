#include "Global.h"
#include "Win32Window.h"

const DWORD kWindowStylesEx = WS_EX_ACCEPTFILES;
const DWORD kWindowStyles   = WS_OVERLAPPEDWINDOW;

// ===== Window ==============================================================

Window::Window()
:	mHandle(0)
{}

Window::Window(const wchar_t *wnd_class_name, const wchar_t *title, HICON icon, int width, int height, int left, int top, bool visible)
:	mHandle(0)
{
	open(wnd_class_name, title, icon, width, height, left, top, visible);
}

Window::Window(const wchar_t *wnd_class_name, const wchar_t *title, HICON icon, int width, int height, bool visible)
:	mHandle(0)
{
	open(wnd_class_name, title, icon, width, height, visible);
}

Window::~Window()
{
	close();
}

void Window::open(const wchar_t *wnd_class_name, const wchar_t *title, HICON icon, int width, int height, bool visible)
{
	open(wnd_class_name, title, icon, width, height, CW_USEDEFAULT, CW_USEDEFAULT, visible);
}

void Window::open(const wchar_t *wnd_class_name, const wchar_t *title, HICON icon, int width, int height, int left, int top, bool visible)
{
	assert(mHandle == 0);

	HMODULE inst = GetModuleHandle(NULL);

	WNDCLASSEX wndClsDef =
	{
		sizeof(wndClsDef),
		0,                            // no special styles
		&Window::wndProc,
		0,                            // no extra class data storage
		0,                            // no extra window data storage
		inst,
		icon,
		LoadCursor(0, IDC_ARROW),
		(HBRUSH)(COLOR_APPWORKSPACE+1), // +1 is just part of the spec (don't ask)
		NULL,                         // no default main menu
		wnd_class_name,
		NULL                          // no special small icon (use the main icon file, which should contain a small version)
	};
	ATOM wndCls = RegisterClassEx(&wndClsDef);
	if (wndCls == 0)
		throw Win32Error("Error in RegisterClassEx()");

	// if left == CW_USEDEFAULT then CreateWindowEx ignores top,
	// so if top is *not* CW_USEDEFAULT and left *is*,
	// then we set our own left value so that the top value will be used (crazy)
	if (left == CW_USEDEFAULT && top != CW_USEDEFAULT)
	{
		RECT workArea = {0,0,800,600};
		if (! SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0))
			throw Win32Error("Error in SystemParametersInfo()");
	}

	RECT bounds = {0, 0, width, height};
	if (! AdjustWindowRectEx(&bounds, kWindowStyles, FALSE, kWindowStylesEx))
		throw Win32Error("Error in AdjustWindowRectEx()");

	DWORD styles = kWindowStyles;
	if (visible)
		styles |= WS_VISIBLE;

	int w = bounds.right - bounds.left, h = bounds.bottom - bounds.top;

	HWND handle = CreateWindowEx(
		kWindowStylesEx, ((LPCWSTR)wndCls), title, styles,
		left, top, w, h,
		0,       // no parent window (this is a top-level window)
		0,       // no main menu
		inst,
		this
	);
	if (handle == 0)
		throw Win32Error("Error in CreateWindowEx()");

	// mHandle is set in the WM_NCCREATE handler	
	assert(mHandle == handle);
}

void Window::show(bool visible)
{
	assert(mHandle != 0);
	ShowWindow(mHandle, visible ? SW_SHOW : SW_HIDE);
}

void Window::hide()
{
	assert(mHandle != 0);
	show(false);
}

void Window::invalidate()
{
	if (!InvalidateRect(getHandle(), 0, TRUE))
		throw Win32Error("Error in InvalidateRect()");
}

void Window::close()
{
	if (mHandle)
		DestroyWindow(mHandle);
}

int Window::RunGlobalLoop()
{
	MSG msg;
	int ret;
	while (ret = GetMessage(&msg, 0, 0, 0))
	{
		if (ret == -1)
			throw Win32Error("Error in GetMessage()");
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_QUIT)
		return static_cast<int>(msg.wParam);
	else
		return 0;
}

bool Window::ProcessWaitingMessages(int *quitcode)
{
	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			if (quitcode)
				*quitcode = msg.wParam;
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

LRESULT Window::handleMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
		close();
		return 0;
	case WM_DESTROY:
		mHandle = 0;
		return 0;
	default:
		return DefWindowProc(mHandle, msg, wparam, lparam);
	}
}

LRESULT CALLBACK Window::wndProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	Window *wnd;
	
#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4312)
	// disable pointer <-> LONG conversion warnings
	// (irrelevant because on a 64-bit platform it won't perform that conversion anyway,
	// because Get/SetWindowLongPtr are defined to take/return LONG_PTR on 64-bit, and LONG on 32-bit)

	if (msg == WM_NCCREATE)
	{
		CREATESTRUCT *info = reinterpret_cast<CREATESTRUCT*>(lparam);
		wnd = reinterpret_cast<Window*>(info->lpCreateParams);
		SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd));
		wnd->mHandle = handle;
	}
	else
		wnd = reinterpret_cast<Window*>(GetWindowLongPtr(handle, GWLP_USERDATA));
#pragma warning(pop)

	// if we get any other messages before the window pointer has been set up,
	// then we won't know what to do with them anyway, so just pass them on to the default handler
	if ((wnd == 0) || (handle != wnd->mHandle))
		return DefWindowProc(handle, msg, wparam, lparam);
	else
		return wnd->handleMessage(msg, wparam, lparam);
}

// ===== MainWindow ==========================================================

LRESULT MainWindow::handleMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return Window::handleMessage(msg, wparam, lparam);
}
