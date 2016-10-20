dnl 	have the otf2 dirs specified
AC_DEFUN([CHECK_OTF2],
[
    otf2_error="no"
    check_otf2="yes"
    force_otf2="no"
    have_otf2="no"

    AH_TEMPLATE(HAVE_OTF2, [], [defined if otf2 library is to be used])

    AC_ARG_WITH([otf2],
        AC_HELP_STRING([--with-otf2],
            [use otf2, default: yes if found by configure]),
        [if test "$withval" = "yes"; then force_otf2="yes"; else check_otf2="no"; fi])

    AC_ARG_WITH([otf2-dir],
        AC_HELP_STRING([--with-otf2-dir],
            [give the path for otf2, default: /opt/otf2/]),
        [otf2_dir="$withval/"])

    if test x"$otf2_dir" == x; then otf2_dir=/opt/otf2/; fi

    AC_ARG_WITH([otf2-inc-dir],
        AC_HELP_STRING([--with-otf2-inc-dir],
            [give the path dir otf2-include files, default: OTF2DIR/include/]),
        [otf2_inc_dir="$withval/"],
        [if test x"$otf2_dir" != x; then otf2_inc_dir="$otf2_dir"include/; fi])

    AC_ARG_WITH([otf2-lib-dir],
        AC_HELP_STRING([--with-otf2-lib-dir],
            [give the path for OTF2-libraries, default: OTF2DIR/lib]),
        [otf2_lib_dir="$withval/"],
        [if test x"$otf2_dir" != x; then otf2_lib_dir="$otf2_dir"lib/; fi])

    AC_ARG_WITH([otf2-lib],
        AC_HELP_STRING([--with-otf2-lib],
            [use given otf2, default: -lotf2]),
        [otf2_lib="$withval"])

    if test "$check_otf2" = "yes"; then
	sav_CPPFLAGS=$CPPFLAGS
	if test x"$otf2_inc_dir" != x; then
		CPPFLAGS="$CPPFLAGS -I$otf2_inc_dir"
	fi
        AC_CHECK_HEADER([otf2/otf2.h], [],
        [
            AC_MSG_NOTICE([error: no otf2.h found; check path for OTF2 package first...])
            otf2_error="yes"
        ])
	CPPFLAGS=$sav_CPPFLAGS

        if test x"$otf2_lib" = x -a "$otf2_error" = "no"; then
            sav_LIBS=$LIBS
            cl="-lotf2"
            if test x"$otf2_lib_dir" != x; then
                cl="-L$otf2_lib_dir $cl"
            fi
            LIBS="$LIBS $cl"
            AC_MSG_CHECKING([whether linking with -lotf2 works])
            AC_TRY_LINK([],[],
            [AC_MSG_RESULT([yes]); otf2_lib=-lotf2],[AC_MSG_RESULT([no])])
            LIBS=$sav_LIBS
        fi

        if test x"$otf2_lib" = x -a "$otf2_error" = "no"; then
            AC_MSG_NOTICE([error: could not find OTF2 library; check path for OTF2 package first...2])
            otf2_error="yes"
        fi

        if test $otf2_error = "no"; then
            AC_DEFINE(HAVE_OTF2)
            have_otf2="yes"
        fi
   fi

    OTF2_LIB_DIR=$otf2_lib_dir
    OTF2_LIB_LINE=$otf2_lib
    if test x"$otf2_lib_dir" != x; then
        OTF2_LIB_LINE="-L$otf2_lib_dir $OTF2_LIB_LINE"
    fi

    OTF2_INCLUDE_DIR=$otf2_inc_dir
    OTF2_INCLUDE_LINE=
    if test x"$otf2_inc_dir" != x; then
        OTF2_INCLUDE_LINE="-I$otf2_inc_dir"
    fi

    AC_SUBST(OTF2_LIB_DIR)
    AC_SUBST(OTF2_LIB_LINE)
    AC_SUBST(OTF2_INCLUDE_DIR)
    AC_SUBST(OTF2_INCLUDE_LINE)
])
