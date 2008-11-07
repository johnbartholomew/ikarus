#ifndef SKELETON_H
#define SKELETON_H

// A Skeleton represents a set of bones and the joints between them
// it provides methods to load the skeleton and get at the bone and joint information

class JointConstraints
{
public:
	enum JointType
	{
		Null,   // a null joint doesn't allow any rotation
		Ball,   // a ball and socket joint allows rotation in a hemisphere, with twist
		Saddle, // a ball and socket joint allows rotation in a hemisphere, but no twist
		Hinge,  // a hinge joint only allows changes in elevation
		Pivot   // a pivot joint only allows twist
	};

	JointConstraints()
	:	type(Null),
		minAzimuth(0.0), maxAzimuth(0.0),
		minElevation(0.0), maxElevation(0.0),
		minTwist(0.0), maxTwist(0.0)
	{}

	JointConstraints(
		JointType type
	)
	:	type(type),
		minAzimuth(0.0), maxAzimuth(0.0),
		minElevation(0.0), maxElevation(0.0),
		minTwist(0.0), maxTwist(0.0)
	{}

	JointConstraints(
		JointType type,
		double minAzimuth, double maxAzimuth,
		double minElevation, double maxElevation,
		double minTwist, double maxTwist
	)
	:	type(type),
		minAzimuth(minAzimuth), maxAzimuth(maxAzimuth),
		minElevation(minElevation), maxElevation(maxElevation),
		minTwist(minTwist), maxTwist(maxTwist)
	{}

	JointType type;

	double minAzimuth, maxAzimuth;
	double minElevation, maxElevation;
	double minTwist, maxTwist;
};

class Bone;

struct Joint
{
	explicit Joint(int id): id(id), a(0), b(0) {}

	int id;
	// constraints for the joint
	JointConstraints constraints;
	// which bones the joint connects
	Bone *a;
	Bone *b;
};

class Bone
{
public:
	struct Connection
	{
		explicit Connection(Joint *j, vec3d v): joint(j), pos(v) {}

		// the joint that this connection goes to
		Joint *joint;

		// position of the joint in bone-space
		// typically one joint will have a position of 0,0,0, but it's not required
		vec3d pos;
	};

	explicit Bone(int id): id(id), effectorPos(0.0, 0.0, 0.0) {}

	int id;
	std::string name;
	std::vector<Connection> joints;

	// expects the matrices to be set up such that vertices are in bone-space
	void render(const vec3f &col) const;

	// all bones have an associated effector pos which is used to drag them (by IK)
	// this is like a joint position except it isn't necessarily the connection point
	// the effectorPos is also used when displaying the bone
	vec3d effectorPos;
};

class Skeleton
{
public:
	void loadFromFile(const std::string &fname);
	void render() const;

	int getNumEffectors() const;
	int getNumBones() const;
	int getNumJoints() const;

	Joint &getDefaultRoot()
	{ return joints[0]; }
	const Joint &getDefaultRoot() const
	{ return joints[0]; }
	const vec3d &getDefaultRootPos() const;

	Joint &getJoint(int idx) { return joints[idx]; }
	Bone &getBone(int idx) { return bones[idx]; }

	const Joint &getJoint(int idx) const { return joints[idx]; }
	const Bone &getBone(int idx) const { return bones[idx]; }
private:
	refvector<Joint> joints;
	refvector<Bone> bones;
	vec3d rootPos;
	int numEffectors;

	void fixJointPositions(Bone &b, const vec3d &worldPos);

	void renderBone(const Bone &b, const vec3d &base) const;
};

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
