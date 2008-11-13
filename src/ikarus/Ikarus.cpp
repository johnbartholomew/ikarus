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

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

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
	:	camX(0), camY(1), camZ(2),
		targetSpeed(0.0),
		curSkel(0),
		ikMode(true),
		ikEnabled(false),
		showJointBasis(false),
		showConstraints(true)
	{
		skeletons.push_back(new SkeletonItem("simple.skl", "Simple"));
		skeletons.push_back(new SkeletonItem("snake.skl", "Snake"));
		skeletons.push_back(new SkeletonItem("human.skl", "Human"));
	}

	void run(OrbGui &gui)
	{
		SkeletonItem &skel = skeletons[curSkel];

		if (ikMode)
		{
			updateTargetPos(gui);
			if (ikEnabled)
				skel.solver->iterateIk();
		}
		runGui(gui);
	}

	void runGui(OrbGui &gui)
	{
		// GUI state
		vec2i wndSize = gui.input->getWindowSize();

		// set up the default projection & modelview matrices
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, (double)wndSize.x, (double)wndSize.y, 0.0, 10.0, -10.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		ColumnLayout lyt(FixedLayout(10, 10, 200, wndSize.y), 10, 10, 10, 10, 3);

		Label("Ikarus").run(gui, lyt);

		Spacer(vec2i(0, 10)).run(gui, lyt);

		Label("Skeleton:").run(gui, lyt);
		ComboBox skelSel("skeleton-sel", WidgetID(curSkel));
		for (int i = 0; i < (int)skeletons.size(); ++i)
			skelSel.add(WidgetID(i), skeletons[i].name);
		curSkel = skelSel.run(gui, lyt).getIndex();

		if (Button("reload-btn", "Reload").run(gui, lyt))
			skeletons.reset_at(curSkel, new SkeletonItem(skeletons[curSkel].fname, skeletons[curSkel].name));
		
		SkeletonItem &skel = skeletons[curSkel];

		Spacer(vec2i(0, 10)).run(gui, lyt);

		if (Button("reset-btn", "Reset Pose").run(gui, lyt))
		{
			skel.solver->resetPose();
			skel.targetPos = skel.solver->getEffectorPos();
			targetSpeed = 0.0;
		}

		if (Button("reset-all-btn", "Reset All").run(gui, lyt))
		{
			skel.solver->resetAll();
			skel.targetPos = skel.solver->getEffectorPos();
			targetSpeed = 0.0;
		}

		showJointBasis = CheckBox("show-joint-basis-chk", "Show joint basis vectors", showJointBasis).run(gui, lyt);
		showConstraints = CheckBox("show-constraints-chk", "Show joint constraints", showConstraints).run(gui, lyt);

		ikMode = CheckBox("ik-mode-chk", "IK Mode", ikMode).run(gui, lyt);
		ikEnabled = CheckBox("ik-enabled-chk", "IK Enabled", ikEnabled, ikMode).run(gui, lyt);
		bool constraintsOn = CheckBox("ik-constrained-chk", "Enable Constraints", skel.solver->areConstraintsEnabled(), ikMode).run(gui, lyt);
		skel.solver->enableConstraints(constraintsOn);

		if (Button("solve-btn", "Solve", ikMode && !ikEnabled).run(gui, lyt))
			skel.solver->solveIk(30);

		if (Button("step-btn", "Step IK", ikMode && !ikEnabled).run(gui, lyt))
			skel.solver->iterateIk();

		if (Button("constraint-btn", "Apply Constraints", ikMode).run(gui, lyt))
		{
			skel.solver->applyAllConstraints();
			skel.targetPos = skel.solver->getEffectorPos();
			targetSpeed = 0.0;
		}

		Label("Root bone:").run(gui, lyt);
		ComboBox rootSel("root-sel", WidgetID(&skel.solver->getRootBone()));
		for (int i = 0; i < skel.skeleton.numBones(); ++i)
		{
			const Bone &b = skel.skeleton[i];
			if (! b.isEffector())
				rootSel.add(WidgetID(&b), b.name);
		}
		const Bone *newRootBone = rootSel.run(gui, lyt).getData<const Bone>();
		skel.solver->setRootBone(*newRootBone);

		Label("Effector:").run(gui, lyt);
		ComboBox effectorSel("effector-sel", WidgetID(&skel.solver->getEffector()));
		for (int i = 0; i < skel.skeleton.numBones(); ++i)
		{
			const Bone &b = skel.skeleton[i];
			if (b.isEffector())
				effectorSel.add(WidgetID(&b), b.name);
		}
		const Bone *newEffector = effectorSel.run(gui, lyt).getData<const Bone>();
		if (newEffector != &skel.solver->getEffector())
		{
			skel.solver->setEffector(*newEffector);
			skel.solver->setTargetPos(skel.solver->getEffectorPos());
			skel.targetPos = skel.solver->getEffectorPos();
		}

		int leftRightSplit = 250;
		int topBottomSplit = wndSize.y - 200;
		int a = leftRightSplit + (wndSize.x - leftRightSplit) / 3;
		int b = leftRightSplit + ((wndSize.x - leftRightSplit)*2) / 3;

		FixedLayout mainViewLyt(leftRightSplit, 0, wndSize.x - leftRightSplit, topBottomSplit);
		FixedLayout ortho0Lyt(leftRightSplit, topBottomSplit, a - leftRightSplit, wndSize.y - topBottomSplit);
		FixedLayout ortho1Lyt(a, topBottomSplit, b - a, wndSize.y - topBottomSplit);
		FixedLayout ortho2Lyt(b, topBottomSplit, wndSize.x - b, wndSize.y - topBottomSplit);

		if (ikMode)
		{
			IkSolverDisplay("displayP", &camPerspective, skel.solver.get(), showJointBasis, showConstraints, gridList).run(gui, mainViewLyt);
			IkSolverDisplay("displayX", &camX, skel.solver.get(), showJointBasis, showConstraints).run(gui, ortho0Lyt);
			IkSolverDisplay("displayY", &camY, skel.solver.get(), showJointBasis, showConstraints).run(gui, ortho1Lyt);
			IkSolverDisplay("displayZ", &camZ, skel.solver.get(), showJointBasis, showConstraints).run(gui, ortho2Lyt);
		}
		else
		{
			SkeletonDisplay("displayP", &camPerspective, &skel.skeleton, showJointBasis, showConstraints, gridList).run(gui, mainViewLyt);
			SkeletonDisplay("displayX", &camX, &skel.skeleton, showJointBasis, showConstraints).run(gui, ortho0Lyt);
			SkeletonDisplay("displayY", &camY, &skel.skeleton, showJointBasis, showConstraints).run(gui, ortho1Lyt);
			SkeletonDisplay("displayZ", &camZ, &skel.skeleton, showJointBasis, showConstraints).run(gui, ortho2Lyt);
		}
	}

	void updateTargetPos(OrbGui &gui)
	{
		SkeletonItem &skel = skeletons[curSkel];

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
			skel.targetPos += normalize(delta) * targetSpeed;

		if (skel.targetPos.x > GridWidth/2.0) skel.targetPos.x = GridWidth/2.0;
		if (skel.targetPos.x < -GridWidth/2.0) skel.targetPos.x = -GridWidth/2.0;

		if (skel.targetPos.y > GridWidth/2.0) skel.targetPos.y = GridWidth/2.0;
		if (skel.targetPos.y < 0.0) skel.targetPos.y = 0.0;
		
		if (skel.targetPos.z > GridWidth/2.0) skel.targetPos.z = GridWidth/2.0;
		if (skel.targetPos.z < -GridWidth/2.0) skel.targetPos.z = -GridWidth/2.0;

		skel.solver->setTargetPos(skel.targetPos);
	}

private:
	struct SkeletonItem
	{
		SkeletonItem(const std::string &fname, const std::string &name)
			:	name(name), fname(fname)
		{
			skeleton.loadFromFile(fname.c_str());
			solver.reset(new IkSolver(skeleton));
			targetPos = solver->getTargetPos();
		}

		Skeleton skeleton;
		ScopedPtr<IkSolver> solver;
		vec3d targetPos;
		std::string name;
		std::string fname;
	};

	CameraAzimuthElevation camPerspective;
	CameraOrtho camX;
	CameraOrtho camY;
	CameraOrtho camZ;

	double targetSpeed;
	int curSkel;
	bool ikMode;
	bool ikEnabled;
	bool showJointBasis;
	bool showConstraints;

	refvector<SkeletonItem> skeletons;
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

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
