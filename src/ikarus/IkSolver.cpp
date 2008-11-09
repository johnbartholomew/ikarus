#include "Global.h"
#include "IkSolver.h"
#include "Skeleton.h"
#include "GfxUtil.h"

// ===== Utilities ===========================================================

quatd calcRotation(const vec3d &tip, const vec3d &target)
{
	double lenSqrTip = dot(tip, tip);
	if (lenSqrTip < 0.001) return vmath::identityq<double>();
	double lenSqrTarget = dot(target, target);
	if (lenSqrTarget < 0.001) return vmath::identityq<double>();

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
		return vmath::identityq<double>();

	return quat_from_axis_angle(cross(a, b), angle);
}

// ===== IkSolver ============================================================

IkSolver::IkSolver(const Skeleton &skel)
:	skeleton(skel),
	rootBone(0),
	effectorBone(0),
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
		boneStates[i].bonespace = vmath::translation_matrix(skeleton[i].worldPos);
		boneStates[i].rot = vmath::identityq<double>();
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

void IkSolver::setTargetPos(const vec3d &target)
{
	targetPos = target;
}

void IkSolver::setRootBone(const Bone &bone)
{
	// not yet implemented
	assert(0);
}

void IkSolver::setEffector(const Bone &bone)
{
	effectorBone = &bone;
	ikChain.clear();
}

void IkSolver::render() const
{
	updateBoneTransforms();
	renderBone(0, *rootBone);
	renderBlob(vec3f(1.0f, 0.0f, 0.0f), rootPos);
	renderBlob(vec3f(0.0f, 1.0f, 0.0f), targetPos);
}

void IkSolver::renderBone(const Bone *parent, const Bone &b) const
{
	const BoneState &bs = boneStates[b.id];

	glPushMatrix();
	glMultMatrixd(bs.bonespace);
	if (&b == effectorBone)
		b.render(vec3f(1.0f, 1.0f, 0.0f));
	else
		b.render(vec3f(1.0f, 1.0f, 1.0f));
	glPopMatrix();

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Bone &bn = *b.joints[i].to;
		if (&bn != parent)
			renderBone(&b, bn);
	}
}

void IkSolver::solveIk(int maxIterations)
{
	// not yet implemented
	assert(0);
}

void IkSolver::iterateIk()
{
	if (ikChain.size() == 0)
		buildChain(*rootBone, *effectorBone, ikChain);

	// need the bone transforms to be valid before the step
	// because we use them to quickly find the joint position for each bone
	//updateBoneTransforms();

	// perform basic CCD
	stepIk();

	// need the bone transforms to be valid again afterwards for consistency
	updateBoneTransforms();
}

vec3d IkSolver::stepIk()
{
	vec3d tip = boneStates[effectorBone->id].bonespace.translation();

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
			vec3d jointPos = b.findJointWith(parent)->pos;
			vec3d origin = transform_point(bs.bonespace, jointPos);
			stepIkBone(b, origin, tip);
		}

		break;
	}

	return tip;
}

void IkSolver::stepIkBone(const Bone &b, const vec3d &origin, vec3d &tip)
{
	vec3d relTip = tip - origin;
	vec3d relTarget = targetPos - origin;

	// calculate the required rotation
	quatd rot = calcRotation(relTip, relTarget);
	if (rot == vmath::identityq<double>()) return;

	// update the tip location
	mat4d rotM = quat_to_mat4(rot);
	tip = origin + transform_vector(rotM, relTip);

	// sanity check
	const vec3d tmpa = normalize(relTarget);
	const vec3d tmpb = normalize(tip - origin);
	assert(dot(tmpa, tmpb) > 0.999);

	// update the bone state
	BoneState &bs = boneStates[b.id];
	bs.rot = bs.rot * rot;
}

void IkSolver::updateBoneTransforms() const
{
	assert(rootBone != 0);
	updateBoneTransform(0, *rootBone, vmath::translation_matrix(rootPos));
}

void IkSolver::updateBoneTransform(const Bone *parent, const Bone &b, const mat4d &basis) const
{
	const BoneState &bs = boneStates[b.id];

	bs.bonespace = basis * quat_to_mat4(bs.rot);

	// basis is an intermediate coordinate frame, with the parent bone's orientation,
	// but the origin at the location of the joint between b and parent
	if (parent != 0)
	{
		const Bone::Connection *c = b.findJointWith(*parent);
		assert(c != 0);
		if (abs(dot(c->pos, c->pos) - 1.0) > 0.0001)
			bs.bonespace = bs.bonespace * vmath::translation_matrix(- c->pos);
	}

	for (int i = 0; i < (int)b.joints.size(); ++i)
	{
		const Bone::Connection &c = b.joints[i];
		Bone &bn = *c.to;
		if (&bn != parent)
			updateBoneTransform(&b, bn, bs.bonespace * vmath::translation_matrix(c.pos));
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
