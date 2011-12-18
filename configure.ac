dnl Process this file with autoconf to create configure.

AC_PREREQ(2.59)

AC_INIT([ikarus], [0.00], [jpa.bartholomew@gmail.com])

AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 dist-xz dist-zip])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_C_CONST
AC_PROG_RANLIB
#AC_PROG_LIBTOOL

AC_CHECK_LIB([GLEW], [glewInit], [], [AC_MSG_ERROR([GLEW is required])])
AC_CHECK_LIB([SOIL], [SOIL_load_image], [], [AC_MSG_ERROR([SOIL is required])])

AC_PATH_PROG([SDLCONFIG], [sdl-config])
if test "x-$SDLCONFIG" = "x-" ; then
	AC_MSG_ERROR([SDL is required])
fi
LIBS="$LIBS `$SDLCONFIG --libs`"
CXXFLAGS="$CXXFLAGS `$SDLCONFIG --cflags`"

AC_CONFIG_FILES([
	Makefile 
	src/Makefile
	src/ikarus/Makefile
])
AC_OUTPUT
