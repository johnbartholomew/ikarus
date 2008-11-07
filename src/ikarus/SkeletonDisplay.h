#ifndef SKELETON_DISPLAY_H
#define SKELETON_DISPLAY_H

#include "OrbGui.h"

class Camera;
class Skeleton;
class Pose;
class IkSolver;

class SkeletonDisplay : public OrbWidget
{
public:
	SkeletonDisplay(const WidgetID &wid, Camera *camera, const IkSolver *solver, GLuint gridList = 0)
		: OrbWidget(wid), mCamera(camera), mSolver(solver), mGridList(gridList) {}

	void run(OrbGui &gui, OrbLayout &lyt);
private:
	const IkSolver *mSolver;
	Camera *mCamera;
	GLuint mGridList;
};

#endif
