#include "Global.h"
#include "Skeleton.h"
#include "Font.h"

const double CameraDistance = 30.0;
const double Aspect = 4.0 / 3.0;
const double FoV = 40.0; // degrees
const double zNear = 0.1;
const double zFar = 100.0;

const int GridCount = 10;
const double GridHeight = 20.0;

double cameraAzimuth = 0.0;
double cameraElevation = 0.0;

TextRenderer *gTextRenderer = 0;
Font *gFont = 0;

GLuint gridList;

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

void renderOverlay()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(100.0, 100.0, 0.0);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	gTextRenderer->drawText(gFont, "Hello, world!");
}

void render(Skeleton &skel, const mat4d &cameraMatrix)
{
	glClear(GL_COLOR_BUFFER_BIT);

	renderScene(skel, cameraMatrix);
	renderOverlay();
}

class CameraGrip
{
public:
	CameraGrip()
		:	mouseOrigin(0, 0),
			elevation0(0), azimuth0(0),
			elevation(0), azimuth(0)
	{
	}

	void startDrag(const vec2i &pos)
	{
		mouseOrigin = pos;
		elevation0 = elevation;
		azimuth0 = azimuth;
	}
	
	void updateDrag(const vec2i &pos)
	{
		vec2i delta = pos - mouseOrigin;
		//elevation = elevation0 + (double)(delta.y / 50.0);
		//azimuth = azimuth0 + (double)(delta.x / 50.0);
	}

	mat4d getCameraMatrix() const
	{
		return vmath::euler(azimuth, elevation, 0.0) * vmath::translation_matrix(0.0, 0.0, -CameraDistance);
	}

private:
	vec2i mouseOrigin;
	double elevation0, azimuth0;
	double elevation, azimuth;
};

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

		CameraGrip cameraGrip;
		bool dragging = false;
		while (true)
		{
			vec2i mousePos;
			glfwGetMousePos(&mousePos.x, &mousePos.y);
			if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				if (!dragging)
				{
					dragging = true;
					cameraGrip.startDrag(mousePos);
				}
				else
					cameraGrip.updateDrag(mousePos);
			}
			else
			{
				if (dragging)
				{
					cameraGrip.updateDrag(mousePos);
					dragging = false;
				}
			}

			render(skel, cameraGrip.getCameraMatrix());

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
