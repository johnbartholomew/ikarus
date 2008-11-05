#pragma once

#include "Window.h"
#include <windows.h>

class WndDC
{
public:
	explicit WndDC()
	: mDC(0) {}

	explicit WndDC(HWND wnd)
	: mWnd(wnd), mDC(GetDC(wnd)) {}
	
	explicit WndDC(const Window &wnd)
	: mWnd(wnd.getHandle()), mDC(GetDC(wnd.getHandle())) {}

	~WndDC()
	{ ReleaseDC(mWnd, mDC); }

	void reset(const Window &wnd)
	{ reset(wnd.getHandle()); }

	void reset(HWND wnd = 0)
	{ WndDC(wnd).swap(*this); }

	HDC get() const
	{ return mDC; }
private:
	WndDC(const WndDC &); // non-copyable
	WndDC &operator=(const WndDC &); // non-assignable

	void swap(WndDC &b)
	{
		std::swap(mDC, b.mDC);
		std::swap(mWnd, b.mWnd);
	}

	HDC mDC;
	HWND mWnd;
};
