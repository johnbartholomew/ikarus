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
			minTwist = minElevation = minAzimuth = -M_PI;
			maxTwist = maxElevation = maxAzimuth = M_PI;
		}
		else if (type == Saddle)
		{
			minElevation = minAzimuth = -M_PI;
			maxElevation = maxAzimuth = M_PI;
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
			: to(to), basis(1.0), constraints(JointConstraints::Ball), pos(v) {}

		// the bone that this connection goes to
		Bone *to;

		// constraints for this joint
		// j = B(b - pos)
		// where j is a position in joint-space, B is the joint basis and b is a position in bone-space
		// B is a rotation matrix
		// the joint constraints limit rotation of the bone in joint-space
		mat3d basis;
		JointConstraints constraints;

		// position of the joint in bone-space
		// typically one joint will have a position of 0,0,0, but it's not required
		vec3d pos;

		mat4d boneToJoint() const
		{ return mat4d(basis) * vmath::translation_matrix(-pos); }
		vec3d boneToJoint(const vec3d &v) const
		{ return basis * (v - pos); }
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
	{ return (joints.size() == 1) && (joints[0].pos == vec3d(0.0, 0.0, 0.0)); }
};

class Skeleton : public RefCounted
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
	void Skeleton::initJointMatrices();
};

#endif
