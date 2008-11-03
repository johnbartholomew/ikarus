#ifndef SKELETON_H
#define SKELETON_H

// A Skeleton represents a set of bones and the joints between them
// it provides methods to load the skeleton and get at the bone and joint information

class Joint
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

	Joint()
	:	type(Null),
		minAzimuth(0.0), maxAzimuth(0.0),
		minElevation(0.0), maxElevation(0.0),
		minTwist(0.0), maxTwist(0.0)
	{}

	Joint(
		JointType type
	)
	:	type(type),
		minAzimuth(0.0), maxAzimuth(0.0),
		minElevation(0.0), maxElevation(0.0),
		minTwist(0.0), maxTwist(0.0)
	{}

	Joint(
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

class Segment
{
public:
	enum SegmentType
	{
		Root,
		Bone,
		Effector
	};

	SegmentType type;

	// a unique name for the segment (valid for all three types)
	std::string name;

	// only valid for segment types Root and Bone
	vec3d displayVector; // the vector to use to display the bone

	// valid for all three types
	vec3d worldOrigin; // the origin position of the segment in world space
	vec3d origin; // the origin position of the segment relative to its parent segment/joint

	// the type and constraints for the joint connecting this bone to its parent
	// only valid for segments of type Bone
	Joint joint;

	// display the segment
	void render();

private:
	friend class Skeleton;
	
	int parent;
	int childrenBegin;
	int childrenEnd;
};

class Skeleton
{
public:
	void loadFromFile(const std::string &fname);
	void render() const;

	int getNumEffectors() const;
	int getNumBones() const;

	// getNumSegments() == getNumBones() + getNumEffectors() + 1 (for the root)
	int getNumSegments() const;

	Segment &getRoot() const;
	Segment &getParent(const Segment &b) const;
	std::vector<Segment>::iterator getChildrenBegin(const Segment &b) const;
	std::vector<Segment>::iterator getChildrenEnd(const Segment &b) const;
private:
	int numBones;
	int numEffectors;
	std::vector<Segment> segments;

	void renderSegment(int idx, const vec3d &base) const;
	void renderBone(const Segment &e, const vec3d &base) const;
	void renderEffector(const Segment &e, const vec3d &base) const;
};

// A pose represents a configuration that a skeleton can be in
// it tracks the state of each joint and can be used to
// query the position of a particular joint or effector
class Pose
{
public:
	struct JointState
	{
		// the rotation parameter values
		double azimuth;
		double elevation;
		double twist;

		// the baked rotation
		quatd rot;
	};

	void render();

	vec3d getSegmentPos();

private:
	Skeleton *skeleton;
	std::vector<JointState> joints;
};

#endif
