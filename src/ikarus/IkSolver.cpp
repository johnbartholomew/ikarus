#include "Global.h"
#include "IkSolver.h"
#include "Skeleton.h"
#include "GfxUtil.h"

// ===== Utilities ===========================================================

mat3d calcRotation(const vec3d &tip, const vec3d &target)
{
	double lenSqrTip = dot(tip, tip);
	if (lenSqrTip < 0.001) return mat3d(1.0);
	double lenSqrTarget = dot(target, target);
	if (lenSqrTarget < 0.001) return mat3d(1.0);

	vec3d a = tip * vmath::rsqrt(lenSqrTip);
	vec3d b = target * vmath::rsqrt(lenSqrTarget);

	// sanity check
	assert(abs(dot(a,a) - 1.0) < 0.0001);
	assert(abs(dot(b,b) - 1.0) < 0.0001);

	double dotAB = dot(a, b);
	double angle;
	if (dotAB <= -1.0)
		angle = M_PI;
	else if (dotAB >= 1.0)
		angle = 0.0;
	else
		angle = std::acos(dotAB);

	// sanity check
	assert(angle >= -M_PI && angle <= M_PI);

	// early-out if the angle is small
	if (angle < 0.001)
		return mat3d(1.0);

	return vmath::rotation_matrix3(angle, cross(a, b));
}

// ===== IkSolver ============================================================

IkSolver::IkSolver(const Skeleton &skel)
:	skeleton(skel),
	rootBone(0),
	effectorBone(0),
	mApplyConstraints(true),
	targetPos(0.0, 0.0, 0.0),
	rootPos(0.0, 0.0, 0.0)
{
	boneStates.resize(skeleton.numBones());
	resetAll();
}

void IkSolver::resetAll()
{
	rootBone = &skeleton[0];
	rootPos = rootBone->worldPos;
	effectorBone = 0;
	for (int i = 0; i < (int)skeleton.numBones(); ++i)
	{
		const Bone &b = skeleton[i];
		if (b.isEffector())
		{
			effectorBone = &b;
			targetPos = b.worldPos;
			break;
		}
	}
	
	resetPose();
}

void IkSolver::resetPose()
{
	rootPos = rootBone->worldPos;
	for (int i = 0; i < (int)skeleton.numBones(); ++i)
	{
		boneStates[i].orient = skeleton[i].defaultOrient;
		boneStates[i].worldPos = skeleton[i].worldPos;
	}
}

const vec3d &IkSolver::getTargetPos() const
{
	return targetPos;
}

const Bone &IkSolver::getRootBone() const
{
	assert(rootBone != 0);
	return *rootBone;
}

const Bone &IkSolver::getEffector() const
{
	assert(effectorBone != 0);
	return *effectorBone;
}

vec3d IkSolver::getEffectorPos() const
{
	assert(effectorBone != 0);
	const BoneState &bs = boneStates[effectorBone->id];
	return bs.worldPos;
}

void IkSolver::setTargetPos(const vec3d &target)
{
	targetPos = target;
}

void IkSolver::setRootBone(const Bone &bone)
{
	// early out if we're not changing anything
	if (rootBone == &bone) return;

	// clear the existing IK chain
	ikChain.clear();

	// set the new root, and its correct position
	rootBone = &bone;
	rootPos = boneStates[bone.id].worldPos;
}

void IkSolver::setEffector(const Bone &bone)
{
	effectorBone = &bone;
	ikChain.clear();
}

void IkSolver::render(bool showJointBasis) const
{
	updateBonePositions();
	renderBone(0, *rootBone, showJointBasis);
	renderBlob(vec3f(1.0f, 0.0f, 0.0f), rootPos);
	renderBlob(vec3f(0.0f, 1.0f, 0.0f), targetPos);
}

void IkSolver::renderBone(const Bone *parent, const Bone &b, bool showJointBasis) const
{
	const BoneState &bs = boneStates[b.id];

	glPushMatrix();
	glMultMatrixd(vmath::translation_matrix(bs.worldPos) * mat4d(bs.orient));
	if (&b == effectorBone)
		b.render(vec3f(1.0f, 1.0f, 0.0f));
	else
		b.render(vec3f(1.0f, 1.0f, 1.0f));
	if (showJointBasis && !b.isEffector())
		b.renderJointCoordinates();
	glPopMatrix();

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Bone &bn = *b.joints[i].to;
		if (&bn != parent)
			renderBone(&b, bn, showJointBasis);
	}
}

