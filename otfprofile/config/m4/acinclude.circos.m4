AC_DEFUN([CHECK_CIRCOS],
[
     AC_ARG_WITH([circos_dir],
        AC_HELP_STRING([--with-circos-dir], 
                [use circos, default: yes if not found by configure]),
        [circos_bin_dir="$withval/bin"],
            [circos_bin_dir="$PATH"])

    AC_CHECK_PROG(have_circos, circos, "yes", "no")

    if test $have_circos = "yes"; then 
        AC_DEFINE(HAVE_CIRCOS, [1], [Circos Check Var])
        have_cube="yes";
    fi

])

