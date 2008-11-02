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
	glColor3f(1.0f, 1.0f, 1.0f);
	renderBone(0, vec3d(0.0, 0.0, 0.0));
	glColor3f(0.0f, 1.0f, 0.0f);
	renderTarget();
}

void Skeleton::renderTarget()
{
	glBegin(GL_LINES);
	{
		const double size = 0.5;
		const vec3d a(size, 0.0, 0.0);
		const vec3d b(0.0, size, 0.0);
		const vec3d c(0.0, 0.0, size);

		const vec3d v0 = targetPos - b;
		const vec3d v1 = targetPos - a;
		const vec3d v2 = targetPos - c;
		const vec3d v3 = targetPos + a;
		const vec3d v4 = targetPos + c;
		const vec3d v5 = targetPos + b;

		glVertex3dv(v0); glVertex3dv(v5);
		glVertex3dv(v1); glVertex3dv(v3);
		glVertex3dv(v2); glVertex3dv(v4);

#if 0
		glVertex3dv(v0); glVertex3dv(v1);
		glVertex3dv(v0); glVertex3dv(v2);
		glVertex3dv(v0); glVertex3dv(v3);
		glVertex3dv(v0); glVertex3dv(v4);

		glVertex3dv(v1); glVertex3dv(v2);
		glVertex3dv(v2); glVertex3dv(v3);
		glVertex3dv(v3); glVertex3dv(v4);
		glVertex3dv(v4); glVertex3dv(v1);

		glVertex3dv(v1); glVertex3dv(v5);
		glVertex3dv(v1); glVertex3dv(v5);
		glVertex3dv(v1); glVertex3dv(v5);
		glVertex3dv(v1); glVertex3dv(v5);
#endif
	}
	glEnd();
}

void Skeleton::renderBone(int idx, const vec3d &root)
{
	const Bone &b = bones[idx];
	const vec3d end = b.orient + root;

	const double len = length(b.orient);
	const double offset = 0.1 * len;
	const double invSqrt2 = 0.70710678118654746;
	
	const vec3d unitX(1.0, 0.0, 0.0);
	const vec3d unitY(0.0, 1.0, 0.0);
	const vec3d unitZ(0.0, 0.0, 1.0);

	const vec3d dir(normalize(b.orient));

	glBegin(GL_LINES);
	{
		vec3d spur0;
		if (abs(dot(dir, unitX)) < 0.8)
			spur0 = cross(dir, unitX);
		else
			spur0 = cross(dir, unitZ);
		vec3d spur1 = cross(spur0, dir);

		spur0 *= offset;
		spur1 *= offset;

		const vec3d v0 = root - dir*offset;
		const vec3d v1 = root - spur0;
		const vec3d v2 = root + spur1;
		const vec3d v3 = root + spur0;
		const vec3d v4 = root - spur1;
		const vec3d v5 = end;

		glVertex3dv(v0); glVertex3dv(v1);
		glVertex3dv(v0); glVertex3dv(v2);
		glVertex3dv(v0); glVertex3dv(v3);
		glVertex3dv(v0); glVertex3dv(v4);

		glVertex3dv(v1); glVertex3dv(v2);
		glVertex3dv(v2); glVertex3dv(v3);
		glVertex3dv(v3); glVertex3dv(v4);
		glVertex3dv(v4); glVertex3dv(v1);
		
		glVertex3dv(v1); glVertex3dv(v5);
		glVertex3dv(v2); glVertex3dv(v5);
		glVertex3dv(v3); glVertex3dv(v5);
		glVertex3dv(v4); glVertex3dv(v5);
	}
	glEnd();

	for (int i = b.childrenBegin; i != b.childrenEnd; ++i)
		renderBone(i, end);
}
