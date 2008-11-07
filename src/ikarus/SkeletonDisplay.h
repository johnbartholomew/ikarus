#ifndef SKELETON_DISPLAY_H
#define SKELETON_DISPLAY_H

#include "OrbGui.h"

class Camera;
class Skeleton;
class Pose;

class SkeletonDisplay : public OrbWidget
{
public:
	SkeletonDisplay(const WidgetID &wid, Camera *camera, Skeleton *skeleton, Pose *pose = 0, GLuint gridList = 0)
		: OrbWidget(wid), mCamera(camera), mSkeleton(skeleton), mPose(pose), mGridList(gridList) {}

	void run(OrbGui &gui, OrbLayout &lyt);
private:
	Skeleton *mSkeleton;
	Pose *mPose;
	Camera *mCamera;
	GLuint mGridList;
};

#endif
