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

void Bone::renderJointCoordinates() const
{
	// render joint coordinate spaces
	glBegin(GL_LINES);
	{
		for (int i = 0; i < (int)joints.size(); ++i)
		{
			if (i != primaryJointIdx)
			{
				const Bone::Connection &c = joints[i];

				// don't bother with joints going to effectors
				// effectors can't do anything anyway (they're just points)
				if (c.to->isEffector()) continue;

				double a = 0.75; // FIXME: shouldn't be constant
				vec3d ux( a , 0.0, 0.0);
				vec3d uy(0.0,  a , 0.0);
				vec3d uz(0.0, 0.0,  a );

				ux = c.pos + c.jointToBone*ux;
				uy = c.pos + c.jointToBone*uy;
				uz = c.pos + c.jointToBone*uz;

				glColor3f(1.0f, 0.0f, 0.0f);
				glVertex3d(c.pos.x, c.pos.y, c.pos.z);
				glVertex3d(ux.x, ux.y, ux.z);

				glColor3f(0.0f, 1.0f, 0.0f);
				glVertex3d(c.pos.x, c.pos.y, c.pos.z);
				glVertex3d(uy.x, uy.y, uy.z);

				glColor3f(0.0f, 0.0f, 1.0f);
				glVertex3d(c.pos.x, c.pos.y, c.pos.z);
				glVertex3d(uz.x, uz.y, uz.z);
			}
		}
	}
	glEnd();
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
			std::string jointType;
			int parentId;

			std::auto_ptr<Bone> bptr(new Bone((int)bones.size()));
			Bone &b = *bptr;

			ss
				>> b.name
				>> b.worldPos.x >> b.worldPos.y >> b.worldPos.z
				>> b.displayVec.x >> b.displayVec.y >> b.displayVec.z
				>> parentId
				>> jointType;

			if (jointType == "fixed")
				b.constraints = JointConstraints(JointConstraints::Fixed);
			else if (jointType == "ball")
				b.constraints = JointConstraints(JointConstraints::Ball);
			else if (jointType == "saddle")
				b.constraints = JointConstraints(JointConstraints::Saddle);
			else if (jointType == "hinge")
				b.constraints = JointConstraints(JointConstraints::Hinge);
			else if (jointType == "pivot")
				b.constraints = JointConstraints(JointConstraints::Pivot);

			// ignore the root bone itself...
			if (parentId >= 0)
			{
				bones.push_back(bptr.release());

				if (parentId > 0)
				{
					Bone &bp = bones[parentId - 1];
					b.primaryJointIdx = 0;
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

				a.primaryJointIdx = (int)a.joints.size();
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
			be.primaryJointIdx = 0;
			be.constraints = JointConstraints(JointConstraints::Fixed);

			be.joints.push_back(Bone::Connection(&b, vec3d(0.0, 0.0, 0.0)));
			b.joints.push_back(Bone::Connection(&be, b.displayVec));
		}
	}

	initBoneMatrices();

#if 0
	// initialize the joint spaces
	for (int i = 0; i < (int)roots.size(); ++i)
	{
		Bone &b = *roots[i];
		if (b.primaryJointIdx >= 0)
			initJointMatrices(*b.joints[b.primaryJointIdx].to, b);
		else
		{
			for (int j = 0; j < (int)b.joints.size(); ++j)
				initJointMatrices(b, *b.joints[j].to);
		}
	}
#endif
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

void Skeleton::initJointMatrices(Bone &parent, Bone &child)
{
	// early-out for effectors (they have no length and no children of their own, so they're sort of irrelevant)
	if (child.isEffector()) return;

	Bone::Connection &pj = *parent.findJointWith(child);
	assert(child.primaryJointIdx >= 0);
	Bone::Connection &cj = child.joints[child.primaryJointIdx];

	initJointMatrix(parent, child, pj, cj);

	for (int i = 0; i < (int)child.joints.size(); ++i)
	{
		if (i != child.primaryJointIdx)
			initJointMatrices(child, *child.joints[i].to);
	}
}

void Skeleton::initJointMatrix(Bone &parent, Bone &child, Bone::Connection &pj, Bone::Connection &cj)
{
	vec3d out;   // (Y)
	vec3d front; // (Z)
	vec3d side;  // (X)

	if (parent.joints.size() == 2)
	{
		vec3d a = parent.joints[0].pos;
		vec3d b = parent.joints[1].pos;
		if (parent.primaryJointIdx == 0)
			out = normalize(b - a);
		else
			out = normalize(a - b);
	}
	else if ((parent.joints.size() > 2) && (child.joints.size() == 2))
	{
		vec3d a = child.joints[0].pos;
		vec3d b = child.joints[1].pos;
		if (child.primaryJointIdx == 0)
			out = normalize(b - a);
		else
			out = normalize(a - b);
	}
	else
		out = normalize(parent.displayVec);

	vec3d unitZ(0.0, 0.0, 1.0);
	vec3d unitY(0.0, 1.0, 0.0);
	if (dot(unitZ, out) > 0.8)
		side = cross(unitY, out);
	else
		side = cross(out, unitZ);
	front = cross(side, out);

	pj.jointToBone = mat3d(
		side.x, out.x, front.x,
		side.y, out.y, front.y,
		side.z, out.z, front.z
	);
}

void Skeleton::initBoneMatrices()
{
	initBoneMatrix(0, bones[0], mat3d(1.0));
}

void Skeleton::initBoneMatrix(const Bone *parent, Bone &bone, const mat3d &parentOrient)
{
	// early-out for effectors (they keep the identity matrix)
	if (bone.isEffector()) return;

	vec3d along; // (Y)
	vec3d front; // (Z)
	vec3d side;  // (X)

	if (bone.joints.size() == 2)
	{
		vec3d a = bone.joints[0].pos;
		vec3d b = bone.joints[1].pos;
		if (bone.primaryJointIdx == 0)
			along = normalize(b - a);
		else
			along = normalize(a - b);
	}
	else
		along = normalize(bone.displayVec);

	vec3d unitX(1.0, 0.0, 0.0);
	vec3d unitY(0.0, 1.0, 0.0);
	vec3d unitZ(0.0, 0.0, 1.0);

	double dotAlongX = dot(along, unitX);
	if (dotAlongX < -0.8)
		front = cross(unitY, along);
	else if (dotAlongX > 0.8)
		front = cross(along, unitY);
	else
		front = cross(unitX, along);
	side = cross(along, front);

	bone.defaultOrient = mat3d(
		side.x, along.x, front.x,
		side.y, along.y, front.y,
		side.z, along.z, front.z
	);

	mat3d invOrient = transpose(bone.defaultOrient);
	for (int i = 0; i < (int)bone.joints.size(); ++i)
	{
		Bone::Connection &c = bone.joints[i];
		c.pos = invOrient * c.pos;
	}
	bone.displayVec = invOrient * bone.displayVec;

	for (int i = 0; i < (int)bone.joints.size(); ++i)
	{
		Bone::Connection &c = bone.joints[i];
		if (c.to != parent)
			initBoneMatrix(&bone, *c.to, bone.defaultOrient);
	}
}

void Skeleton::render(bool showJointBasis) const
{
	const vec3d rootPos = bones[0].worldPos;
	renderBlob(vec3f(1.0f, 0.0f, 0.0f), rootPos);
	renderBone(0, bones[0], rootPos, showJointBasis);
}

void Skeleton::renderBone(const Bone *from, const Bone &b, const vec3d &pos, bool showJointBasis) const
{
	const mat3d &basis = b.defaultOrient;
	// render the bone...
	glPushMatrix();
	const mat4d frame(vmath::translation_matrix(pos) * mat4d(basis));
	glMultMatrixd(frame);
	b.render(vec3f(1.0f, 1.0f, 1.0f));
	if (showJointBasis && !b.isEffector())
		b.renderJointCoordinates();
	glPopMatrix();

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Bone::Connection &c = b.joints[i];
		if (c.to != from)
			renderBone(&b, *c.to, pos + basis*c.pos, showJointBasis);
	}
}
