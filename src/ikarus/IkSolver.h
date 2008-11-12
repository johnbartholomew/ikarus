#ifndef IK_SOLVER_H
#define IK_SOLVER_H

#include "Skeleton.h"

class Skeleton;
class Bone;

class IkSolver : public RefCounted
{
public:
	IkSolver(const Skeleton &skel);

	const vec3d &getTargetPos() const;
	const Bone &getRootBone() const;
	const Bone &getEffector() const;
	vec3d getEffectorPos() const;

	void setTargetPos(const vec3d &target);
	void setRootBone(const Bone &bone);
	void setEffector(const Bone &bone);

	// resets the root bone, effector and target position
	void resetAll();

	// resets the pose to be the neutral (skeleton-default) pose
	void resetPose();

	// render the skeleton, with root, effector and target highlighted
	void render(bool showJointBasis, bool showJointConstraints) const;

	// try to completely solve for the current target
	void solveIk(int maxIterations, double threshold = 0.001);

	// perform one iteration of whatever IK algorithm is being implemented
	void iterateIk();

	// apply all joint constraints
	void applyAllConstraints();
	
private:
	struct BoneState
	{
		BoneState(): rot(1.0), boneToWorld(1.0) {}

		// nominal rotation relative to parent
		mat3d rot;

		// cached world-space position & absolute orientation
		mutable mat4d boneToWorld;
	};

	// an IkSolver is linked at construction with a skeleton
	// and cannot be switched to a different skeleton
	const Skeleton &skeleton;

	const Bone *rootBone;
	const Bone *effectorBone;
	std::vector<const Bone*> ikChain;
	std::vector<BoneState> boneStates;

	bool mApplyConstraints;

	vec3d targetPos;
	vec3d rootPos;

	vec3d stepIk();
	vec3d updateJointByIk(const Bone &b, const Bone::Connection &joint, const vec3d &target, const vec3d &tip);

	void applyAllConstraints(const Bone *parent, const Bone &b);
	void applyConstraints(const Bone &b, const Bone::Connection &bj);

	void resetBoneRot(const Bone *parent, const Bone &b);
	void updateBoneTransforms() const;
	void updateBoneTransforms(const Bone *parent, const Bone &b, const mat4d &base) const;

	bool buildChain(const Bone &from, const Bone &to, std::vector<const Bone*> &chain) const;
	bool buildChain(const Bone *parent, const Bone &b, const Bone &target, std::vector<const Bone*> &chain) const;

	bool isAngleInRange(double minA, double maxA, double a) const;
};


#endif
