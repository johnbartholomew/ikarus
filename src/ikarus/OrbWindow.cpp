#include "Global.h"
#include "OrbWindow.h"
#include "resources.h"

const wchar_t *kOrbWindowClass = L"OrbWndCls";

OrbWindow::OrbWindow()
{
}

OrbWindow::~OrbWindow()
{
}

void OrbWindow::open(const wchar_t *title, int width, int height)
{
	HINSTANCE inst = GetModuleHandle(NULL);
	MainWindow::open(kOrbWindowClass, title, LoadIcon(inst, MAKEINTRESOURCE(ICO_MAIN)), width, height);

	mOpenGLTarget.open(getHandle());
	mOpenGLContext.open(&mOpenGLTarget);
}

void OrbWindow::flipGL()
{
	mOpenGLContext.swapBuffers();
}

void OrbWindow::initViewport()
{
	glViewport(0, 0, mSize.x, mSize.y);
}

void OrbWindow::handleDropFiles(HDROP dropHandle)
{
	int numFiles = DragQueryFileW(dropHandle, 0xffffffff, 0, 0);

	if (numFiles == 0)
		return;

	// nb: because we can only display one image at a time,
	// we just take the first file from the list and ignore the others
	int len = DragQueryFileW(dropHandle, 0, 0, 0);
	std::wstring fileName(len, '_');
	int lenWritten = DragQueryFileW(dropHandle, 0, &fileName[0], len+1);
	assert(lenWritten == len);

	// just in case it gives a lower count when you actually get the name,
	// (eg, if it always gives a default maximum or an estimate when you ask for the required size),
	// resize the std::wstring to ensure its stored size and null-terminator will match the real length of the file path
	fileName.resize(lenWritten);

	// HACK: technically, we shouldn't ram characters straight into a std::wstring,
	// we should allocate a separate buffer of a known length to give to DragQueryFileW(),
	// and then construct a std::wstring from that buffer.
	// but that totally pointless and unnecessary extra allocation and copy
	// gets on my nerves even more than a little type-unsafety

	std::wstring fileExt = fileName.substr(fileName.find_last_of('.'));
	std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), &::tolower);
	
	// do something with the file, based on the extension
}

int MessageToMouseButton(UINT msg, WPARAM wparam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN: return MouseButton::Left;
	case WM_RBUTTONDOWN: return MouseButton::Right;
	case WM_MBUTTONDOWN: return MouseButton::Middle;
	case WM_XBUTTONDOWN:
		if (HIWORD(wparam) == XBUTTON1)
			return MouseButton::X1;
		else if (HIWORD(wparam) == XBUTTON2)
			return MouseButton::X2;
		else
			return -1;
	default:
		return -1;
	}
}

LRESULT OrbWindow::handleMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DROPFILES:
		{
			HDROP dropHandle = reinterpret_cast<HDROP>(wparam);
			handleDropFiles(dropHandle);
			DragFinish(dropHandle);
		}
		return 0;
	case WM_CREATE:
		{
			CREATESTRUCT *info = reinterpret_cast<CREATESTRUCT*>(lparam);
			mSize = vec2i(info->cx, info->cy);
			initViewport();
		}
		return MainWindow::handleMessage(msg, wparam, lparam);
	case WM_ERASEBKGND:
		// don't bother with erasing the background
		return 1;
	case WM_SIZE:
		{
			mSize = vec2i(
				static_cast<signed short>(LOWORD(lparam)),
				static_cast<signed short>(HIWORD(lparam))
			);
			initViewport();
		}
		return 0;

	// mouse input handling
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		{
			SetCapture(getHandle());

			int btn = MessageToMouseButton(msg, wparam);
			if (btn != -1)
			{
				input.mousePress(
					btn,
					static_cast<signed short>(LOWORD(lparam)),
					static_cast<signed short>(HIWORD(lparam))
				);
			}
		}
		return 0;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		{
			ReleaseCapture();

			int btn = MessageToMouseButton(msg, wparam);
			if (btn != -1)
			{
				input.mouseRelease(
					btn,
					static_cast<signed short>(LOWORD(lparam)),
					static_cast<signed short>(HIWORD(lparam))
				);
			}
		}
		return 0;

	case WM_MOUSEMOVE:
		{
			input.mouseMove(
				static_cast<signed short>(LOWORD(lparam)),
				static_cast<signed short>(HIWORD(lparam))
			);
		}
		return 0;

	case WM_MOUSEWHEEL:
		{
			input.mouseScroll(static_cast<signed short>(HIWORD(wparam)));
		}
		return 0;

	// keyboard handling
	case WM_KEYDOWN:
		{
			input.keyPress(static_cast<int>(wparam));
		}
		return 0;
	case WM_KEYUP:
		{
			input.keyRelease(static_cast<int>(wparam));
		}
		return 0;

	// pass anything else back to the MainWindow class
	default:
		return MainWindow::handleMessage(msg, wparam, lparam);
	}
}
