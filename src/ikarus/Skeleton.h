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

	Joint &getDefaultRoot();
	const vec3d &getDefaultRootPos();

	Joint &getJoint(int idx) { return joints[idx]; }
	Bone &getBone(int idx) { return bones[idx]; }
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
	Pose(Skeleton &skel);

	void reset();

	void render();
private:
	struct JointState
	{
		JointState(): orientA(vmath::identityq<double>()), orientB(vmath::identityq<double>()) {}

		quatd orientA;
		quatd orientB;
	};

	Skeleton *skeleton;
	std::vector<JointState> jointStates;

	void renderJoint(const Bone *fromBone, const Joint &j, const mat4d &basis) const;
	void renderBone(const Joint *fromJoint, const Bone &b, const mat4d &basis) const;
};

#endif
