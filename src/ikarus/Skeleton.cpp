#include "Global.h"
#include "Skeleton.h"
#include "GfxUtil.h"

// ===== Bone ================================================================

void Bone::render(const vec3f &col) const
{
	if ((length(displayVec) < 0.001))
		renderBlob(col, vec3d(0.0, 0.0, 0.0));
	else
	{
		const double len = length(displayVec);
		const double offset = 0.1 * len;
		const double invSqrt2 = 0.70710678118654746;
		
		const vec3d unitX(1.0, 0.0, 0.0);
		const vec3d unitY(0.0, 1.0, 0.0);
		const vec3d unitZ(0.0, 0.0, 1.0);

		const vec3d dir(normalize(displayVec));

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
			const vec3d v5 = displayVec;

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
}

// ===== Skeleton ============================================================

void Skeleton::loadFromFile(const std::string &fname)
{
	// reset the existing skeleton
	bones.clear();

	std::ifstream fs(fname.c_str(), std::ios::in);
	std::string ln;

	std::getline(fs, ln);
	if (ln != "skeleton")
		throw std::runtime_error("Invalid skeleton file: bad header.");

	std::vector<Bone*> roots;
	vec3d summedRootWorldPos(0.0, 0.0, 0.0);

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
			bones.reserve(n - 1);
		}
		else if (cmd == "bone")
		{
			int parentId;

			std::auto_ptr<Bone> bptr(new Bone((int)bones.size()));
			Bone &b = *bptr;

			ss
				>> b.name
				>> b.worldPos.x >> b.worldPos.y >> b.worldPos.z
				>> b.displayVec.x >> b.displayVec.y >> b.displayVec.z
				>> parentId;

			// ignore the root bone itself...
			if (parentId >= 0)
			{
				bones.push_back(bptr.release());

				if (parentId > 0)
				{
					Bone &bp = bones[parentId - 1];
					b.joints.push_back(Bone::Connection(&bp, vec3d(0.0, 0.0, 0.0)));
					bp.joints.push_back(Bone::Connection(&b, b.worldPos - bp.worldPos));
				}
				else
				{
					summedRootWorldPos += b.worldPos;
					roots.push_back(&b);
				}
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

	// fix up the root bones to all connect to each other
	// and ensure they only connect in a single place
	// (if they didn't, they'd need another bone to connect
	// them all to give a known spatial relationship between
	// the root joints, and so *that* bone would be the root bone)
	vec3d rootWorldPos = summedRootWorldPos / (double)roots.size();

	for (int i = 0; i < (int)roots.size(); ++i)
	{
		for (int j = 0; j < (int)roots.size(); ++j)
		{
			if (i != j)
			{
				Bone &a = *roots[i];
				Bone &b = *roots[j];

				a.joints.push_back(Bone::Connection(&b, vec3d(0.0, 0.0, 0.0)));
				const vec3d shift = rootWorldPos - a.worldPos;
				if (length(shift) > 0.0000001)
					shiftBoneWorldPositions(0, a, shift);
			}
		}
	}

	// add an extra bone to represent each effector tip
	for (int i = 0; i < (int)bones.size(); ++i)
	{
		Bone &b = bones[i];
		if (b.isEffector() && (length(b.displayVec) > 0.0001))
		{
			bones.push_back(new Bone((int)bones.size()));
			Bone &be = bones.back();
			be.name = b.name + "-tip";
			be.displayVec = vec3d(0.0, 0.0, 0.0);
			be.worldPos = b.worldPos + b.displayVec;

			be.joints.push_back(Bone::Connection(&b, vec3d(0.0, 0.0, 0.0)));
			b.joints.push_back(Bone::Connection(&be, b.displayVec));
		}
	}
}

void Skeleton::shiftBoneWorldPositions(const Bone *from, Bone &b, const vec3d &shift)
{
	b.worldPos += shift;
	for (int i = 1; i < (int)b.joints.size(); ++i)
	{
		Bone &c = *b.joints[i].to;
		if (&c != from)
			shiftBoneWorldPositions(&b, c, shift);
	}
}

void Skeleton::render() const
{
	const vec3d rootPos = bones[0].worldPos;
	renderBlob(vec3f(1.0f, 0.0f, 0.0f), rootPos);
	renderBone(0, bones[0], rootPos);
}

void Skeleton::renderBone(const Bone *from, const Bone &b, const vec3d &base) const
{
	// render the bone...
	glPushMatrix();
	mat4d basis = vmath::translation_matrix(base);
	glMultMatrixd(basis);
	b.render(vec3f(1.0f, 1.0f, 1.0f));
	glPopMatrix();

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Bone::Connection &c = b.joints[i];
		if (c.to != from)
			renderBone(&b, *c.to, c.pos + base);
	}
}
