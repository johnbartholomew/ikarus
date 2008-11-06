#include "Global.h"
#include "OrbWindow.h"
#include "OrbGui.h"

#include "Font.h"
#include "Skeleton.h"

const double CameraDistance = 30.0;
const double Aspect = 4.0 / 3.0;
const double FoV = 40.0 * (M_PI/180.0);
const double zNear = 0.1;
const double zFar = 100.0;
const double CameraDistWheelScale = 1.1;

const int GridCount = 10;
const double GridWidth = 20.0;

const double MoveStep = 0.2;

double cameraAzimuth = 0.0;
double cameraElevation = 0.0;

TextRenderer *gTextRenderer = 0;
Font *gFont = 0;

GLuint gridList;

class Camera
{
public:
	virtual void update(const OrbInput &input, const recti &bounds) = 0;
	virtual void renderUI(const recti &bounds) const = 0;
	virtual mat4d getProjection(const recti &bounds) const = 0;
	virtual mat4d getModelView() const = 0;
};

class CameraOrtho : public Camera
{
public:
	CameraOrtho(int axis)
		:	axis(axis),
			scale(GridWidth/2.0) // nb: if you change this change update()
	{
	}

	virtual void update(const OrbInput &input, const recti &bounds)
	{
		int wheel = input.getMouseWheelPos();
		// nb: if you change this change the CameraOrtho initializer
		scale = (GridWidth/2.0) * std::pow(CameraDistWheelScale, -wheel);
	}

	virtual void renderUI(const recti &bounds) const {}

	virtual mat4d getProjection(const recti &bounds) const
	{
		double aspect = (double)bounds.size.x / (double)bounds.size.y;
		return vmath::ortho_matrix(-aspect, aspect, -1.0, 1.0, 0.1, 100.0);
	}

	virtual mat4d getModelView() const
	{
		mat4d m;
		if (axis == 0)
		{
			m = mat4d(
				0.0, 0.0, 1.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				1.0, 0.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 1.0
			);
		}
		else if (axis == 1)
		{
			m = mat4d(
				1.0, 0.0, 0.0, 0.0,
				0.0, 0.0, -1.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 1.0
			);
		}
		else if (axis == 2)
		{
			m = mat4d(
				1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0
			);
		}

		return vmath::scaling_matrix(1.0/scale, 1.0/scale, 1.0) * m * vmath::translation_matrix(0.0, -GridWidth/4.0, 0.0);
	}
private:
	double aspect;
	double scale;
	int axis;
};

class CameraAzimuthElevation : public Camera
{
public:
	CameraAzimuthElevation()
		:	dragging(false),
			cameraDist(CameraDistance),
			az(0.0), el(0.0), az0(0.0), el0(0.0)
	{
	}

	virtual void update(const OrbInput &input, const recti &bounds)
	{
		const vec2i screenCentre = bounds.topLeft + vec2i(bounds.size.x/2, bounds.size.y/2);
		const double screenRadius = std::min(bounds.size.x, bounds.size.y) / 2.0;

		int wheel = input.getMouseWheelPos();
		cameraDist = CameraDistance * std::pow(CameraDistWheelScale, -wheel);

		vec2i mousePos = input.getMousePos();
		if (input.isMouseDown(MouseButton::Left))
		{
			if (!dragging)
			{
				if (bounds.contains(mousePos))
				{
					dragging = true;
					startDrag(screenCentre, screenRadius, mousePos);
				}
			}
			else
				updateDrag(screenCentre, screenRadius, mousePos);
		}
		else
		{
			if (dragging)
			{
				updateDrag(screenCentre, screenRadius, mousePos);
				dragging = false;
			}
		}
	}

	virtual mat4d getProjection(const recti &bounds) const
	{
		double aspect = (double)bounds.size.x / (double)bounds.size.y;
		double sx = (double)bounds.size.x / 2.0;
		double sy = (double)bounds.size.y / 2.0;
		double x = (double)bounds.topLeft.x + sx;
		double y = (double)bounds.topLeft.y + sy;
		return mat4d(
			sx , 0.0, 0.0,   x,
			0.0, -sy, 0.0,   y,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		) * vmath::perspective_matrix(FoV, aspect, zNear, zFar);
	}

	virtual mat4d getModelView() const
	{
		return vmath::translation_matrix(0.0, -GridWidth/4.0, -cameraDist) * vmath::azimuth_elevation_matrix(az, el);
	}

	virtual void renderUI(const recti &bounds) const
	{
		const vec2i screenCentre = bounds.topLeft + vec2i(bounds.size.x/2, bounds.size.y/2);
		const double screenRadius = std::min(bounds.size.x, bounds.size.y) / 2.0;

		if (dragging)
		{
			glDisable(GL_TEXTURE_2D);

			vec2i pos0 = sphereToScreen(screenCentre, screenRadius, pt0);
			vec2i pos1 = sphereToScreen(screenCentre, screenRadius, pt1);

			glPointSize(5.0);
			glBegin(GL_POINTS);
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex2i(pos0.x, pos0.y);

			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex2i(pos1.x, pos1.y);
			glEnd();

			glColor3f(0.7f, 0.7f, 0.7f);
			glBegin(GL_LINE_STRIP);
			const int N = 30;
			for (int i = 0; i <= N; ++i)
			{
				double a = (double)i / (double)N;
				vec2i p = sphereToScreen(screenCentre, screenRadius, slerp(quatd(pt0, 0.0), quatd(pt1, 0.0), a).v);
				glVertex2i(p.x, p.y);
			}
			glEnd();
		}
	}

private:
	bool dragging;
	double cameraDist;

