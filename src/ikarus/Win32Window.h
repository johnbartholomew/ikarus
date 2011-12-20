#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H

#include "Win32Error.h"
#define NOMINMAX
#include <Windows.h>

class Window
{
public:
	explicit Window();
	explicit Window(
		const wchar_t *wnd_class_name,
		const wchar_t *title,
		HICON icon,
		int width,
		int height,
		bool visible = true
	);
	explicit Window(
		const wchar_t *wnd_class_name,
		const wchar_t *title,
		HICON icon,
		int width,
		int height,
		int left,
		int top,
		bool visible = true
	);
	virtual ~Window();

	void open(
		const wchar_t *wnd_class_name,
		const wchar_t *title,
		HICON icon,
		int width,
		int height,
		bool visible = true
	);
	void open(
		const wchar_t *wnd_class_name,
		const wchar_t *title,
		HICON icon,
		int width,
		int height,
		int left,
		int top,
		bool visible = true
	);

	void show(bool visible = true);
	void hide();
	void invalidate();
	void close();

	HWND getHandle() const { return mHandle; }

	static int RunGlobalLoop();
	static bool ProcessWaitingMessages(int *quitcode = 0);

protected:
	virtual LRESULT handleMessage(UINT msg, WPARAM wparam, LPARAM lparam);

private:
	Window(const Window &); // non-copyable
	Window &operator=(const Window &); // non-assignable

	HWND mHandle;

	static LRESULT CALLBACK wndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

class MainWindow : public Window
{
public:
	explicit MainWindow(): Window() {}
	explicit MainWindow(const wchar_t *wnd_class_name, const wchar_t *title, HICON icon, int width, int height, bool visible)
		: Window(wnd_class_name, title, icon, width, height, visible) {}
	explicit MainWindow(const wchar_t *wnd_class_name, const wchar_t *title, HICON icon, int width, int height, int left = CW_USEDEFAULT, int top = CW_USEDEFAULT, bool visible = true)
		: Window(wnd_class_name, title, icon, width, height, left, top, visible) {}
protected:
	virtual LRESULT handleMessage(UINT msg, WPARAM wparam, LPARAM lparam);
};

#endif
