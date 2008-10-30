#include "Global.h"
#include "Skeleton.h"

void Skeleton::loadFromFile(const std::string &fname)
{
	// reset the existing skeleton
	bones.clear();

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
			bones.reserve(n);
		}
		else if (cmd == "bone")
		{
			Bone bone;
			ss >> bone.name >> bone.orient[0] >> bone.orient[1] >> bone.orient[2] >> bone.parent >> bone.childrenBegin >> bone.childrenEnd;
			if (bone.childrenEnd < bone.childrenBegin)
				throw std::runtime_error("Invalid bone command in skeleton file.");
			bones.push_back(bone);
		}
		else if (cmd == "")
		{
			// ignore blank lines
		}
		else
			throw std::runtime_error("Invalid command in skeleton file.");
	}
	fs.close();
}

void Skeleton::render()
{
	renderBone(0, vec3d(0.0, 0.0, 0.0));
}

void Skeleton::renderBone(int idx, const vec3d &root)
{
	const Bone &b = bones[idx];
	const vec3d end = b.orient + root;
	
	glBegin(GL_LINES);
		glVertex3d(root[0], root[1], root[2]);
		glVertex3d(end[0], end[1], end[2]);
	glEnd();

	for (int i = b.childrenBegin; i != b.childrenEnd; ++i)
		renderBone(i, end);
}
