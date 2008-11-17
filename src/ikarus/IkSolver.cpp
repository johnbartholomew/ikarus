#include "Global.h"
#include "IkSolver.h"
#include "Skeleton.h"
#include "GfxUtil.h"
#include "MathUtil.h"

// ===== Utility Joint Constraint Application function =======================

mat3d constrainRot(const JointConstraints &cnst, const mat3d &rot)
{
	vec3d dir;
	double az, el, twist;

	rotationToAzimuthElevationTwist(rot, dir, az, el, twist);

	// clamp the azimuth
	az = clamp(cnst.minAzimuth, cnst.maxAzimuth, az);

	// if the elevation can be varied at all, then work out the optimal elevation given the selected azimuth, and clamp it into range
	if (cnst.minElevation != cnst.maxElevation)
	{
		double K = dir.z*cos(az) + dir.x*sin(az);
		el = atan2(K, dir.y);
		if (el < cnst.minElevation || el > cnst.maxElevation)
		{
			double dotMin = K*sin(cnst.minElevation) + dir.y*cos(cnst.minElevation);
			double dotMax = K*sin(cnst.maxElevation) + dir.y*cos(cnst.maxElevation);
			if (dotMin < dotMax)
				el = cnst.maxElevation;
			else
				el = cnst.minElevation;
		}
	}
	else
		el = cnst.minElevation;

	// clamp the twist
	twist = clamp(cnst.minTwist, cnst.maxTwist, twist);

	return rotationFromAzElTwist(az, el, twist);
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
		const Bone &b = skeleton[i];
		boneStates[i].boneToWorld = vmath::translation_matrix(b.worldPos) * mat4d(b.defaultOrient);
	}

	resetBoneRot(0, *rootBone);

	//updateBoneTransforms();
}

void IkSolver::resetBoneRot(const Bone *parent, const Bone &b)
{
	BoneState &bs = boneStates[b.id];
	if (parent != 0)
		bs.rot = transpose(parent->defaultOrient) * b.defaultOrient;
	else
		bs.rot = b.defaultOrient;

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Bone::Connection &c = b.joints[i];
		if (c.to != parent)
			resetBoneRot(&b, *c.to);
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
	return bs.boneToWorld.translation();
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

	// form a chain between the old root and the new root
	std::vector<const Bone*> chain;
	buildChain(bone, *rootBone, chain);

	// rebuild the rotation values along the chain
	// this is required because the rotation values are specified relative to the parent bone,
	// and the parent bone is dependent on which bone is root
	std::vector<const Bone*>::const_iterator it = chain.begin();
	while (it != chain.end())
	{
		const Bone &b = **it;
		BoneState &bs = boneStates[b.id];
		
		++it;
		if (it != chain.end())
		{
			const Bone &nb = **it;
			BoneState &nbs = boneStates[nb.id];
			bs.rot = transpose(nbs.rot);
		}
		else
			bs.rot = minor(bs.boneToWorld);
	}

	// set the new root, and its correct position
	rootBone = &bone;
	rootPos = boneStates[bone.id].boneToWorld.translation();
	
	updateBoneTransforms();
}

void IkSolver::setEffector(const Bone &bone)
{
	effectorBone = &bone;
	ikChain.clear();
}

bool IkSolver::areConstraintsEnabled() const
{
	return mApplyConstraints;
}

void IkSolver::enableConstraints(bool enabled)
{
	mApplyConstraints = enabled;
}

