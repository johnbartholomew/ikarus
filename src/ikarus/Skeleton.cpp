#include "Global.h"
#include "Skeleton.h"

void Skeleton::loadFromFile(const std::string &fname)
{
	// reset the existing skeleton
	joints.clear();
	bones.clear();
	rootPos = vec3d(0.0, 0.0, 0.0);

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
			if (n == 0)
				break;
			bones.reserve(n - 1);
		}
		else if (cmd == "bone")
		{
			vec3d worldPos;
			int parentId, childrenBegin, childrenEnd;

			std::auto_ptr<Bone> bptr(new Bone);
			Bone &b = *bptr;

			ss
				>> b.name
				>> worldPos.x >> worldPos.y >> worldPos.z
				>> b.effectorPos.x >> b.effectorPos.y >> b.effectorPos.z
				>> parentId >> childrenBegin >> childrenEnd;

			int numChildren = (childrenEnd - childrenBegin);

			if (parentId < 0)
			{
				// special case for the root bone
				assert(numChildren <= 2);
				joints.push_back(new Joint);
			}
			else
			{
				bones.push_back(bptr.release());

				Joint *j;
				if (parentId == 0)
				{
					j = &joints[0];
					if (j->a == 0)
						j->a = &b;
					else
					{
						assert(j->b == 0);
						j->b = &b;
					}
				}
				else
				{
					joints.push_back(j = new Joint);
					j->a = &b;
				}

				b.joints.push_back(Bone::Connection(j, vec3d(0.0, 0.0, 0.0)));

				if (parentId > 0)
				{
					Bone &bp = bones[parentId - 1];
					
					assert(j->a != 0);
					assert(j->b == 0);
					j->b = &bp;

					bp.joints.push_back(Bone::Connection(j, worldPos));
				}
				else
					rootPos = worldPos;
			}


			// add a joint to connect to each child
			if (parentId >= 0)
			{
				for (int i = childrenBegin; i < childrenEnd; ++i)
					joints.push_back(new Joint);
			}
			else
			{
				// the root only ever creates one joint
				assert(childrenEnd - childrenBegin <= 2);
				joints.push_back(new Joint);
			}
		}
		else if (cmd == "")
		{
			// ignore blank lines
		}
		else
			throw std::runtime_error("Invalid command in skeleton file.");
	}
	fs.close();

	// fix up joint positions to all be in the local bone space
	fixJointPositions(*joints[0].a, rootPos);
	fixJointPositions(*joints[0].b, rootPos);
}

void Skeleton::fixJointPositions(Bone &b, const vec3d &worldPos)
{
	for (int i = 1; i < (int)b.joints.size(); ++i)
	{
		Bone::Connection &c = b.joints[i];
		fixJointPositions(*c.joint->a, c.pos);
		c.pos -= worldPos;
	}
}

void Skeleton::render() const
{
	renderPoint(vec3f(1.0f, 0.0f, 0.0f), rootPos);

	renderBone(*joints[0].a, rootPos);
	renderBone(*joints[0].b, rootPos);
}

void Skeleton::renderBone(const Bone &b, const vec3d &base) const
{
	const vec3d head = base;
	const vec3d end = head + b.effectorPos;

	const double len = length(b.effectorPos);
	const double offset = 0.1 * len;
	const double invSqrt2 = 0.70710678118654746;
	
	const vec3d unitX(1.0, 0.0, 0.0);
	const vec3d unitY(0.0, 1.0, 0.0);
	const vec3d unitZ(0.0, 0.0, 1.0);

	const vec3d dir(normalize(b.effectorPos));

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

	if (b.joints.size() == 1)
		renderPoint(vec3f(1.0f, 1.0f, 0.0f), end);
	else
	{
		for (int i = 1; i < (int)b.joints.size(); ++i)
		{
			const Bone::Connection &c = b.joints[i];
			renderBone(*c.joint->a, c.pos + base);
		}
	}
}

void Skeleton::renderPoint(const vec3f &col, const vec3d &pos) const
{
	glColor3fv(col);
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
