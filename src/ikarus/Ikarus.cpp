#include "Global.h"
#include "OrbWindow.h"
#include "OrbGui.h"

#include "Camera.h"
#include "SkeletonDisplay.h"

#include "Font.h"
#include "Skeleton.h"

TextRenderer *gTextRenderer = 0;
Font *gFont = 0;

GLuint gridList;

void renderGrid(int N, double m)
{
	double b = m/2.0, a = -b;
	double xd = m / (double)N;
	double x;

	glColor3f(0.9f, 0.5f, 0.5f);
	glBegin(GL_LINES);
	x = a;
	for (int i = 0; i <= N; ++i, x += xd)
	{	glVertex3d(a, 0.0, x); glVertex3d(a, b, x); }// y/z plane (left); lines bottom-to-top
	x = a;
	for (int i = 0; i <= N/2; ++i, x += xd)
	{	glVertex3d(a, x+b, a); glVertex3d(a, x+b, b); }// y/z plane (left); lines back-to-front
	glEnd();

	glColor3f(0.5f, 0.9f, 0.5f);
	glBegin(GL_LINES);
	x = a;
	for (int i = 0; i <= N; ++i, x += xd)
	{	glVertex3d(x, 0.0, a); glVertex3d(x, b, a); }// x/y plane (back); lines bottom-to-top

	x = a;
	for (int i = 0; i <= N/2; ++i, x += xd)
	{	glVertex3d(a, x+b, a); glVertex3d(b, x+b, a); }// x/y plane (back); lines left-to-right
	glEnd();

	glColor3f(0.5f, 0.5f, 0.9f);
	glBegin(GL_LINES);
	x = a;
	for (int i = 0; i <= N; ++i, x += xd)
	{
		// x/z plane (bottom)
		glVertex3d(a, 0.0, x); glVertex3d(b, 0.0, x); // lines left-to-right
		glVertex3d(x, 0.0, a); glVertex3d(x, 0.0, b); // lines back-to-front
	}
	glEnd();
}

