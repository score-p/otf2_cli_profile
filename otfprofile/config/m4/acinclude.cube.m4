AC_DEFUN([CHECK_CUBE],
[
    cube_error="no"
    check_cube="yes"
    force_cube="no"
    have_cube="no"

    AH_TEMPLATE(HAVE_CUBE, [], [defined if cube library is to be used])

    AC_ARG_WITH([cube],
        AC_HELP_STRING([--with-cube],
            [use cube, default: yes if found by configure]),
        [if test "$withval" = "yes"; then force_cube="yes"; else check_cube="no"; fi])

    AC_ARG_WITH([cube-dir],
        AC_HELP_STRING([--with-cube-dir],
            [give the path for cube, default: /opt/cube/]),
	[cube_dir="$withval/"])

    if test x"$cube_dir" == x; then cube_dir=/opt/cube/; fi

    AC_ARG_WITH([cube-inc-dir],
        AC_HELP_STRING([--with-cube-inc-dir],
            [give the path dir cube-include files, default: CUBEDIR/include/cube]),
        [cube_inc_dir="$withval/"],
        [if test x"$cube_dir" != x; then cube_inc_dir="$cube_dir"include/; fi])

    AC_ARG_WITH([cube-lib-dir],
        AC_HELP_STRING([--with-cube-lib-dir],
            [give the path for CUBE-libraries, default: CUBEDIR/lib]),
        [cube_lib_dir="$withval/"],
        [if test x"$cube_dir" != x; then cube_lib_dir="$cube_dir"lib/; fi])

    AC_ARG_WITH([cube-lib],
        AC_HELP_STRING([--with-cube-lib],
            [use given cube, default: -lcube4]),
        [cube_lib="$withval"])

    if test "$check_cube" = "yes"; then
	AC_LANG_SAVE
	AC_LANG_CPLUSPLUS

	sav_CPPFLAGS=$CPPFLAGS
	if test x"$cube_inc_dir" != x; then
                CPPFLAGS="$CPPFLAGS -I$cube_inc_dir"
        fi
        AC_CHECK_HEADER([cube/Cube.h], [],
        [
            AC_MSG_NOTICE([error: no Cube.h found; check path for CUBE package first...])
            cube_error="yes"
        ])
	CPPFLAGS=$sav_CPPFLAGS

        if test x"$cube_lib" = x -a "$cube_error" = "no"; then
            sav_LIBS=$LIBS
            cl="-lcube4"
            if test x"$cube_lib_dir" != x; then
                cl="-L$cube_lib_dir $cl"
            fi
            LIBS="$LIBS $cl"
            AC_MSG_CHECKING([whether linking with -lcube4 works])
            AC_TRY_LINK([],[],
            [AC_MSG_RESULT([yes]); cube_lib=-lcube4],[AC_MSG_RESULT([no])])
            LIBS=$sav_LIBS
        fi

        if test x"$cube_lib" = x -a "$cube_error" = "no"; then
            AC_MSG_NOTICE([error: could not find cube4 library; check path for CUBE package first...])
            cube_error="yes"
        fi

        if test $cube_error = "no"; then
            AC_DEFINE(HAVE_CUBE)
            have_cube="yes"
        fi

	AC_LANG_RESTORE
    fi

    CUBE_LIB_DIR=$cube_lib_dir
    CUBE_LIB_LINE=$cube_lib
    if test x"$cube_lib_dir" != x; then
        CUBE_LIB_LINE="-L$cube_lib_dir $CUBE_LIB_LINE"
    fi

    CUBE_INCLUDE_DIR=$cube_inc_dir
    CUBE_INCLUDE_LINE=
    if test x"$cube_inc_dir" != x; then
        CUBE_INCLUDE_LINE="-I$cube_inc_dir"
    fi

    AC_SUBST(CUBE_LIB_DIR)
    AC_SUBST(CUBE_LIB_LINE)
    AC_SUBST(CUBE_INCLUDE_DIR)
    AC_SUBST(CUBE_INCLUDE_LINE)
])
