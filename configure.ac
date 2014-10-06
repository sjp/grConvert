AC_PREREQ([2.68])
AC_INIT([grConvert], [0.1-0], [simon.potter@auckland.ac.nz])
AC_CONFIG_SRCDIR([src/convert.h])
AC_CONFIG_HEADERS([src/config.h])

dnl Providing optional arguments so that it's not strictly necessary
dnl to have each of the components available for installing the package.
dnl For example, we could, if desired, remove PostScript support.
AC_ARG_ENABLE([ps-input],
              AS_HELP_STRING([--disable-ps-input	Disable conversion of PostScript input]),
              [HAVE_PS=FALSE],
              [HAVE_PS=TRUE])
dnl or perhaps PDF support
AC_ARG_ENABLE([pdf-input],
              AS_HELP_STRING([--disable-pdf-input	Disable conversion of PDF input]),
              [HAVE_PDF=FALSE],
              [HAVE_PDF=TRUE])
dnl or even SVG support
AC_ARG_ENABLE([svg-input],
              AS_HELP_STRING([--disable-svg-input	Disable conversion of SVG input]),
              [HAVE_SVG=FALSE],
              [HAVE_SVG=TRUE])

dnl Checks for programs (e.g. compiler, GNU tools).
AC_PROG_CC

dnl Initialising empty strings to build up, will be used to determine
dnl cflags and libs for compiler settings
CONVERT_LIBS=

dnl Checks for libraries via pkg-config.
PKG_PROG_PKG_CONFIG
dnl Strict check on cairo being available
PKG_CHECK_MODULES([cairo], [cairo], ,
                  AC_MSG_ERROR([Cairo is required for grConvert]))
dnl If we have enabled support for SVG (default) then check whether we
dnl can actually render it via librsvg
AS_IF([test "$HAVE_SVG" = "TRUE"], [
    PKG_CHECK_MODULES([rsvg], [librsvg-2.0],
        [
            AC_DEFINE([HAVE_LIBRSVG], [1], [Use librsvg])
            CONVERT_LIBS="$CONVERT_LIBS librsvg-2.0"
        ],
        [
            AC_MSG_WARN([librsvg not found; SVG support disabled])
            HAVE_SVG=FALSE
        ])
])
dnl same for PDF
AS_IF([test "$HAVE_PDF" = "TRUE"], [
    PKG_CHECK_MODULES([poppler], [poppler-glib],
        [
            AC_DEFINE(HAVE_LIBPOPPLER, [1], [Use poppler])
            CONVERT_LIBS="$CONVERT_LIBS poppler-glib"
        ], [
        AC_MSG_WARN([poppler not found; PDF support disabled])
        dnl poppler is required for PS support (because we just convert
        dnl PS -> PDF) so disable and warn if PDF support is not available.
        dnl Note: we can have PDF without PS, but not PS without PDF.
        HAVE_PS=FALSE
        HAVE_PDF=FALSE
    ])], [
    AS_IF([test "$HAVE_PS" = "TRUE"], [
        AC_MSG_WARN([PDF support not available; PostScript support disabled])
        HAVE_PS=FALSE
    ])
])
dnl same for PostScript
AS_IF([test "$HAVE_PS" = "TRUE"], [
    PKG_CHECK_MODULES([spectre], [libspectre],
        [
            AC_DEFINE([HAVE_LIBSPECTRE], [1], [Use libspectre])
            CONVERT_LIBS="$CONVERT_LIBS libspectre"
        ], [
        AC_MSG_WARN([libspectre not found; PostScript support disabled])
        HAVE_PS=FALSE
    ])
])

dnl If *none* of the renderers are available just stop. This is because we
dnl cannot perform any conversion without at least one renderer.
AS_IF([test "$HAVE_PDF" = "FALSE" && test "$HAVE_PS" = "FALSE" && test "$HAVE_SVG" = "FALSE"], [
    AC_MSG_ERROR([At least one renderer must be enabled and available for compiling])
])

dnl Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

dnl Checks for library functions.
AC_FUNC_ERROR_AT_LINE

dnl Process R file that contains supported (or lack of) features.
dnl Tells us (in R code) whether a feature is present. 
AC_SUBST(HAVE_PDF)
AC_SUBST(HAVE_PS)
AC_SUBST(HAVE_SVG)
AC_CONFIG_FILES([R/convert.R])
dnl Also don't run code in tests when we don't have the
dnl required feature
AC_CONFIG_FILES([tests/test-pdf-in-ps-out.R])
AC_CONFIG_FILES([tests/test-pdf-in-svg-out.R])
AC_CONFIG_FILES([tests/test-ps-in-ps-out.R])
AC_CONFIG_FILES([tests/test-ps-in-svg-out.R])
AC_CONFIG_FILES([tests/test-svg-in-ps-out.R])
AC_CONFIG_FILES([tests/test-svg-in-svg-out.R])
AC_SUBST(CONVERT_LIBS)
AC_CONFIG_FILES([src/Makevars])
AC_OUTPUT
