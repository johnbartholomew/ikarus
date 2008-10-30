#ifndef SKELETON_H
#define SKELETON_H

class Skeleton
{
public:
	struct Bone
	{
		std::string name;
		vec3d orient;

		int parent;
		int childrenBegin;
		int childrenEnd;
	};

	std::vector<Bone> bones;

	void render();
	void loadFromFile(const std::string &fname);
private:
	void renderBone(int idx, const vec3d &root);
};

#endif
