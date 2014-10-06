#include <stdio.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

/*
 * Conditionally include headers based on whether we want to apply
 * the related feature
 */
#if HAVE_LIBPOPPLER
#include <poppler/glib/poppler.h>
#endif
#if HAVE_LIBSPECTRE
#include <libspectre/spectre.h>
#endif
#if HAVE_LIBRSVG
#include <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#include <librsvg/rsvg-cairo.h>
#endif
#endif

#include <cairo.h>
#include <cairo-ps.h>
#include <cairo-svg.h>

#include <R.h>
#define USE_RINTERNALS 1
#include <Rinternals.h>

/* Defining custom error macro so we know where to look when debugging */
#define C_ERROR(x) error("ERROR %s:%d: %s", __FILE__, __LINE__, x)

SEXP pdf_get_n_pages(SEXP filename);
SEXP ps_get_n_pages(SEXP filename);
SEXP ps_to_pdf(SEXP ps_file, SEXP pdf_file);
SEXP pdf_to_ps(SEXP pdf_file, SEXP ps_file, SEXP page_num);
SEXP svg_to_ps(SEXP svg_file, SEXP cairo_ps_file);
SEXP pdf_to_svg(SEXP pdf_file, SEXP svgfile, SEXP page_num);
SEXP svg_to_svg(SEXP svg_file, SEXP cairo_svg_file);
