#include "Global.h"
#include "Skeleton.h"

// ===== Utilities ===========================================================

void renderPoint(const vec3f &col, const vec3d &pos)
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
	renderPoint(vec3f(1.0f, 0.0f, 0.0f), rootPos);

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

	if (b.joints.size() == 1)
		renderPoint(vec3f(1.0f, 1.0f, 0.0f), base + b.effectorPos);
	else
	{
		for (int i = 1; i < (int)b.joints.size(); ++i)
		{
			const Bone::Connection &c = b.joints[i];
			Bone *child = c.joint->a;
			assert (child != 0);
			renderBone(*child, c.pos + base);
		}
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

Joint &Skeleton::getDefaultRoot()
{
	return joints[0];
}

const vec3d &Skeleton::getDefaultRootPos()
{
	return rootPos;
}

// ===== Pose ================================================================

Pose::Pose(Skeleton &skel)
:	skeleton(&skel),
	jointPositionsDirty(true)
{
	int N = skel.getNumJoints();
	jointStates.reserve(N);
	for (int i = 0; i < N; ++i)
		jointStates.push_back(JointState());
	root = &skeleton->getDefaultRoot();
	rootPos = skeleton->getDefaultRootPos();
}

void Pose::reset()
{
	int N = skeleton->getNumJoints();
	root = &skeleton->getDefaultRoot();
	rootPos = skeleton->getDefaultRootPos();
	for (int i = 0; i < N; ++i)
	{
		JointState &js = jointStates[i];
		js.orientA = vmath::identityq<double>();
		js.orientB = vmath::identityq<double>();
	}
	jointPositionsDirty = true;
}

vec3d Pose::getJointPos(Joint &j)
{
	updateJointPositions();
	return jointStates[j.id].worldPos;
}

const Joint &Pose::getRoot() const
{
	return *root;
}

const vec3d &Pose::getRootPos() const
{
	return rootPos;
}

void Pose::setRoot(Joint &j)
{
	rootPos = getJointPos(j);
	root = &j;
}

void Pose::setRootPos(const vec3d &pos)
{
	rootPos = pos;
}

const Pose::JointState &Pose::getState(const Joint &j)
{
	assert(j.id >= 0 && j.id < (int)jointStates.size());
	updateJointPositions();
	return jointStates[j.id];
}

void Pose::updateJointPositions()
{
	if (jointPositionsDirty)
	{
		updateJointPos(0, *root, vmath::translation_matrix(rootPos));
		jointPositionsDirty = false;
	}
}

void Pose::updateJointPos(Bone *fromBone, Joint &j, const mat4d &basis)
{
	JointState &js = jointStates[j.id];
	js.worldPos = vec3d(basis.elem[3][0], basis.elem[3][1], basis.elem[3][2]);

	if (j.a && (j.a != fromBone)) updateBoneJointPos(&j, *j.a, basis * quat_to_mat4(js.orientA));
	if (j.b && (j.b != fromBone)) updateBoneJointPos(&j, *j.b, basis * quat_to_mat4(js.orientB));
}

void Pose::updateBoneJointPos(Joint *fromJoint, Bone &b, const mat4d &basis)
{
	vec3d origin;
	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		Joint &j = *b.joints[i].joint;
		if (&j != fromJoint) continue;
		origin = b.joints[i].pos;
		break;
	}

	mat4d bonespace = basis * vmath::translation_matrix(-origin);

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		Joint &j = *b.joints[i].joint;
		if (&j == fromJoint) continue;

		updateJointPos(&b, j, bonespace * vmath::translation_matrix(b.joints[i].pos));
	}
}

void Pose::render()
{
	renderPoint(vec3f(1.0f, 0.0f, 0.0f), rootPos);
	renderJoint(0, *root, vmath::translation_matrix(rootPos));
}

void Pose::renderJoint(const Bone *fromBone, const Joint &j, const mat4d &basis) const
{
	const JointState &js = jointStates[j.id];

	if (j.a && (j.a != fromBone))
		renderBone(&j, *j.a, basis * quat_to_mat4(js.orientA));

	if (j.b && (j.b != fromBone))
		renderBone(&j, *j.b, basis * quat_to_mat4(js.orientB));
}

void Pose::renderBone(const Joint *fromJoint, const Bone &b, const mat4d &basis) const
{
	vec3d origin;

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		Joint &j = *b.joints[i].joint;
		if (&j != fromJoint) continue;
		origin = b.joints[i].pos;
		break;
	}

	mat4d bonespace = basis * vmath::translation_matrix(-origin);

	glPushMatrix();
	glMultMatrixd(bonespace);
	vec3f col;
	if (fromJoint == root)
		col = vec3f(1.0f, 1.0f, 0.0f);
	else
		col = vec3f(1.0f, 1.0f, 1.0f);
	b.render(col);
	glPopMatrix();

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		Joint &j = *b.joints[i].joint;
		if (&j == fromJoint) continue;

		renderJoint(&b, j, bonespace * vmath::translation_matrix(b.joints[i].pos));
	}
}

// ===========================================================================

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

#endif
