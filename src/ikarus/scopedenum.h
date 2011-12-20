#ifndef SCOPED_ENUM_H
#define SCOPED_ENUM_H

#define SCOPED_ENUM(name)                   \
struct name {                               \
	enum Type : int;                        \
	name(): value(0) {}                     \
	name(Type v): value(v) {}               \
	explicit name(int v): value(v) {}       \
	name(const name &v): value(v.value) {}  \
	operator int() const { return value; }  \
	name &operator=(const name &v)          \
	{ value = v.value; return *this; }      \
	int value;                              \
};                                          \
enum name::Type : int                       \
//===========================================

#endif
