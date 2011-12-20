#include "Global.h"
#include "OrbSDL.h"
#include <SDL/SDL.h>

static bool have_orb_window = false;

OrbWindow::OrbWindow()
{
	assert(!have_orb_window);
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw std::runtime_error(SDL_GetError());
	}
	have_orb_window = true;
}

OrbWindow::~OrbWindow()
{
	assert(have_orb_window);
	SDL_Quit();
	have_orb_window = false;
}

bool OrbWindow::processEvents()
{
	SDL_PumpEvents();
	SDL_Event event;
	bool do_quit = false;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				do_quit = true;
				break;
			case SDL_VIDEORESIZE:
				handleSize(event.resize.w, event.resize.h);
				break;
			case SDL_KEYDOWN:
				input.keyPress(event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				input.keyRelease(event.key.keysym.sym);
				break;
			case SDL_MOUSEMOTION:
				input.mouseMove(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button)
				{
					case SDL_BUTTON_WHEELUP: input.mouseScroll(WHEEL_DELTA); break;
					case SDL_BUTTON_WHEELDOWN: input.mouseScroll(-WHEEL_DELTA); break;
					case SDL_BUTTON_LEFT: input.mousePress(MouseButton::Left); break;
					case SDL_BUTTON_RIGHT: input.mousePress(MouseButton::Right); break;
					case SDL_BUTTON_MIDDLE: input.mousePress(MouseButton::Middle); break;
					case SDL_BUTTON_X1: input.mousePress(MouseButton::X1); break;
					case SDL_BUTTON_X2: input.mousePress(MouseButton::X2); break;
					default: assert(0); break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				switch (event.button.button)
				{
					// wheel motion is handled in the BUTTONDOWN event
					case SDL_BUTTON_WHEELUP: break;
					case SDL_BUTTON_WHEELDOWN: break;
					case SDL_BUTTON_LEFT: input.mouseRelease(MouseButton::Left); break;
					case SDL_BUTTON_RIGHT: input.mouseRelease(MouseButton::Right); break;
					case SDL_BUTTON_MIDDLE: input.mouseRelease(MouseButton::Middle); break;
					case SDL_BUTTON_X1: input.mouseRelease(MouseButton::X1); break;
					case SDL_BUTTON_X2: input.mouseRelease(MouseButton::X2); break;
					default: assert(0); break;
				}
				break;
			default: break;
		}
	}
	return !do_quit;
}

void OrbWindow::open(const wchar_t *title, int width, int height)
{
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
	SDL_Surface *video = SDL_SetVideoMode(width, height, 0, SDL_OPENGL | SDL_RESIZABLE | SDL_DOUBLEBUF);
	if (!video) {
		throw std::runtime_error(SDL_GetError());
	}
	input.windowResize(width, height);
}

void OrbWindow::flipGL()
{
	SDL_GL_SwapBuffers();
}

void OrbWindow::handleSize(int x, int y)
{
	input.windowResize(x, y);
	glViewport(0, 0, x, y);
}
