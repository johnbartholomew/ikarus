#ifndef GLOBAL_H
#define GLOBAL_H

// global standard libraries

#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cmath>

#include <algorithm>
#include <functional>
#include <utility>
#include <limits>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

// global non-standard libraries

#ifdef _WIN32

#define _WIN32_WINNT 0x0501
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#define GLEW_STATIC
#include <GL/glew.h>

#else

#include <GL/glew.h>

#endif

// global internal headers

#include "refvector.h"
#include "smartptr.h"
#include "vmath.h"
#include "scopedenum.h"
#include "murmurhash.h"

namespace vmath
{
	template <typename T>
	struct rect
	{
		rect(): topLeft(T(0), T(0)), size(T(0), T(0)) {}

		explicit rect(const vec2<T> &v): topLeft(v), size(T(0), T(0)) {}
		explicit rect(const vec2<T> &v0, const vec2<T> &v1): topLeft(v0), size(v1) {}
		explicit rect(T left, T top): topLeft(left, top), size(T(0), T(0)) {}
		explicit rect(T left, T top, T width, T height): topLeft(left, top), size(width, height) {}

		bool operator==(const rect<T> &b) const
		{ return (topLeft == b.topLeft) && (size == b.size); }
		bool operator!=(const rect<T> &b) const
		{ return !(*this == b); }

		bool contains(const vec2<T> &v) const
		{
			vec2<T> v2(v - topLeft);
			return
				(v2.x >= 0) && (v2.y >= 0) &&
				(v2.x < size.x) && (v2.y < size.y);
		}

		vec2<T> topLeft;
		vec2<T> size;
	};
}

typedef vmath::vec2<int> vec2i; // useful for screen/pixel coordinates
typedef vmath::rect<int> recti;

typedef vmath::vec2<double> vec2d;
typedef vmath::vec3<double> vec3d;
typedef vmath::vec4<double> vec4d;
typedef vmath::rect<double> rectd;

typedef vmath::vec2<float> vec2f;
typedef vmath::vec3<float> vec3f;
typedef vmath::vec4<float> vec4f;
typedef vmath::rect<float> rectf;

typedef vmath::mat2<double> mat2d;
typedef vmath::mat3<double> mat3d;
typedef vmath::mat4<double> mat4d;

typedef vmath::quat<double> quatd;

static const vec3d unitX(1.0, 0.0, 0.0);
static const vec3d unitY(0.0, 1.0, 0.0);
static const vec3d unitZ(0.0, 0.0, 1.0);

// ===== Utility Functions =====

template <typename T>
T clamp(const T &a, const T &b, const T &v)
{
	if (v < a)
		return a;
	else if (v > b)
		return b;
	else
		return v;
}

// ===== Configuration Constants =====

const double CameraDistance = 30.0;
const double Aspect = 4.0 / 3.0;
const double FoV = 40.0 * (M_PI/180.0);
const double zNear = 0.1;
const double zFar = 100.0;
const double CameraDistWheelScale = 1.1;

const int GridCount = 10;
const double GridWidth = 20.0;

const double MoveStep = 0.2;

#endif
