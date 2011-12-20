#ifndef POSE_H
#define POSE_H

class Skeleton;
class Bone;

class Pose
{
public:
	struct BoneState
	{
		BoneState(): orient(vmath::identityq<double>()) {}
		explicit BoneState(quatd a): orient(a) {}

		vec3d worldPos;
		// orient is the absolute orientation of the bone
		// (not an orientation relative to some other bone)
		// this is because if the orientation was relative,
		// then the pose would need to specify a root bone...
		quatd orient;
	};

	Pose();
	explicit Pose(const Skeleton &skel);

	void reset();
	void render() const;

private:
	const Skeleton *skeleton;
	std::vector<BoneState> boneStates;
};

#endif
