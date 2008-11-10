#include "Global.h"
#include "OrbWindow.h"
#include "OrbGui.h"

#include "Camera.h"
#include "SkeletonDisplay.h"

#include "Font.h"
#include "Skeleton.h"
//#include "Pose.h"
#include "IkSolver.h"

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

class Ikarus
{
public:
	Ikarus()
	:	camX(0), camY(1), camZ(2), targetSpeed(0.0), targetPos(0.0, 0.0, 0.0)
	{
		skel.loadFromFile("human.skl");
		solver.reset(new IkSolver(skel));

		targetPos = solver->getTargetPos();
	}

	void run(OrbGui &gui)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		updateTargetPos(gui);
		solver->iterateIk();
		runGui(gui);
	}

	void runGui(OrbGui &gui)
	{
		// GUI state
		vec2i wndSize = gui.input->getWindowSize();

		// set up the default projection & modelview matrices
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, (double)wndSize.x, (double)wndSize.y, 0.0, 1.0, -1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		ColumnLayout lyt(FixedLayout(10, 10, 200, wndSize.y), 10, 10, 10, 10, 3);

		Label("title", "Ikarus").run(gui, lyt);

		if (Button("reset-btn", "Reset").run(gui, lyt))
		{
			solver->resetAll();
			targetPos = solver->getTargetPos();
			targetSpeed = 0.0;
		}

		int leftRightSplit = 250;
		int topBottomSplit = wndSize.y - 200;
		int a = leftRightSplit + (wndSize.x - leftRightSplit) / 3;
		int b = leftRightSplit + ((wndSize.x - leftRightSplit)*2) / 3;

		IkSolverDisplay("displayP", &camPerspective, solver.get(), gridList).run(gui, FixedLayout(leftRightSplit, 0, wndSize.x - leftRightSplit, topBottomSplit));
		IkSolverDisplay("displayX", &camX, solver.get()).run(gui, FixedLayout(leftRightSplit, topBottomSplit, a - leftRightSplit, wndSize.y - topBottomSplit));
		IkSolverDisplay("displayY", &camY, solver.get()).run(gui, FixedLayout(a, topBottomSplit, b - a, wndSize.y - topBottomSplit));
		IkSolverDisplay("displayZ", &camZ, solver.get()).run(gui, FixedLayout(b, topBottomSplit, wndSize.x - b, wndSize.y - topBottomSplit));
	}

	void updateTargetPos(OrbGui &gui)
	{
		// update the target pos...
		vec3d delta(0.0, 0.0, 0.0);
		if (gui.input->isKeyDown('W')) delta.z -= 1.0;
		if (gui.input->isKeyDown('S')) delta.z += 1.0;
		if (gui.input->isKeyDown('A')) delta.x -= 1.0;
		if (gui.input->isKeyDown('D')) delta.x += 1.0;
		if (gui.input->isKeyDown('Q')) delta.y += 1.0;
		if (gui.input->isKeyDown('Z')) delta.y -= 1.0;

		if (dot(delta,delta) > 0.0)
		{
			targetSpeed += 0.05;
			if (targetSpeed > MoveStep)
				targetSpeed = MoveStep;
		}
		else
			targetSpeed = 0.0;

		if (targetSpeed > 0.0)
			targetPos += normalize(delta) * targetSpeed;

		if (targetPos.x > GridWidth/2.0) targetPos.x = GridWidth/2.0;
		if (targetPos.x < -GridWidth/2.0) targetPos.x = -GridWidth/2.0;

		if (targetPos.y > GridWidth/2.0) targetPos.y = GridWidth/2.0;
		if (targetPos.y < 0.0) targetPos.y = 0.0;
		
		if (targetPos.z > GridWidth/2.0) targetPos.z = GridWidth/2.0;
		if (targetPos.z < -GridWidth/2.0) targetPos.z = -GridWidth/2.0;

		solver->setTargetPos(targetPos);
	}

private:
	CameraAzimuthElevation camPerspective;
	CameraOrtho camX;
	CameraOrtho camY;
	CameraOrtho camZ;
	Skeleton skel;

	ScopedPtr<IkSolver> solver;
	double targetSpeed;
	vec3d targetPos;
};

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

		// load the default font
		Font font;
		//font.loadFromFile("arial-rounded-18.fnt");
		font.loadFromFile("ms-sans-serif-13.fnt");
		gFont = &font;
		TextRenderer textRenderer;
		gTextRenderer = &textRenderer;
		
		OrbGui gui(&wnd.input, gFont, gTextRenderer);
		Ikarus ikarus;

		wnd.input.beginFrame();
		while (true)
		{
			wnd.input.beginFrame();

			if (! Window::ProcessWaitingMessages(&retval))
				break;

			if (wnd.input.wasKeyPressed(KeyCode::Escape))
				break;

			ikarus.run(gui);

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
