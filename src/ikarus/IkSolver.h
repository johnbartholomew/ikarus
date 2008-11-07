#ifndef IK_SOLVER_H
#define IK_SOLVER_H

#include "Pose.h"

class Skeleton;
class Bone;
class Joint;

class IkSolver
{
public:
	void setPose(const Pose &from);
	void setRoot(const Joint &root);
	void setEffector(const Bone &b);
	void setTargetPos(const vec3d &pos);

	const Pose &getCurrentPose() const;

	Pose solveIk(const Pose &from, const Joint &root, const Bone &effector, const vec3d &target, int maxIterations = 20);

	virtual void solveIk(int maxIterations) = 0;
	virtual void iterateIk() = 0;

	void render() const;
protected:
	Pose currentPose;
	vec3d targetPos;
	const Joint *root;
	vec3d rootPos;
	const Bone *effector;
};

class IkSolverCCD : public IkSolver
{
public:
	virtual void solveIk(int maxIterations);
	virtual void iterateIk();

private:
	struct IkLink
	{
		explicit IkLink(const Joint &j, const Bone &b, const vec3d &pos): joint(&j), bone(&b), worldPos(pos) {}
		const Joint *joint;
		const Bone *bone;
		vec3d worldPos; // position of the joint
	};

	std::vector<IkLink> chain;
	vec3d effectorPos;

	vec3d ikStep();

	void findChain();
	bool findChainBone(const Joint *fromJoint, const Bone &b, const mat4d &basis);
	bool findChainJoint(const Bone *fromBone, const Joint &j, const mat4d &basis);
};


#endif
