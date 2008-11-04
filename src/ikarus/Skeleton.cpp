#include "Global.h"
#include "Skeleton.h"

void Skeleton::loadFromFile(const std::string &fname)
{
	// reset the existing skeleton
	segments.clear();
	numBones = 0;
	numEffectors = 0;

	std::ifstream fs(fname.c_str(), std::ios::in);
	std::string ln;

	std::getline(fs, ln);
	if (ln != "skeleton")
		throw std::runtime_error("Invalid skeleton file: bad header.");

	while (fs.good())
	{
		std::getline(fs, ln);
		std::istringstream ss(ln);
		std::string cmd;
		ss >> cmd;

		if (cmd == "bonecount")
		{
			int n;
			ss >> n;
			segments.reserve(n);
		}
		else if (cmd == "bone")
		{
			Segment b;
			b.type = Segment::Bone;
			ss >> b.name >> b.worldOrigin.x >> b.worldOrigin.y >> b.worldOrigin.z >> b.displayVector.x >> b.displayVector.y >> b.displayVector.z >> b.parent >> b.childrenBegin >> b.childrenEnd;
			if (b.childrenEnd < b.childrenBegin)
				throw std::runtime_error("Invalid bone command in skeleton file.");
			segments.push_back(b);

			++numBones;

			if (b.childrenBegin >= b.childrenEnd)
				++numEffectors;
		}
		else if (cmd == "")
		{
			// ignore blank lines
		}
		else
			throw std::runtime_error("Invalid command in skeleton file.");
	}
	fs.close();

	segments[0].type = Segment::Root;

	// add an extra segment for each end-effector
	segments.reserve(segments.size() + numEffectors);
	for (int i = 0; i < numBones; ++i)
	{
		Segment &b = segments[i];
		if (b.childrenBegin >= b.childrenEnd)
		{
			b.childrenBegin = segments.size();
			b.childrenEnd = segments.size() + 1;
			
			Segment effector;
			effector.name = b.name + "-target";
			effector.worldOrigin = b.worldOrigin + b.displayVector;
			effector.displayVector = vec3d(0.0, 0.0, 0.0);
			effector.type = Segment::Effector;
			effector.parent = i;
			effector.childrenBegin = segments.size();
			effector.childrenEnd = segments.size();
			segments.push_back(effector);
		}
	}

	// calculate the relative origins of each bone
	segments[0].origin = segments[0].worldOrigin;
	for (int i = 1; i < (int)segments.size(); ++i)
	{
		Segment &b = segments[i];
		Segment &p = segments[b.parent];
		b.origin = b.worldOrigin - p.worldOrigin;
	}
}

void Skeleton::render() const
{
	glColor3f(1.0f, 1.0f, 1.0f);
	renderSegment(0, vec3d(0.0, 0.0, 0.0));
}

void Skeleton::renderSegment(int idx, const vec3d &base) const
{
	const Segment &b = segments[idx];
	if (b.type == Segment::Effector || b.type == Segment::Root)
		renderPoint(b, base);
	else
		renderBone(b, base);
	
	for (int i = b.childrenBegin; i != b.childrenEnd; ++i)
		renderSegment(i, base + b.origin);
}

void Skeleton::renderBone(const Segment &b, const vec3d &base) const
{
	const vec3d head = base + b.origin;
	const vec3d end = head + b.displayVector;

	const double len = length(b.displayVector);
	const double offset = 0.1 * len;
	const double invSqrt2 = 0.70710678118654746;
	
	const vec3d unitX(1.0, 0.0, 0.0);
	const vec3d unitY(0.0, 1.0, 0.0);
	const vec3d unitZ(0.0, 0.0, 1.0);

	const vec3d dir(normalize(b.displayVector));

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	{
		vec3d spur0;
		if (abs(dot(dir, unitX)) < 0.8)
			spur0 = cross(dir, unitX);
		else
			spur0 = cross(dir, unitZ);
		vec3d spur1 = cross(spur0, dir);

		spur0 *= offset;
		spur1 *= offset;

		const vec3d v0 = head - dir*offset;
		const vec3d v1 = head - spur0;
		const vec3d v2 = head + spur1;
		const vec3d v3 = head + spur0;
		const vec3d v4 = head - spur1;
		const vec3d v5 = end;

		glVertex3dv(v0); glVertex3dv(v1);
		glVertex3dv(v0); glVertex3dv(v2);
		glVertex3dv(v0); glVertex3dv(v3);
		glVertex3dv(v0); glVertex3dv(v4);

		glVertex3dv(v1); glVertex3dv(v2);
		glVertex3dv(v2); glVertex3dv(v3);
		glVertex3dv(v3); glVertex3dv(v4);
		glVertex3dv(v4); glVertex3dv(v1);
		
		glVertex3dv(v1); glVertex3dv(v5);
		glVertex3dv(v2); glVertex3dv(v5);
		glVertex3dv(v3); glVertex3dv(v5);
		glVertex3dv(v4); glVertex3dv(v5);
	}
	glEnd();
}

