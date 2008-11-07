#include "Global.h"
#include "Skeleton.h"
#include "GfxUtil.h"

// ===== Bone ================================================================

void Bone::render(const vec3f &col) const
{
	const vec3d tip = effectorPos;

	const double len = length(effectorPos);
	const double offset = 0.1 * len;
	const double invSqrt2 = 0.70710678118654746;
	
	const vec3d unitX(1.0, 0.0, 0.0);
	const vec3d unitY(0.0, 1.0, 0.0);
	const vec3d unitZ(0.0, 0.0, 1.0);

	const vec3d dir(normalize(effectorPos));

	glColor3fv(col);
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

		const vec3d v0 = - dir*offset;
		const vec3d v1 = - spur0;
		const vec3d v2 =   spur1;
		const vec3d v3 =   spur0;
		const vec3d v4 = - spur1;
		const vec3d v5 = tip;

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

// ===== Skeleton ============================================================

void Skeleton::loadFromFile(const std::string &fname)
{
	// reset the existing skeleton
	joints.clear();
	bones.clear();
	rootPos = vec3d(0.0, 0.0, 0.0);
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
			if (n == 0)
				break;
			bones.reserve(n - 1);
		}
		else if (cmd == "bone")
		{
			vec3d worldPos;
			int parentId, childrenBegin, childrenEnd;

			std::auto_ptr<Bone> bptr(new Bone(bones.size()));
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
				joints.push_back(new Joint(joints.size()));
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
					joints.push_back(j = new Joint(joints.size()));
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

			if (numChildren <= 0)
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
	renderBlob(vec3f(1.0f, 0.0f, 0.0f), rootPos);

	Bone *a = joints[0].a, *b = joints[0].b;
	if (a != 0) renderBone(*a, rootPos);
	if (b != 0) renderBone(*b, rootPos);
}

void Skeleton::renderBone(const Bone &b, const vec3d &base) const
{
	glPushMatrix();
	mat4d basis = vmath::translation_matrix(base);
	glMultMatrixd(basis);
	b.render(vec3f(1.0f, 1.0f, 1.0f));
	glPopMatrix();

	for (int i = 1; i < (int)b.joints.size(); ++i)
	{
		const Bone::Connection &c = b.joints[i];
		Bone *child = c.joint->a;
		assert (child != 0);
		renderBone(*child, c.pos + base);
	}
}

int Skeleton::getNumBones() const
{
	return bones.size();
}

int Skeleton::getNumJoints() const
{
	return joints.size();
}

int Skeleton::getNumEffectors() const
{
	return numEffectors;
}

const vec3d &Skeleton::getDefaultRootPos() const
{
	return rootPos;
}
