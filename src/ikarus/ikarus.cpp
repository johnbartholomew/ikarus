
#include <windows.h>
#include <GL/glfw.h>

void initGL()
{
	glClearColor(0.7f, 0.8f, 0.1f, 1.0f);
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
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
			throw "WAT?";

		glfwSetWindowTitle("Ikarus");

		initGL();

		while (true)
		{
			draw();
			glfwSwapBuffers();
			
			if (glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
				break;
		}
	}
	catch (...)
	{
		retval = 1;
	}

	glfwTerminate();
	return retval;
}