void IkSolver::solveIk(int maxIterations, double threshold)
{
	if (ikChain.size() == 0)
		buildChain(*rootBone, *effectorBone, ikChain);

	for (int i = 0; i < maxIterations; ++i)
	{
		// perform basic CCD
		stepIk();

		// need the bone transforms to be valid again afterwards for consistency
		updateBonePositions();

		vec3d delta = getEffectorPos() - getTargetPos();
		if (abs(dot(delta,delta)) < threshold*threshold)
			break;
	}
}

void IkSolver::iterateIk()
{
	if (ikChain.size() == 0)
		buildChain(*rootBone, *effectorBone, ikChain);

	// perform basic CCD
	stepIk();

	// need the bone transforms to be valid again afterwards for consistency
	updateBonePositions();
}

vec3d IkSolver::stepIk()
{
	vec3d tip = boneStates[effectorBone->id].worldPos;

	std::vector<const Bone*>::iterator it = ikChain.begin();
	// the first bone is the effector bone, so rotating it won't help; just skip it
	++it;
	while (it != ikChain.end())
	{
		const Bone &b = **it;
		BoneState &bs = boneStates[b.id];

		++it;
		if (it != ikChain.end())
		{
			const Bone &parent = **it;
			const Bone::Connection &joint = *b.findJointWith(parent);

			tip = updateJointByIk(b, joint, targetPos, tip);
		}
	}

	return tip;
}

vec3d IkSolver::updateJointByIk(const Bone &b, const Bone::Connection &joint, const vec3d &target, const vec3d &tip)
{
	BoneState &bs = boneStates[b.id];

	const vec3d jointPos = bs.worldPos + bs.orient*joint.pos;
	const vec3d relTip = tip - jointPos;
	const vec3d relTarget = target - jointPos;

	// calculate the required rotation
	mat3d rot = calcRotation(relTip, relTarget);
	if (rot == mat3d(1.0)) return tip;

	const vec3d tipInBoneSpace = transpose(bs.orient) * (tip - bs.worldPos);
	
	// update the bone state
	bs.orient = rot * bs.orient;

	// apply constraints to the bone's orientation	
	if (mApplyConstraints)
		applyConstraints(b, joint);	

	return bs.worldPos + bs.orient*tipInBoneSpace;
}

void IkSolver::applyConstraints(const Bone &b, const Bone::Connection &bj)
{
}

void IkSolver::updateBonePositions() const
{
	assert(rootBone != 0);
	updateBonePositions(0, *rootBone, rootPos);
}

void IkSolver::updateBonePositions(const Bone *parent, const Bone &b, const vec3d &base) const
{
	const BoneState &bs = boneStates[b.id];

	if (parent == 0)
		bs.worldPos = base;
	else
	{
		const Bone::Connection &c = *b.findJointWith(*parent);
		bs.worldPos = base - bs.orient*c.pos;
	}

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Bone::Connection &c = b.joints[i];
		Bone &bn = *c.to;
		vec3d &pos = bs.orient*c.pos;
		if (&bn != parent)
			updateBonePositions(&b, bn, bs.worldPos + pos);
	}
}

bool IkSolver::buildChain(const Bone &from, const Bone &to, std::vector<const Bone*> &chain) const
{
	assert(chain.size() == 0);
	return buildChain(0, from, to, chain);
}

bool IkSolver::buildChain(const Bone *parent, const Bone &b, const Bone &target, std::vector<const Bone*> &chain) const
{
	bool found = false;

	if (&b == &target)
		found = true;
	else
	{
		for (int i = 0; i < (int)b.joints.size(); ++i)
		{
			Bone &bn = *b.joints[i].to;
			if (&bn != parent)
			{
				if (buildChain(&b, bn, target, chain))
				{
					found = true;
					break;
				}
			}
		}
		
	}

	if (found)
		chain.push_back(&b);

	return found;
}

bool IkSolver::isAngleInRange(double minA, double maxA, double a) const
{
	assert(minA >= -M_PI && minA < M_PI);
	assert(maxA >= -M_PI && maxA < M_PI);
	assert(minA <= maxA);

	while (a < -M_PI) a += 2.0*M_PI;
	while (a >= M_PI) a -= 2.0*M_PI;
	return (minA <= a) && (a < maxA);
}
