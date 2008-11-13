#include "Global.h"
#include "Skeleton.h"
#include "GfxUtil.h"
#include "MathUtil.h"

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

#define RENDER_BONE_COORDS 0
#define RENDER_JOINT_COORDS 1

void Bone::renderJointCoordinates() const
{
	const double a = 0.75; // FIXME: shouldn't be hardcoded

	// render joint coordinate spaces
	glBegin(GL_LINES);
	{
#if RENDER_BONE_COORDS
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(a, 0.0, 0.0);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, a, 0.0);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, 0.0, a);
#endif

#if RENDER_JOINT_COORDS
		for (int i = 0; i < (int)joints.size(); ++i)
		{
			const Bone::Connection &c = joints[i];

			if ((i != primaryJointIdx) || (c.to->isChildOf(*this)))
			{
				// don't bother with joints going to effectors
				// effectors can't do anything anyway (they're just points)
				if (c.to->isEffector()) continue;

				vec3d ux( a , 0.0, 0.0);
				vec3d uy(0.0,  a , 0.0);
				vec3d uz(0.0, 0.0,  a );

				ux += c.pos;
				uy += c.pos;
				uz += c.pos;

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
#endif
	}
	glEnd();
}

void Bone::renderJointConstraints() const
{
	const double radius = 0.75;

	// render joint constraints with the parent bone
	// twist is constrained in bone-space
	if ((primaryJointIdx >= 0) && (constraints.minTwist < constraints.maxTwist))
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_STRIP);
		arcPoints(
			joints[primaryJointIdx].pos,
			vec3d(0.0, 1.0, 0.0),
			vec3d(0.0, 0.0, 1.0),
			radius/2.0,
			constraints.minTwist,
			constraints.maxTwist
		);
		glEnd();
	}

	// render joint constraints for joints with child bones
	// azimuth & elevation are constrained in the parent bone-space
	for (int i = 0; i < (int)joints.size(); ++i)
	{
		const Bone::Connection &c = joints[i];

		if ((i != primaryJointIdx) || (c.to->isChildOf(*this)))
		{
			// don't bother with joints going to effectors
			// effectors can't do anything anyway (they're just points)
			if (c.to->isEffector()) continue;

			const Bone &child = *c.to;
			const JointConstraints &cnst = child.constraints;

			// draw the real azimuth range
			glLineWidth(1.25f);
			glColor3f(0.0f, 1.0f, 0.0f);
			glBegin(GL_LINE_STRIP);
			arcPoints(
				c.pos,
				vec3d(0.0, 1.0, 0.0),
				vec3d(0.0, 0.0, 1.0),
				radius,
				cnst.minAzimuth,
				cnst.maxAzimuth
			);
			glEnd();

			if (cnst.minAzimuth >= cnst.maxAzimuth)
			{
				double a = cnst.minAzimuth;
				glPointSize(3.5f);
				glBegin(GL_POINTS);
				glVertex3d(c.pos.x + radius*sin(a), c.pos.y, c.pos.z + radius*cos(a));
				glEnd();
			}

			// draw the rest of the azimuth range in a fainter green
			// so that it's possible to see where the joint plane is when the azimuth is fixed
			glLineWidth(0.75f);
			glColor3f(0.2f, 0.5f, 0.2f);
			glBegin(GL_LINE_STRIP);
			arcPoints(
				c.pos,
				vec3d(0.0, 1.0, 0.0),
				vec3d(0.0, 0.0, 1.0),
				radius,
				cnst.maxAzimuth,
				cnst.minAzimuth+(2.0*M_PI)
			);
			glEnd();

			glColor3f(0.0f, 0.0f, 1.0f);
			double range = cnst.maxAzimuth - cnst.minAzimuth;
			int N = 1 + (int)(range / (M_PI/6.0));
			for (int i = 0; i <= N; ++i)
			{
				double a = cnst.minAzimuth + i*(range/N);

				glBegin(GL_LINE_STRIP);
				arcPoints(
					c.pos,
					vec3d(cos(a), 0.0, -sin(a)),
					vec3d(0.0, 1.0, 0.0),
					radius,
					cnst.minElevation,
					cnst.maxElevation
				);
				glEnd();
			}
		}
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

		// skip comments (comments start with %)
		if ((cmd.size() > 0) && (cmd[0] == '%'))
			continue;

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
			else if (jointType == "custom")
			{
				b.constraints.type = JointConstraints::Custom;
				ss
					>> b.constraints.minAzimuth >> b.constraints.maxAzimuth
					>> b.constraints.minElevation >> b.constraints.maxElevation
					>> b.constraints.minTwist >> b.constraints.maxTwist;
				b.constraints.minAzimuth *= M_PI/180.0;
				b.constraints.maxAzimuth *= M_PI/180.0;
				b.constraints.minElevation *= M_PI/180.0;
				b.constraints.maxElevation *= M_PI/180.0;
				b.constraints.minTwist *= M_PI/180.0;
				b.constraints.maxTwist *= M_PI/180.0;
			}

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

void Skeleton::initBoneMatrices()
{
	initBoneMatrix(0, bones[0]);
}

void Skeleton::initBoneMatrix(const Bone *parent, Bone &bone)
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

	double dotAlongX = dot(along, unitX);
	if (dotAlongX < -0.8)
		front = normalize(cross(unitY, along));
	else if (dotAlongX > 0.8)
		front = normalize(cross(along, unitY));
	else
		front = normalize(cross(unitX, along));
	side = cross(along, front);

	// sanity check
	assert(along.isnormalized());
	assert(front.isnormalized());
	assert(side.isnormalized());

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
			initBoneMatrix(&bone, *c.to);
	}
}

void Skeleton::render(bool showJointBasis, bool showJointConstraints) const
{
	const vec3d rootPos = bones[0].worldPos;
	renderBlob(vec3f(1.0f, 0.0f, 0.0f), rootPos);
	renderBone(0, bones[0], rootPos, showJointBasis, showJointConstraints);
}

void Skeleton::renderBone(const Bone *from, const Bone &b, const vec3d &pos, bool showJointBasis, bool showJointConstraints) const
{
	const mat3d &basis = b.defaultOrient;
	// render the bone...
	glPushMatrix();
	const mat4d frame(vmath::translation_matrix(pos) * mat4d(basis));
	glMultMatrixd(frame);
	b.render(vec3f(1.0f, 1.0f, 1.0f));
	if (showJointBasis && !b.isEffector())
		b.renderJointCoordinates();
	if (showJointConstraints && !b.isEffector())
		b.renderJointConstraints();
	glPopMatrix();

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Bone::Connection &c = b.joints[i];
		if (c.to != from)
			renderBone(&b, *c.to, pos + basis*c.pos, showJointBasis, showJointConstraints);
	}
}
