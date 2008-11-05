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
	virtual void update() = 0;
	virtual void render() const = 0;
	virtual mat4d getProjection() const = 0;
	virtual mat4d getModelView() const = 0;
};

class CameraOrtho : public Camera
{
public:
	CameraOrtho(int w, int h, int axis)
		:	aspect((double)w / (double)h),
			axis(axis),
			scale(GridWidth/2.0) // nb: if you change this change update()
	{
	}

	virtual void update()
	{
#if 0
		int wheel = glfwGetMouseWheel();
		// nb: if you change this change the CameraOrtho initializer
		scale = (GridWidth/2.0) * std::pow(CameraDistWheelScale, -wheel);
#endif
	}

	virtual void render() const {}

	virtual mat4d getProjection() const
	{
		double a = aspect*scale;
		return vmath::ortho_matrix(-a, a, -scale, scale, 0.1, 100.0);
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
		return m * vmath::translation_matrix(0.0, -GridWidth/4.0, 0.0);
	}
private:
	double aspect;
	double scale;
	int axis;
};

class CameraAzimuthElevation : public Camera
{
public:
	CameraAzimuthElevation(int w, int h)
		:	dragging(false),
			cameraDist(CameraDistance),
			screenCentre(w/2, h/2),
			screenRadius(std::min(w, h)/2.0),
			az(0.0), el(0.0), az0(0.0), el0(0.0)
	{
	}

	virtual void update()
	{
#if 0
		int wheel = glfwGetMouseWheel();
		cameraDist = CameraDistance * std::pow(CameraDistWheelScale, -wheel);

		vec2i mousePos;
		glfwGetMousePos(&mousePos.x, &mousePos.y);
		if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			if (!dragging)
			{
				dragging = true;
				startDrag(mousePos);
			}
			else
				updateDrag(mousePos);
		}
		else
		{
			if (dragging)
			{
				updateDrag(mousePos);
				dragging = false;
			}
		}
#endif
	}

	void startDrag(const vec2i &pos)
	{
		pt0 = pt1 = screenToSphere(pos);
		az0 = az;
		el0 = el;
	}
	
	void updateDrag(const vec2i &pos)
	{
		pt1 = screenToSphere(pos);

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

	virtual mat4d getProjection() const
	{
		return vmath::perspective_matrix(FoV, Aspect, zNear, zFar);
	}

	virtual mat4d getModelView() const
	{
		return vmath::translation_matrix(0.0, -GridWidth/4.0, -cameraDist) * vmath::azimuth_elevation_matrix(az, el);
	}

	virtual void render() const
	{
		if (dragging)
		{
			glDisable(GL_TEXTURE_2D);

			vec2i pos0 = sphereToScreen(pt0);
			vec2i pos1 = sphereToScreen(pt1);

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
				vec2i p = sphereToScreen(slerp(quatd(pt0, 0.0), quatd(pt1, 0.0), a).v);
				glVertex2i(p.x, p.y);
			}
			glEnd();
		}
	}

private:
	bool dragging;
	double cameraDist;
	vec2i screenCentre;
	double screenRadius;

	vec3d pt0, pt1;
	double az0, el0;
	double az, el;

	// constrains a point on the unit sphere to the given axis (0 = x, 1 = y, 2 = z)
	vec3d constrainToAxis(vec3d pt, int axis) const
	{
		pt[axis] = 0.0;
		return normalize(pt);
	}

	vec3d screenToSphere(vec2i pos) const
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

	vec2i sphereToScreen(const vec3d &pt) const
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

void renderScene(Skeleton &skel, const Camera &cam)
{
	glDisable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(cam.getProjection());

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(cam.getModelView());

	glCallList(gridList);
	glColor3f(1.0f, 1.0f, 1.0f);
	skel.render();
}

void renderOverlay(const Camera &cam)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	cam.render();
}

void render(Skeleton &skel, const Camera &cam)
{
	glClear(GL_COLOR_BUFFER_BIT);

	renderScene(skel, cam);
	renderOverlay(cam);
}

void renderGui(OrbGui &gui)
{
	static bool showControls = false;
	static double sliderVal = 50.0;
	
	Label("title", "MONKEY").run(gui, FixedLayout(10, 10));
	showControls = CheckBox("show", "Show Controls", showControls).run(gui, FixedLayout(10, 32));

	if (showControls)
	{
		Label("hello", "Hello, world!").run(gui, FixedLayout(30, 54));
		sliderVal = Slider("value", 0.0, 100.0, sliderVal).run(gui, FixedLayout(30, 76, 100, 15));
		std::ostringstream ss;
		ss << sliderVal;
		Label("value-lbl", ss.str()).run(gui, FixedLayout(140, 76));
		if (Button("hide-btn", "Hide").run(gui, FixedLayout(30, 98)))
			showControls = false;
	}
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
		CameraAzimuthElevation cameraPerspective(800, 600);
		CameraOrtho cameraX(800, 600, 0);
		CameraOrtho cameraY(800, 600, 1);
		CameraOrtho cameraZ(800, 600, 2);

		// pick the default camera
		Camera *cam = &cameraPerspective;

		wnd.input.beginFrame();
		while (Window::ProcessWaitingMessages(&retval))
		{
			glClear(GL_COLOR_BUFFER_BIT);
			
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			vec2i sz = wnd.getSize();
			glOrtho(0.0, (double)sz.x, (double)sz.y, 0.0, -1.0, 1.0);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			renderGui(gui);

			if (wnd.input.wasKeyPressed(KeyCode::Escape))
				break;

			wnd.flipGL();
			wnd.input.beginFrame();
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