void IkSolver::render(bool showJointBasis, bool showJointConstraints) const
{
	for (int i = 0; i < skeleton.numBones(); ++i)
	{
		const Bone &b = skeleton[i];
		const BoneState &bs = boneStates[i];

		glPushMatrix();
		glMultMatrixd(bs.boneToWorld);

		if (&b == effectorBone)
			b.render(vec3f(1.0f, 1.0f, 0.0f));
		else
			b.render(vec3f(1.0f, 1.0f, 1.0f));

		if (showJointBasis && !b.isEffector())
			b.renderJointCoordinates();
		
		if (showJointConstraints && !b.isEffector())
			b.renderJointConstraints(bs.rot);

		glPopMatrix();
	}

	renderBlob(vec3f(1.0f, 0.0f, 0.0f), rootPos);
	renderBlob(vec3f(0.0f, 1.0f, 0.0f), targetPos);
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
		updateBoneTransforms();

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
	updateBoneTransforms();
}

vec3d IkSolver::stepIk()
{
	vec3d tip = boneStates[effectorBone->id].boneToWorld.translation();

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

			const mat4d worldToBone = vmath::fast_inverse(bs.boneToWorld);
			
			vec3d targetB = transform_point(worldToBone, targetPos);
			vec3d tipB = transform_point(worldToBone, tip);

			tipB = updateJointByIk(b, joint, targetB, tipB);

			tip = transform_point(bs.boneToWorld, tipB);
		}
	}

	return tip;
}

vec3d IkSolver::updateJointByIk(const Bone &b, const Bone::Connection &joint, const vec3d &target, const vec3d &tip)
{
	BoneState &bs = boneStates[b.id];

	const vec3d relTip = tip - joint.pos;
	const vec3d relTarget = target - joint.pos;

	// calculate the required rotation
	mat3d rot = calcDirectRotation(relTip, relTarget);
	if (rot == mat3d(1.0)) return tip;
	
	// apply constraints to the bone's orientation	
	if (mApplyConstraints)
	{
		mat3d oldRot = bs.rot;
		bs.rot = bs.rot * rot;
		applyConstraints(b, joint);
		rot = transpose(oldRot) * bs.rot;
	}
	else // alternatively, just apply the rotation directly
		bs.rot = bs.rot * rot;

	return joint.pos + rot*relTip;
}

void IkSolver::applyAllConstraints()
{
	applyAllConstraints(0, *rootBone);
	updateBoneTransforms();
}

void IkSolver::applyAllConstraints(const Bone *parent, const Bone &b)
{
	if ((parent == 0) && (b.primaryJointIdx >= 0))
		applyConstraints(b, b.joints[b.primaryJointIdx]);
	else if (parent != 0)
		applyConstraints(b, *b.findJointWith(*parent));

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Bone::Connection &c = b.joints[i];
		if (c.to != parent)
			applyAllConstraints(&b, *c.to);
	}
}

void IkSolver::applyConstraints(const Bone &b, const Bone::Connection &bj)
{
	// in our tree,
	// b is the child
	// bj.to is the parent
	// but this may not be the same as the canonical skeleton tree

	BoneState &bs = boneStates[b.id];
	if (bj.to == b.getParent())
		bs.rot = constrainRot(b.constraints, bs.rot);
	else
	{
		// in our internal tree, the parent/child relationship is reversed...
		// this is a somewhat painful situation

		const JointConstraints &cnst = bj.to->constraints;
		bs.rot = transpose(constrainRot(cnst, transpose(bs.rot)));
	}
}

void IkSolver::updateBoneTransforms() const
{
	assert(rootBone != 0);
	updateBoneTransforms(0, *rootBone, vmath::translation_matrix(rootPos));
}

void IkSolver::updateBoneTransforms(const Bone *parent, const Bone &b, const mat4d &base) const
{
	const BoneState &bs = boneStates[b.id];

	if (parent == 0)
		bs.boneToWorld = base * mat4d(bs.rot);
	else
	{
		const Bone::Connection &c = *b.findJointWith(*parent);
		bs.boneToWorld = base * mat4d(bs.rot) * vmath::translation_matrix(-c.pos);
	}

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Bone::Connection &c = b.joints[i];
		Bone &bn = *c.to;
		if (&bn != parent)
			updateBoneTransforms(&b, bn, bs.boneToWorld * vmath::translation_matrix(c.pos));
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