	vec3d pt0, pt1;
	double az0, el0;
	double az, el;

	void startDrag(const vec2i &screenCentre, double screenRadius, const vec2i &pos)
	{
		pt0 = pt1 = screenToSphere(screenCentre, screenRadius, pos);
		az0 = az;
		el0 = el;
	}
	
	void updateDrag(const vec2i &screenCentre, double screenRadius, const vec2i &pos)
	{
		pt1 = screenToSphere(screenCentre, screenRadius, pos);

		vec3d a, b;

		a = constrainToAxis(pt0, 1);
		b = constrainToAxis(pt1, 1);
		double deltaAz = std::acos(dot(a, b));
		if (b.x < a.x)
			deltaAz *= -1.0;
		
		a = constrainToAxis(pt0, 0);
		b = constrainToAxis(pt1, 0);
		double deltaEl = std::acos(dot(a, b));
		if (b.y < a.y)
			deltaEl *= -1.0;

		az = az0 + deltaAz;
		el = el0 + deltaEl;
	}

	// constrains a point on the unit sphere to the given axis (0 = x, 1 = y, 2 = z)
	vec3d constrainToAxis(vec3d pt, int axis) const
	{
		pt[axis] = 0.0;
		if (dot(pt, pt) <= 0.00001)
		{
			// FIXME: vector is nearly zero, what should we do?!
			return vec3d(0.0, 0.0, 0.0);
		}
		else
			return normalize(pt);
	}

	vec3d screenToSphere(const vec2i &screenCentre, double screenRadius, vec2i pos) const
	{
		pos -= screenCentre;
		pos.y *= -1;

		vec3d v(pos.x / screenRadius, pos.y / screenRadius, 0.0);

		double r = v.x*v.x + v.y*v.y;
		if (r > 1.0) // normalize v
			v *= vmath::rsqrt(r);
		else
			v.z = std::sqrt(1.0 - r);

		return v;
	}

	vec2i sphereToScreen(const vec2i &screenCentre, const double screenRadius, const vec3d &pt) const
	{
		vec2i pos((int)(pt.x*screenRadius), (int)(pt.y*screenRadius));
		pos.y *= -1;
		pos += screenCentre;
		return pos;
	}
};

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

class SkeletonDisplay : public OrbWidget
{
public:
	SkeletonDisplay(const WidgetID &wid, Camera *camera, Skeleton *skeleton)
		: OrbWidget(wid), mCamera(camera), mSkeleton(skeleton) {}

	void run(OrbGui &gui, OrbLayout &lyt)
	{
		vec2i wndSize = gui.input->getWindowSize();
		recti bounds = lyt.place(vec2i(0, 0));
		double aspect = (double)bounds.size.x / (double)bounds.size.y;
		
		// update input
		mCamera->update(*gui.input, bounds);

		glEnable(GL_SCISSOR_TEST);
		glScissor(bounds.topLeft.x, wndSize.y - (bounds.topLeft.y + bounds.size.y), bounds.size.x, bounds.size.y);

		mat4d proj = mCamera->getProjection(bounds);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glMultMatrixd(proj);

		mat4d view = mCamera->getModelView();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadMatrixd(view);

		glDisable(GL_TEXTURE_2D);
		glCallList(gridList);
		glColor3f(1.0f, 1.0f, 1.0f);
		mSkeleton->render();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, (double)wndSize.x, (double)wndSize.y, 0.0, -1.0, 1.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		mCamera->renderUI(bounds);

		glDisable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		glVertex2i(bounds.topLeft.x, bounds.topLeft.y);
		glVertex2i(bounds.topLeft.x + bounds.size.x, bounds.topLeft.y);
		glVertex2i(bounds.topLeft.x + bounds.size.x, bounds.topLeft.y + bounds.size.y);
		glVertex2i(bounds.topLeft.x, bounds.topLeft.y + bounds.size.y);
		glEnd();

		// reset the matrices and viewport
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		
		glDisable(GL_SCISSOR_TEST);
	}
private:
	Skeleton *mSkeleton;
	Camera *mCamera;
};

void renderGui(OrbGui &gui, Camera &cam, Skeleton &skel)
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

	SkeletonDisplay("display", &cam, &skel).run(gui, FixedLayout(leftRightSplit, 0, wndSize.x - leftRightSplit, topBottomSplit));
	Button("x-ortho", "X").run(gui, FixedLayout(leftRightSplit, topBottomSplit, a - leftRightSplit, wndSize.y - topBottomSplit));
	Button("y-ortho", "Y").run(gui, FixedLayout(a, topBottomSplit, b - a, wndSize.y - topBottomSplit));
	Button("z-ortho", "Z").run(gui, FixedLayout(b, topBottomSplit, wndSize.x - b, wndSize.y - topBottomSplit));
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
			renderGui(gui, *cam, skel);
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
