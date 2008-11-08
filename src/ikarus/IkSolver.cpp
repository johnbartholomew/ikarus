#include "Global.h"
#include "IkSolver.h"
#include "Pose.h"
#include "Skeleton.h"
#include "GfxUtil.h"

#if 0

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
	renderBlob(vec3f(0.0f, 1.0f, 0.0f), targetPos);
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

#endif
