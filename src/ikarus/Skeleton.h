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

class Bone
{
public:
	struct Connection
	{
		explicit Connection(Bone *to, vec3d v): to(to), pos(v) {}

		// the bone that this connection goes to
		Bone *to;

		// position of the joint in bone-space
		// typically one joint will have a position of 0,0,0, but it's not required
		vec3d pos;
	};

	explicit Bone(int id): id(id), displayVec(0.0, 0.0, 0.0), worldPos(0.0, 0.0, 0.0) {}

	const int id;
	std::string name;
	std::vector<Connection> joints;

	const Connection *findJointWith(const Bone &b) const
	{
		for (
			std::vector<Connection>::const_iterator it = joints.begin();
			it != joints.end();
			++it)
		{
			if (it->to == &b)
				return &(*it);
		}
		return 0;
	}

	// expects the matrices to be set up to put vertices in bone-space
	void render(const vec3f &col) const;

	// the bone is displayed going from its 0,0,0 point
	// to the displayVec (in bone-space)
	// (0,0,0 is usually the position of one of its connections)
	vec3d displayVec;

	// the default world position of the origin of the bone-space basis
	vec3d worldPos;

	bool isEffector() const
	{ return (joints.size() == 1); }
};

class Skeleton
{
public:
	void loadFromFile(const std::string &fname);
	void render() const;

	const Bone &operator[](int idx) const
	{ return bones[idx]; }
	Bone &operator[](int idx)
	{ return bones[idx]; }

	int numBones() const
	{ return (int)bones.size(); }
private:
	refvector<Bone> bones;
	void renderBone(const Bone *from, const Bone &b, const vec3d &base) const;
	void shiftBoneWorldPositions(const Bone *from, Bone &b, const vec3d &shift);
};

#endif
