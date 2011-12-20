#include "Global.h"
#include "SkeletonDisplay.h"
#include "Camera.h"
#include "Skeleton.h"
#include "Pose.h"
#include "IkSolver.h"
#include "OrbGui.h"
#include "OrbInput.h"

void ThreeDDisplay::run(OrbGui &gui, OrbLayout &lyt)
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
	if (mGridList != 0)
		glCallList(mGridList);
	glColor3f(1.0f, 1.0f, 1.0f);

	this->renderScene();

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

void SkeletonDisplay::renderScene() const
{
	mSkeleton->render(mShowJointBasis, mShowConstraints);
}

void PoseDisplay::renderScene() const
{
	//mPose->render(mShowJointBasis, mShowConstraints);
}

void IkSolverDisplay::renderScene() const
{
	mSolver->render(mShowJointBasis, mShowConstraints);
}