void initGL()
{
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(0.75f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	gridList = glGenLists(1);
	glNewList(gridList, GL_COMPILE);
	renderGrid(GridCount, GridWidth);
	glEndList();
}

void renderGui(OrbGui &gui, Camera &camPerspective, Camera &camX, Camera &camY, Camera &camZ, Skeleton &skel)
{
	// GUI state
	static bool showControls = true;
	static double sliderVal = 50.0;

	vec2i wndSize = gui.input->getWindowSize();

	// set up the default projection & modelview matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (double)wndSize.x, (double)wndSize.y, 0.0, 1.0, -1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	ColumnLayout lyt(FixedLayout(10, 10, 200, wndSize.y), 10, 10, 10, 10, 3);

	Label("title", "MONKEY").run(gui, lyt);
	showControls = CheckBox("show", "Show Controls", showControls).run(gui, lyt);

	if (showControls)
	{
		Label("hello", "Hello, world!").run(gui, lyt);
		sliderVal = Slider("value", 0.0, 100.0, 10.0, sliderVal, true).run(gui, lyt);
		std::ostringstream ss;
		ss << sliderVal;
		Label("value-lbl", ss.str()).run(gui, lyt);
		if (Button("hide-btn", "Hide").run(gui, lyt))
			showControls = false;
	}

	int leftRightSplit = 250;
	int topBottomSplit = wndSize.y - 200;
	int a = leftRightSplit + (wndSize.x - leftRightSplit) / 3;
	int b = leftRightSplit + ((wndSize.x - leftRightSplit)*2) / 3;

	SkeletonDisplay("displayP", &camPerspective, &skel, gridList).run(gui, FixedLayout(leftRightSplit, 0, wndSize.x - leftRightSplit, topBottomSplit));
	SkeletonDisplay("displayX", &camX, &skel).run(gui, FixedLayout(leftRightSplit, topBottomSplit, a - leftRightSplit, wndSize.y - topBottomSplit));
	SkeletonDisplay("displayY", &camY, &skel).run(gui, FixedLayout(a, topBottomSplit, b - a, wndSize.y - topBottomSplit));
	SkeletonDisplay("displayZ", &camZ, &skel).run(gui, FixedLayout(b, topBottomSplit, wndSize.x - b, wndSize.y - topBottomSplit));
}

#ifdef _WIN32
int APIENTRY wWinMain(HINSTANCE hPrevInstance, HINSTANCE hInstance, LPWSTR cmdLine, int nShowCmd)
#else
int main(int argc, char *argv[])
#endif
{
	int retval = 0;

	try
	{
		OrbWindow wnd;
		wnd.open(L"Ikarus", 1024, 768);

		// initialize GLEW and set up OpenGL
		glewInit();
		initGL();

		// load the skeleton
		Skeleton skel;
		skel.loadFromFile("human.skl");

		// load the default font
		Font font;
		//font.loadFromFile("arial-rounded-18.fnt");
		font.loadFromFile("ms-sans-serif-13.fnt");
		gFont = &font;
		TextRenderer textRenderer;
		gTextRenderer = &textRenderer;
		
		OrbGui gui(&wnd.input, gFont, gTextRenderer);

		// set up the cameras
		CameraAzimuthElevation cameraPerspective;
		CameraOrtho cameraX(0);
		CameraOrtho cameraY(1);
		CameraOrtho cameraZ(2);

		// pick the default camera
		Camera *cam = &cameraPerspective;

		wnd.input.beginFrame();
		while (true)
		{
			wnd.input.beginFrame();

			if (! Window::ProcessWaitingMessages(&retval))
				break;

			if (wnd.input.wasKeyPressed(KeyCode::Escape))
				break;

			glClear(GL_COLOR_BUFFER_BIT);
			renderGui(gui, cameraPerspective, cameraX, cameraY, cameraZ, skel);
			wnd.flipGL();
		}

		{
#if 0
			cam->update();
			render(skel, *cam);

			glfwSwapBuffers();

			if (glfwGetKey(GLFW_KEY_KP_7))
				cam = &cameraY;
			if (glfwGetKey(GLFW_KEY_KP_1))
				cam = &cameraZ;
			if (glfwGetKey(GLFW_KEY_KP_3))
				cam = &cameraX;
			if (glfwGetKey(GLFW_KEY_KP_5))
				cam = &cameraPerspective;

			vec3d delta(0.0, 0.0, 0.0);
			if (glfwGetKey('W'))
				delta.z -= 1.0;
			if (glfwGetKey('S'))
				delta.z += 1.0;
			if (glfwGetKey('A'))
				delta.x -= 1.0;
			if (glfwGetKey('D'))
				delta.x += 1.0;
			if (glfwGetKey('Q'))
				delta.y += 1.0;
			if (glfwGetKey('Z'))
				delta.y -= 1.0;
#endif

#if 0
			if (delta.x != 0.0 || delta.y != 0.0 || delta.z != 0.0)
				skel.targetPos += normalize(delta) * MoveStep;

			if (skel.targetPos.x > GridWidth/2.0) skel.targetPos.x = GridWidth/2.0;
			if (skel.targetPos.x < -GridWidth/2.0) skel.targetPos.x = -GridWidth/2.0;

			if (skel.targetPos.y > GridWidth/2.0) skel.targetPos.y = GridWidth/2.0;
			if (skel.targetPos.y < 0.0) skel.targetPos.y = 0.0;
			
			if (skel.targetPos.z > GridWidth/2.0) skel.targetPos.z = GridWidth/2.0;
			if (skel.targetPos.z < -GridWidth/2.0) skel.targetPos.z = -GridWidth/2.0;
#endif
			
#if 0
			if (glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
				break;
#endif
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		retval = 1;
	}
	catch (...)
	{
		std::cerr << "Unknown exception." << std::endl;
		retval = 1;
	}

	return retval;
}
