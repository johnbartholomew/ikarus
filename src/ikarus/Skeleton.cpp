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

// ===== Pose ================================================================

Pose::Pose()
:	skeleton(0), root(0), rootPos(0.0, 0.0, 0.0), jointPositionsDirty(true)
{
}

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

const vec3d &Pose::getJointPos(const Joint &j)
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

void Pose::setRoot(const Joint &j)
{
	rootPos = getJointPos(j);
	root = &j;
}

void Pose::setRootPos(const vec3d &pos)
{
	rootPos = pos;
}

const Pose::JointState &Pose::getJointState(const Joint &j)
{
	assert(j.id >= 0 && j.id < (int)jointStates.size());
	updateJointPositions();
	return jointStates[j.id];
}

void Pose::setJointState(const Joint &j, const JointState &js)
{
	jointStates[j.id] = js;
	jointPositionsDirty = true;
}

void Pose::updateJointPositions()
{
	if (jointPositionsDirty)
	{
		updateJointPos(0, *root, vmath::translation_matrix(rootPos));
		jointPositionsDirty = false;
	}
}

void Pose::updateJointPos(const Bone *fromBone, const Joint &j, const mat4d &basis)
{
	JointState &js = jointStates[j.id];
	js.worldPos = vec3d(basis.elem[3][0], basis.elem[3][1], basis.elem[3][2]);

	if (j.a && (j.a != fromBone)) updateBoneJointPos(&j, *j.a, basis * quat_to_mat4(js.orientA));
	if (j.b && (j.b != fromBone)) updateBoneJointPos(&j, *j.b, basis * quat_to_mat4(js.orientB));
}

void Pose::updateBoneJointPos(const Joint *fromJoint, const Bone &b, const mat4d &basis)
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

void Pose::render() const
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

// ===== IkSolver ============================================================

void IkSolver::setPose(const Pose &from)
{
	currentPose = from;
}

void IkSolver::setRoot(const Joint &j)
{
	root = &j;
	rootPos = currentPose.getJointPos(j);
}

void IkSolver::setEffector(const Bone &b)
{
	effector = &b;
}

void IkSolver::setTargetPos(const vec3d &pos)
{
	targetPos = pos;
}

const Pose &IkSolver::getCurrentPose() const
{
	return currentPose;
}

Pose IkSolver::solveIk(const Pose &from, const Joint &root, const Bone &effector, const vec3d &target, int maxIterations)
{
	setPose(from);
	setRoot(root);
	setEffector(effector);
	setTargetPos(target);

	this->solveIk(maxIterations);

	return currentPose;
}

void IkSolver::render() const
{
	currentPose.render();
	renderPoint(vec3f(0.0f, 1.0f, 0.0f), targetPos);
}

// ===== IkSolverCCD =========================================================

void IkSolverCCD::solveIk(int maxIterations)
{
	const double TargetError = 0.0001;
	int tries = 0;

	findChain();

	vec3d tip;
	vec3d delta;
	do
	{
		tip = ikStep();
		delta = tip - targetPos;
		++tries;
	} while ((tries < maxIterations) && (dot(delta, delta) > TargetError));
}

void IkSolverCCD::iterateIk()
{
	findChain();
	ikStep();
}

vec3d IkSolverCCD::ikStep()
{
	const IkLink &effectorLink = chain.front();
	vec3d effectorPos = this->effectorPos;

	std::vector<IkLink>::const_iterator it = chain.begin();
	while (it != chain.end())
	{
		const Bone &b = *it->bone;
		const Joint &j = *it->joint;
		const Pose::JointState &js = currentPose.getJointState(j);

		const vec3d base = it->worldPos;

		vec3d v0 = effectorPos - base;
		vec3d v1 = targetPos - base;

		// now we've stored the next base, we can work out the rotation to apply
		double angle;
		double v0dotv1 = dot(v0, v1);
		if (v0dotv1 > 1.0)
			angle = 0.0; // angle is 0 with some floating point errors
		else if (v0dotv1 < -1.0)
			angle = M_PI; // angle is 180 with some floating point errors
		else
			angle = std::acos(v0dotv1);

		if (abs(angle) > 0.0001)
		{
			vec3d axis = cross(v0, v1);

			quatd rot = quat_from_axis_angle(axis, angle);

			Pose::JointState njs(js);
			if (j.a == &b)
				njs.orientA = njs.orientA * rot;
			else
				njs.orientB = njs.orientB * rot;

			currentPose.setJointState(j, njs);
			effectorPos = base + rot*v0;
		}

		++it;
	}

	return effectorPos;
}

void IkSolverCCD::findChain()
{
	chain.clear();
	findChainJoint(0, *root, vmath::translation_matrix(rootPos));
}

bool IkSolverCCD::findChainJoint(const Bone *fromBone, const Joint &j, const mat4d &basis)
{
	bool found = false;
	const Pose::JointState &js = currentPose.getJointState(j);
	if (j.a && (j.a != fromBone))
		found = findChainBone(&j, *j.a, basis * quat_to_mat4(js.orientA));
	if (j.b && (j.b != fromBone) && !found)
		found = findChainBone(&j, *j.b, basis * quat_to_mat4(js.orientB));
	return found;
}

bool IkSolverCCD::findChainBone(const Joint *fromJoint, const Bone &b, const mat4d &basis)
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

	if (&b == effector)
	{
		this->effectorPos = transform_point(bonespace, b.effectorPos);
		const vec3d pos(bonespace.elem[3][0], bonespace.elem[3][1], bonespace.elem[3][2]);
		chain.push_back(IkLink(*fromJoint, b, pos));
		return true;
	}

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Joint &j = *b.joints[i].joint;
		if (&j != fromJoint)
		{
			if (findChainJoint(&b, j, bonespace * vmath::translation_matrix(b.joints[i].pos)))
			{
				const vec3d pos(basis.elem[3][0], basis.elem[3][1], basis.elem[3][2]);
				chain.push_back(IkLink(*fromJoint, b, pos));
				return true;
			}
		}
	}

	return false;
}
