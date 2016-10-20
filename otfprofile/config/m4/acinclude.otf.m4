dnl 	have the otf dirs specified
AC_DEFUN([CHECK_OTF],
[
    otf_error="no"
    check_otf="yes"
    force_otf="no"
    have_otf="no"

    AH_TEMPLATE(HAVE_OPEN_TRACE_FORMAT, [], [defined if otf library is to be used])

    AC_ARG_WITH([otf],
        AC_HELP_STRING([--with-otf],
            [use otf, default: yes if found by configure]),
        [if test "$withval" = "yes"; then force_otf="yes"; else check_otf="no"; fi])

    

    AC_ARG_WITH([otf-dir],
        AC_HELP_STRING([--with-otf-dir],
            [give the path for otf, default: /usr/local/]),
        [otf_dir="$withval/"])

    if test x"$otf_dir" == x; then otf_dir=/usr/local/; fi

    AC_ARG_WITH([otf-inc-dir],
        AC_HELP_STRING([--with-otf-inc-dir],
            [give the path dir otf-include files, default: OTFDIR/include/open-trace-format/]),
        [otf_inc_dir="$withval/"],
        [if test x"$otf_dir" != x; then otf_inc_dir="$otf_dir"include/open-trace-format/; fi])

    AC_ARG_WITH([otf-lib-dir],
        AC_HELP_STRING([--with-otf-lib-dir],
            [give the path for OTF-libraries, default: OTFDIR/lib]),
        [otf_lib_dir="$withval/"],
        [if test x"$otf_dir" != x; then otf_lib_dir="$otf_dir"/lib/; fi])

    AC_ARG_WITH([otf-lib],
        AC_HELP_STRING([--with-otf-lib],
            [use given otf, default: -lopen-trace-format]), dnl -lotf
        [otf_lib="$withval"])

    if test "$check_otf" = "yes"; then
	sav_CPPFLAGS=$CPPFLAGS
	if test x"$otf_inc_dir" != x; then
		CPPFLAGS="$CPPFLAGS -I$otf_inc_dir"
	fi
        AC_CHECK_HEADER([otf.h], [],
        [
            AC_MSG_NOTICE([error: no otf.h found; check path for OTF package first...])
            otf_error="yes"
        ])
	CPPFLAGS=$sav_CPPFLAGS

        if test x"$otf_lib" = x -a "$otf_error" = "no"; then
            sav_LIBS=$LIBS
            cl="-lopen-trace-format"
            if test x"$otf_lib_dir" != x; then
                cl="-L$otf_lib_dir$cl"
            fi
            LIBS="$LIBS $cl"
            AC_MSG_CHECKING([whether linking with -lopen-trace-format works])
            AC_TRY_LINK([],[],
            [AC_MSG_RESULT([yes]); otf_lib=-lopen-trace-format],[AC_MSG_RESULT([no])])
            LIBS=$sav_LIBS
        fi

        if test x"$otf_lib" = x -a "$otf_error" = "no"; then
            AC_MSG_NOTICE([error: could not find OTF library; check path for OTF package first...])
            otf_error="yes"
        fi

        if test $otf_error = "no"; then
            AC_DEFINE(HAVE_OPEN_TRACE_FORMAT)
            have_otf="yes"
        fi
   fi

    OTF_LIB_DIR=$otf_lib_dir
    OTF_LIB_LINE=$otf_lib
    if test x"$otf_lib_dir" != x; then
        OTF_LIB_LINE="-L$otf_lib_dir $OTF_LIB_LINE"
    fi

    OTF_INCLUDE_DIR=$otf_inc_dir
    OTF_INCLUDE_LINE=
    if test x"$otf_inc_dir" != x; then
        OTF_INCLUDE_LINE="-I$otf_inc_dir"
    fi

    AC_SUBST(OTF_LIB_DIR)
    AC_SUBST(OTF_LIB_LINE)
    AC_SUBST(OTF_INCLUDE_DIR)
    AC_SUBST(OTF_INCLUDE_LINE)
])
