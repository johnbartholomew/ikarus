#ifndef POSE_H
#define POSE_H

class Skeleton;
class Bone;
class Joint;

class Pose
{
public:
	struct JointState
	{
		JointState(): orientA(vmath::identityq<double>()), orientB(vmath::identityq<double>()) {}
		explicit JointState(quatd a, quatd b): orientA(a), orientB(b) {}

		vec3d worldPos;
		quatd orientA;
		quatd orientB;
	};

	Pose();
	explicit Pose(Skeleton &skel);

	void reset();
	void render() const;

	const vec3d &getJointPos(const Joint &j);

	const Joint &getRoot() const;
	const vec3d &getRootPos() const;
	void setRoot(const Joint &j);
	void setRootPos(const vec3d &pos);
	const JointState &getJointState(const Joint &j);
	void setJointState(const Joint &j, const JointState &js);
private:
	const Skeleton *skeleton;
	std::vector<JointState> jointStates;
	const Joint *root;
	vec3d rootPos;
	bool jointPositionsDirty;

	void updateJointPositions();
	void updateJointPos(const Bone *fromBone, const Joint &j, const mat4d &basis);
	void updateBoneJointPos(const Joint *fromJoint, const Bone &b, const mat4d &basis);

	void renderJoint(const Bone *fromBone, const Joint &j, const mat4d &basis) const;
	void renderBone(const Joint *fromJoint, const Bone &b, const mat4d &basis) const;
};


#endif
