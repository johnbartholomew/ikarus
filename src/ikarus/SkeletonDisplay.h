#ifndef SKELETON_DISPLAY_H
#define SKELETON_DISPLAY_H

#include "OrbGui.h"

class Camera;
class Skeleton;
class Pose;
class IkSolver;

class ThreeDDisplay : public OrbWidget
{
public:
	ThreeDDisplay(const WidgetID &wid, Camera *camera, GLuint gridList = 0)
		: OrbWidget(wid), mCamera(camera), mGridList(gridList) {}

	void run(OrbGui &gui, OrbLayout &lyt);
	virtual void renderScene() const = 0;
private:
	Camera *mCamera;
	GLuint mGridList;
};

class SkeletonDisplay : public ThreeDDisplay
{
public:
	SkeletonDisplay(const WidgetID &wid, Camera *camera, const Skeleton *skeleton, bool showJointBasis, bool showConstraints, GLuint gridList = 0)
		: ThreeDDisplay(wid, camera, gridList), mSkeleton(skeleton), mShowJointBasis(showJointBasis), mShowConstraints(showConstraints) {}

	virtual void renderScene() const;
private:
	const Skeleton *mSkeleton;
	bool mShowJointBasis;
	bool mShowConstraints;
};

class PoseDisplay : public ThreeDDisplay
{
public:
	PoseDisplay(const WidgetID &wid, Camera *camera, const Pose *pose, bool showJointBasis, bool showConstraints, GLuint gridList = 0)
		: ThreeDDisplay(wid, camera, gridList), mPose(pose), mShowJointBasis(showJointBasis), mShowConstraints(showConstraints) {}

	virtual void renderScene() const;
private:
	const Pose *mPose;
	bool mShowJointBasis;
	bool mShowConstraints;
};

class IkSolverDisplay : public ThreeDDisplay
{
public:
	IkSolverDisplay(const WidgetID &wid, Camera *camera, const IkSolver *solver, bool showJointBasis, bool showConstraints, GLuint gridList = 0)
		: ThreeDDisplay(wid, camera, gridList), mSolver(solver), mShowJointBasis(showJointBasis), mShowConstraints(showConstraints) {}

	virtual void renderScene() const;
private:
	const IkSolver *mSolver;
	bool mShowJointBasis;
	bool mShowConstraints;
};

#endif
