#ifndef SKELETON_DISPLAY_H
#define SKELETON_DISPLAY_H

#include "OrbGui.h"

class Camera;
class Skeleton;

class SkeletonDisplay : public OrbWidget
{
public:
	SkeletonDisplay(const WidgetID &wid, Camera *camera, Skeleton *skeleton, GLuint gridList = 0)
		: OrbWidget(wid), mCamera(camera), mSkeleton(skeleton), mGridList(gridList) {}

	void run(OrbGui &gui, OrbLayout &lyt);
private:
	Skeleton *mSkeleton;
	Camera *mCamera;
	GLuint mGridList;
};

#endif
