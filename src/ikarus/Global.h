#ifndef GLOBAL_H
#define GLOBAL_H

// global standard libraries


#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <cctype>
#include <cmath>

#include <algorithm>
#include <functional>
#include <utility>
#include <limits>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

// global non-standard libraries

#define _WIN32_WINNT 0x0501
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

// global internal headers

#include "refvector.h"
#include "smartptr.h"
#include "vmath.h"

typedef vmath::vec2<int> vec2i; // useful for screen/pixel coordinates

typedef vmath::vec2<double> vec2d;
typedef vmath::vec3<double> vec3d;
typedef vmath::vec4<double> vec4d;

typedef vmath::vec2<float> vec2f;
typedef vmath::vec3<float> vec3f;
typedef vmath::vec4<float> vec4f;

typedef vmath::mat2<double> mat2d;
typedef vmath::mat3<double> mat3d;
typedef vmath::mat4<double> mat4d;

typedef vmath::quat<double> quatd;

#endif
