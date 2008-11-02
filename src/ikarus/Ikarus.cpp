#include "Global.h"
#include "Skeleton.h"
#include "Font.h"

const double CameraDistance = 30.0;
const double Aspect = 4.0 / 3.0;
const double FoV = 40.0 * (M_PI/180.0);
const double zNear = 0.1;
const double zFar = 100.0;

const int GridCount = 10;
const double GridHeight = 20.0;

double cameraAzimuth = 0.0;
double cameraElevation = 0.0;

TextRenderer *gTextRenderer = 0;
Font *gFont = 0;

GLuint gridList;


class CameraGrip
{
public:
	CameraGrip(int w, int h)
		:	dragging(false),
			screenCentre(w/2, h/2),
			screenRadius(std::min(w, h)/2.0),
			az(0.0), el(0.0), az0(0.0), el0(0.0)
	{
	}

	void update()
	{
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

	mat4d getCameraMatrix() const
	{
		return vmath::translation_matrix(0.0, 0.0, -CameraDistance) * vmath::azimuth_elevation_matrix(az, el);
	}

	void render() const
	{
		if (dragging)
		{
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);

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

	glColor3f(0.7f, 0.2f, 0.2f);
	glBegin(GL_LINES);
	x = a;
	for (int i = 0; i <= N; ++i)
	{
		// y/z plane (left)
		glVertex3d(a, 0.0, x); glVertex3d(a, m, x); // lines bottom-to-top
		glVertex3d(a, x+b, a); glVertex3d(a, x+b, b); // lines back-to-front
		x += xd;
	}
	glEnd();
	
	glColor3f(0.2f, 0.7f, 0.2f);
	glBegin(GL_LINES);
	x = a;
	for (int i = 0; i <= N; ++i)
	{
		// x/y plane (back)
		glVertex3d(a, x+b, a); glVertex3d(b, x+b, a); // lines left-to-right
		glVertex3d(x, 0.0, a); glVertex3d(x, m, a); // lines bottom-to-top
		x += xd;
	}
	glEnd();

	glColor3f(0.2f, 0.2f, 0.7f);
	glBegin(GL_LINES);
	x = a;
	for (int i = 0; i <= N; ++i)
	{
		// x/z plane (bottom)
		glVertex3d(a, 0.0, x); glVertex3d(b, 0.0, x); // lines left-to-right
		glVertex3d(x, 0.0, a); glVertex3d(x, 0.0, b); // lines back-to-front
		x += xd;
	}
	glEnd();
}

void initGL()
{
	mat4d camera = vmath::perspective_matrix(FoV, Aspect, zNear, zFar);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(camera);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	gridList = glGenLists(1);
	glNewList(gridList, GL_COMPILE);
	renderGrid(GridCount, GridHeight);
	glEndList();
}

void renderScene(Skeleton &skel, const mat4d &cameraMatrix)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	mat4d camera = vmath::perspective_matrix(FoV, Aspect, zNear, zFar);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(camera);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(cameraMatrix);

	glCallList(gridList);
	glColor3f(1.0f, 1.0f, 1.0f);
	skel.render();
}

void renderOverlay(const CameraGrip &camGrip)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glTranslated(100.0, 100.0, 0.0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	gTextRenderer->drawText(gFont, "Hello, world!");

	glLoadIdentity();
	camGrip.render();
}

void render(Skeleton &skel, const mat4d &cameraMatrix, const CameraGrip &camGrip)
{
	glClear(GL_COLOR_BUFFER_BIT);

	renderScene(skel, cameraMatrix);
	renderOverlay(camGrip);
}

#ifdef _WIN32
int APIENTRY wWinMain(HINSTANCE hPrevInstance, HINSTANCE hInstance, LPWSTR cmdLine, int nShowCmd)
#else
int main(int argc, char *argv[])
#endif
{
	int retval = 0;
	glfwInit();

	try
	{
		if (!glfwOpenWindow(800, 600, 8, 8, 8, 8, 24, 0, GLFW_WINDOW))
			throw std::runtime_error("Could not create OpenGL window");

		glfwSetWindowTitle("Ikarus");

		glewInit();
		initGL();

		Skeleton skel;
		skel.loadFromFile("skeleton.txt");

		Font font;
		font.loadFromFile("arial-rounded-18.fnt");
		gFont = &font;
		TextRenderer textRenderer;
		gTextRenderer = &textRenderer;

		CameraGrip cameraGrip(800, 600);
		bool dragging = false;
		while (true)
		{
			cameraGrip.update();
			render(skel, cameraGrip.getCameraMatrix(), cameraGrip);

			glfwSwapBuffers();
			
			if (glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
				break;
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

	glfwTerminate();
	return retval;
}