void Skeleton::renderPoint(const Segment &e, const vec3d &base) const
{
	const vec3d pos = base + e.origin + e.displayVector;

	if (e.type == Segment::Effector)
		glColor3f(1.0f, 1.0f, 0.0f);
	else
		glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	{
		const double size = 0.25;
		const vec3d a(size, 0.0, 0.0);
		const vec3d b(0.0, size, 0.0);
		const vec3d c(0.0, 0.0, size);

		const vec3d v0 = pos - b;
		const vec3d v1 = pos - a;
		const vec3d v2 = pos - c;
		const vec3d v3 = pos + a;
		const vec3d v4 = pos + c;
		const vec3d v5 = pos + b;

		glVertex3dv(v0); glVertex3dv(v1);
		glVertex3dv(v0); glVertex3dv(v2);
		glVertex3dv(v0); glVertex3dv(v3);
		glVertex3dv(v0); glVertex3dv(v4);

		glVertex3dv(v1); glVertex3dv(v2);
		glVertex3dv(v2); glVertex3dv(v3);
		glVertex3dv(v3); glVertex3dv(v4);
		glVertex3dv(v4); glVertex3dv(v1);

		glVertex3dv(v1); glVertex3dv(v5);
		glVertex3dv(v2); glVertex3dv(v5);
		glVertex3dv(v3); glVertex3dv(v5);
		glVertex3dv(v4); glVertex3dv(v5);
	}
	glEnd();
}

#if 0
void Skeleton::iterateIK()
{
	ikStep(0, vec3d(0.0, 0.0, 0.0));
}

void Skeleton::solveIK()
{
	const double TargetError = 0.0001;
	const int MaxTries = 50;
	int tries = 0;

	vec3d target = targetPos;

	// if the target is further away than we can reach, then pretend it's just within reach but in the same direction
	double totalLen = 0.0;
	for (std::vector<Bone>::const_iterator it = bones.begin(); it != bones.end(); ++it)
		totalLen += it->length;
	double targetDist = length(target);
	if (targetDist > totalLen)
		target *= (totalLen / targetDist);

	vec3d tip;
	vec3d delta;
	do
	{
		tip = ikStep(0, vec3d(0.0, 0.0, 0.0));
		delta = tip - target;
		++tries;
	} while ((tries < MaxTries) && (dot(delta, delta) > TargetError));
}

vec3d Skeleton::ikStep(int boneIdx, const vec3d &root)
{
	Bone &b = bones[boneIdx];
	const vec3d end = b.orient + root;

	vec3d tip(end);

	if (b.childrenEnd > b.childrenBegin)
	{
		// can currently only deal with exactly 0 or 1 children
		assert(b.childrenEnd - b.childrenBegin == 1);
		for (int i = b.childrenBegin; i != b.childrenEnd; ++i)
			tip = ikStep(i, end);
	}

	vec3d v0 = tip - root;
	vec3d v1 = targetPos - root;

	if (abs(dot(v1, v1)) < 0.01 || abs(dot(v0, v0)) < 0.01)
		return tip;

	v0 = normalize(v0);
	v1 = normalize(v1);

	double v0dotv1 = dot(v0, v1);
	if (v0dotv1 > 1.0)
		// 0 degrees
		return tip;
	if (v0dotv1 < -1.0)
		// 180 degrees with some floating point errors
		v0dotv1 = -1.0;

	double angle = std::acos(v0dotv1);
	if (abs(angle) < 0.0001)
		return tip;

	vec3d axis = cross(v0, v1);

	mat4d rot = rotation_matrix(angle, axis);

	assert(b.orient.hasvalidfloats());
	b.orient = transform_vector(rot, b.orient);
	assert(b.orient.hasvalidfloats());
	
	assert(tip.hasvalidfloats());
	tip = root + transform_vector(rot, tip - root);
	assert(tip.hasvalidfloats());

	// re-normalize to the correct length
	b.orient = b.orient * (b.length / length(b.orient));

	return tip;
}

void Skeleton::renderTarget()
{
	glBegin(GL_LINES);
	{
		const double size = 0.25;
		const vec3d a(size, 0.0, 0.0);
		const vec3d b(0.0, size, 0.0);
		const vec3d c(0.0, 0.0, size);

		const vec3d v0 = targetPos - b;
		const vec3d v1 = targetPos - a;
		const vec3d v2 = targetPos - c;
		const vec3d v3 = targetPos + a;
		const vec3d v4 = targetPos + c;
		const vec3d v5 = targetPos + b;

#if 0
		glVertex3dv(v0); glVertex3dv(v5);
		glVertex3dv(v1); glVertex3dv(v3);
		glVertex3dv(v2); glVertex3dv(v4);
#endif

#if 1
		glVertex3dv(v0); glVertex3dv(v1);
		glVertex3dv(v0); glVertex3dv(v2);
		glVertex3dv(v0); glVertex3dv(v3);
		glVertex3dv(v0); glVertex3dv(v4);

		glVertex3dv(v1); glVertex3dv(v2);
		glVertex3dv(v2); glVertex3dv(v3);
		glVertex3dv(v3); glVertex3dv(v4);
		glVertex3dv(v4); glVertex3dv(v1);

		glVertex3dv(v1); glVertex3dv(v5);
		glVertex3dv(v2); glVertex3dv(v5);
		glVertex3dv(v3); glVertex3dv(v5);
		glVertex3dv(v4); glVertex3dv(v5);
#endif
	}
	glEnd();
}
#endif
