#ifndef ORB_SDL_H
#define ORB_SDL_H

#include "OrbInput.h"

struct SDL_Surface;

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
	void handleSize(int x, int y);
};


#endif
