#include "Global.h"
#include "IkSolver.h"
#include "Skeleton.h"
#include "GfxUtil.h"

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
	// not yet implemented
	assert(0);
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
	// not yet implemented
	assert(0);
}

void IkSolver::updateBoneTransforms() const
{
	assert(rootBone != 0);
	updateBoneTransform(0, *rootBone, vmath::translation_matrix(rootPos));
}

void IkSolver::updateBoneTransform(const Bone *parent, const Bone &b, const mat4d &basis) const
{
	const BoneState &bs = boneStates[b.id];

	// basis is an intermediate coordinate frame, with the parent bone's orientation,
	// but the origin at the location of the joint between b and parent
	if (parent == 0)
	{
		// we're at the root bone
		bs.bonespace = basis * quat_to_mat4(bs.rot);
	}
	else
	{
		const Bone::Connection *c = b.findJointWith(*parent);
		assert(c != 0);
		bs.bonespace = basis * quat_to_mat4(bs.rot) * vmath::translation_matrix(- c->pos);
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
	bool found = buildChain(0, from, to, chain);
	if (found) std::reverse(chain.begin(), chain.end());
	return found;
}

bool IkSolver::buildChain(const Bone *parent, const Bone &b, const Bone &target, std::vector<const Bone*> &chain) const
{
	if (&b == &target)
	{
		chain.push_back(&b);
		return true;
	}
	else
	{
		for (int i = 0; i < (int)b.joints.size(); ++i)
		{
			Bone &bn = *b.joints[i].to;
			if (&bn != parent)
			{
				if (buildChain(&b, bn, target, chain))
					return true;
			}
		}
		return false;
	}
}
