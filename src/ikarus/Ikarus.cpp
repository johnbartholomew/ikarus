#include "Global.h"
#include "Skeleton.h"

const double CameraDistance = 20.0;
const double Aspect = 4.0 / 3.0;
const double FoV = 40.0; // degrees
const double zNear = 0.1;
const double zFar = 100.0;

double cameraAzimuth = 0.0;
double cameraElevation = 0.0;

void initGL()
{
	mat4d camera = transpose(vmath::perspective_matrix(FoV, Aspect, zNear, zFar));

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(&camera[0]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0.0, 0.0, -CameraDistance);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0);
}

void renderGrid(int N, double m)
{
	glBegin(GL_LINES);

	double b = m/2.0, a = -b;
	double x = a;
	double xd = m / (double)N;

	for (int i = 0; i <= N; ++i)
	{
		// y/z plane
		glVertex3d(a, x, a);
		glVertex3d(b, x, a);
		glVertex3d(a, a, x);
		glVertex3d(b, a, x);

		// x/y plane
		glVertex3d(a, x, a);
		glVertex3d(a, x, b);
		glVertex3d(x, a, a);
		glVertex3d(x, a, b);

		// x/z plane
		glVertex3d(a, a, x);
		glVertex3d(a, b, x);
		glVertex3d(x, a, a);
		glVertex3d(x, b, a);

		x += xd;
	}
	glEnd();
}

void render(Skeleton &skel)
{
	glClear(GL_COLOR_BUFFER_BIT);
	renderGrid(10, 20.0);
	skel.render();
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

		initGL();

		Skeleton skel;
		skel.loadFromFile("skeleton.txt");

		while (true)
		{
			render(skel);
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
