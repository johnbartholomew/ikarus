#ifndef SKELETON_H
#define SKELETON_H

// A Skeleton represents a set of bones and the joints between them
// it provides methods to load the skeleton and get at the bone and joint information

class JointConstraints
{
public:
	enum JointType
	{
		Fixed,  // a fixed joint doesn't allow any rotation
		Ball,   // a ball and socket joint allows reorientation, with twist
		Saddle, // a ball and socket joint allows reorientation, but no twist
		Hinge,  // a hinge joint only allows changes in elevation (eg, knee)
		Pivot   // a pivot joint only allows twist (eg, neck, sort of...)
	};

	JointConstraints()
	:	type(Ball),
		minAzimuth(-M_PI), maxAzimuth(M_PI),
		minElevation(-M_PI), maxElevation(M_PI),
		minTwist(-M_PI), maxTwist(M_PI)
	{}

	JointConstraints(
		JointType type
	)
	:	type(type),
		minAzimuth(0.0), maxAzimuth(0.0),
		minElevation(0.0), maxElevation(0.0),
		minTwist(0.0), maxTwist(0.0)
	{
		if (type == Ball)
		{
			minElevation = 0.0;
			maxElevation = M_PI;
			minTwist = minAzimuth = -M_PI;
			maxTwist = maxAzimuth = M_PI;
		}
		else if (type == Saddle)
		{
			minElevation = 0.0;
			maxElevation = M_PI;
			minAzimuth = -M_PI;
			maxAzimuth = M_PI;
			minTwist = maxTwist = 0.0;
		}
		else if (type == Hinge)
		{
			minElevation = -M_PI;
			maxElevation = M_PI;
			minAzimuth = maxAzimuth = 0.0;
			minTwist = maxTwist = 0.0;
		}
		else if (type == Pivot)
		{
			minElevation = maxElevation = 0.0;
			minAzimuth = maxAzimuth = 0.0;
			minTwist = -M_PI;
			maxTwist = M_PI;
		}
	}

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
		explicit Connection(Bone *to, vec3d v)
			: to(to), pos(v), jointToBone(1.0) {}

		// the bone that this connection goes to
		Bone *to;

		// position of the joint in bone-space
		// typically one joint will have a position of 0,0,0, but it's not required
		vec3d pos;

		// a matrix to take a point from joint-space to bone-space
		// this matrix is used when applying joint constraints...
		mat3d jointToBone;
	};

	explicit Bone(int id):
		id(id), primaryJointIdx(-1),
		displayVec(0.0, 0.0, 0.0), worldPos(0.0, 0.0, 0.0), defaultOrient(1.0)
	{}

	const int id;
	std::string name;
	std::vector<Connection> joints;

	// the skeleton has a tree of bones
	// this may not be the same tree used by clients of the skeleton,
	// (eg, the IkSolver can change the tree to set different bones as root),
	// but the parent-child relationships in this tree are used
	// when dealing with joint constraints and some other joint properties
	// it is assumed that each bone has a 'primary' joint which links to its parent
	int primaryJointIdx;
	JointConstraints constraints;

	// the bone is displayed going from its 0,0,0 point
	// to the displayVec (in bone-space)
	// (0,0,0 is usually the position of one of its connections)
	vec3d displayVec;

	// the default world position of the origin of the bone-space basis
	vec3d worldPos;

	// the default orientation of the bone
	// this is a matrix which takes a vector from bone space to world space
	// nb: this is the *absolute* orientation of the bone
	// (which makes it independent of traversal order)
	mat3d defaultOrient;

	bool isChildOf(const Bone &b) const
	{
		return (getParent() == &b);
	}

	bool isParentOf(const Bone &b) const
	{
		return (this == b.getParent());
	}

	bool hasParent() const
	{
		return (getParent() != 0);
	}

	const Bone *getParent() const
	{
		if (primaryJointIdx >= 0 && primaryJointIdx < (int)joints.size())
			return joints[primaryJointIdx].to;
	}

	Bone *getParent()
	{
		if (primaryJointIdx >= 0 && primaryJointIdx < (int)joints.size())
			return joints[primaryJointIdx].to;
	}

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

	Connection *findJointWith(const Bone &b)
	{
		for (
			std::vector<Connection>::iterator it = joints.begin();
			it != joints.end();
			++it)
		{
			if (it->to == &b)
				return &(*it);
		}
		return 0;
	}

	bool isEffector() const
	{ return (joints.size() == 1) && (joints[0].pos == vec3d(0.0, 0.0, 0.0)); }

	// expects the matrices to be set up to put vertices in bone-space
	void render(const vec3f &col) const;
	void renderJointCoordinates() const;
};

class Skeleton : public RefCounted
{
public:
	void loadFromFile(const std::string &fname);
	void render(bool showJointBasis) const;

	const Bone &operator[](int idx) const
	{ return bones[idx]; }
	Bone &operator[](int idx)
	{ return bones[idx]; }

	int numBones() const
	{ return (int)bones.size(); }
private:
	refvector<Bone> bones;

	void renderBone(const Bone *from, const Bone &b, const vec3d &pos, bool showJointBasis) const;
	void shiftBoneWorldPositions(const Bone *from, Bone &b, const vec3d &shift);
	
	void initJointMatrices(Bone &parent, Bone &child);
	void initJointMatrix(Bone &parent, Bone &child, Bone::Connection &pj, Bone::Connection &cj);

	void initBoneMatrices();
	void initBoneMatrix(const Bone *parent, Bone &bone, const mat3d &parentOrient);
};

#endif
