#include "Global.h"
#include "Pose.h"
#include "Skeleton.h"
#include "GfxUtil.h"

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
	renderBlob(vec3f(1.0f, 0.0f, 0.0f), rootPos);
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
