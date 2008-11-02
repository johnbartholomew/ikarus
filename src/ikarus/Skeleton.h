#ifndef SKELETON_H
#define SKELETON_H

class Skeleton
{
public:
	struct Bone
	{
		std::string name;
		vec3d orient;

		// store the original/correct length
		// that way we don't have to rely on the IK solver exactly maintaining the bone's length
		double length;

		int parent;
		int childrenBegin;
		int childrenEnd;
	};

	std::vector<Bone> bones;
	vec3d targetPos;

	void render();
	void loadFromFile(const std::string &fname);

	void solveIK();
	void iterateIK();
private:
	void renderTarget();
	void renderBone(int boneIdx, const vec3d &root);

	vec3d ikStep(int boneIdx, const vec3d &rootPos);
};

#endif
