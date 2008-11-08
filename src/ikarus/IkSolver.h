#ifndef IK_SOLVER_H
#define IK_SOLVER_H

#include "Skeleton.h"

class Skeleton;
class Bone;

class IkSolver
{
public:
	IkSolver(const Skeleton &skel);

	const vec3d &getTargetPos() const;
	const Bone &getRootBone() const;
	const Bone &getEffector() const;

	void setTargetPos(const vec3d &target);
	void setRootBone(const Bone &bone);
	void setEffector(const Bone &bone);

	// resets the root bone, effector and target position
	void resetAll();

	// resets the pose to be the neutral (skeleton-default) pose
	void resetPose();

	// render the skeleton, with root, effector and target highlighted
	void render() const;

	// try to completely solve for the current target
	void solveIk(int maxIterations);

	// perform one iteration of whatever IK algorithm is being implemented
	void iterateIk();

private:
	struct BoneState
	{
		BoneState(): bonespace(1.0), rot(vmath::identityq<double>()) {}
		BoneState(const vec3d &worldPos)
			: bonespace(vmath::translation_matrix(worldPos)), rot(vmath::identityq<double>()) {}

		// nominal (relative) state values
		quatd rot;

		// cached absolute transform
		mutable mat4d bonespace;
	};

	// an IkSolver is linked at construction with a skeleton
	// and cannot be switched to a different skeleton
	const Skeleton &skeleton;

	const Bone *rootBone;
	const Bone *effectorBone;
	std::vector<const Bone*> ikChain;
	std::vector<BoneState> boneStates;

	vec3d targetPos;
	vec3d rootPos;

	void renderBone(const Bone *parent, const Bone &b) const;

	void updateBoneTransforms() const;
	void updateBoneTransform(const Bone *parent, const Bone &b, const mat4d &basis) const;

	bool buildChain(const Bone &from, const Bone &to, std::vector<const Bone*> &chain) const;
	bool buildChain(const Bone *parent, const Bone &b, const Bone &target, std::vector<const Bone*> &chain) const;
};


#endif
