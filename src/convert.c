#include "config.h"
#include "convert.h"

/*
 * This function should give us the number of pages that a PDF document
 * contains. This should be useful for retrieving a page to import
 * later.
 */
SEXP pdf_get_n_pages(SEXP filename) {
#if HAVE_LIBPOPPLER
#if !defined(GLIB_VERSION_2_36)
    g_type_init();
#endif
    PopplerDocument *document;
    const char *input_filename;
    gchar *input_uri;
    GError *gerr = NULL;
    int n_pages;

    input_filename = CHAR(STRING_ELT(filename, 0)); 

    input_uri = g_filename_to_uri(input_filename, NULL, &gerr);
    if (input_uri == NULL)
        C_ERROR(gerr->message);

    document = poppler_document_new_from_file(input_uri, NULL, &gerr);
    free(input_uri);
    if (document == NULL)
        C_ERROR(gerr->message);

    n_pages = poppler_document_get_n_pages(document);
    g_object_unref(document);
    return ScalarInteger(n_pages);
#else
    return ScalarInteger(-1); /* This should not be called */
#endif
}

/*
 * This function should give us the number of pages that a PS document
 * contains. This should be useful for retrieving a page to import
 * later.
 */
SEXP ps_get_n_pages(SEXP filename) {
#if HAVE_LIBSPECTRE
    SpectreDocument *document;
    SpectreStatus status;
    const char *input_filename;
    int n_pages;

    input_filename = CHAR(STRING_ELT(filename, 0)); 

    document = spectre_document_new();
    if (!document)
        C_ERROR("unable to load PS image");
    spectre_document_load(document, input_filename);
    status = spectre_document_status(document);
    if (status != SPECTRE_STATUS_SUCCESS)
        C_ERROR(spectre_status_to_string(status));
    n_pages = spectre_document_get_n_pages(document);
    spectre_document_free(document);

    return ScalarInteger(n_pages);
#else
    return ScalarInteger(-1); /* This should not be called */
#endif
}

/*
 * Converts a PS document to a PDF image via libspectre/gs.
 * This is because we cannot render a PS file directly via Cairo, and
 * instead we have to rely on PDF. Fortunately PDF is a reasonably
 * similar image format that we should not expect any major difficulties
 * in this step.
 */
SEXP ps_to_pdf(SEXP ps_file, SEXP pdf_file) {
#if HAVE_LIBSPECTRE
#if !defined(GLIB_VERSION_2_36)
    g_type_init();
#endif
    SpectreDocument *ps_document;
    SpectreStatus status;
    const char *ps_input_file, *pdf_output_file;

    ps_input_file = CHAR(STRING_ELT(ps_file, 0)); 
    pdf_output_file = CHAR(STRING_ELT(pdf_file, 0)); 

    ps_document = spectre_document_new();
    if (!ps_document)
        C_ERROR("unable to load PS image");
    spectre_document_load(ps_document, ps_input_file);
    status = spectre_document_status(ps_document);
    if (status != SPECTRE_STATUS_SUCCESS)
        C_ERROR(spectre_status_to_string(status));
    spectre_document_save_to_pdf(ps_document, pdf_output_file);
    spectre_document_free(ps_document);
#endif
    return R_NilValue;
}

/*
 * Converts a PDF document to a Cairo PS image via libpoppler.
 */
SEXP pdf_to_ps(SEXP pdf_file, SEXP ps_file, SEXP page_num) {
#if HAVE_LIBPOPPLER
#if !defined(GLIB_VERSION_2_36)
    g_type_init();
#endif
    PopplerDocument *document;
    PopplerPage *page;
    GError *gerr = NULL;
    double width, height;
    const char *pdf_input_file, *ps_output_file;
    gchar *pdf_input_uri;
    int input_page_num;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;

    pdf_input_file = CHAR(STRING_ELT(pdf_file, 0)); 
    ps_output_file = CHAR(STRING_ELT(ps_file, 0)); 
    input_page_num = asInteger(page_num);

    pdf_input_uri = g_filename_to_uri(pdf_input_file, NULL, &gerr);
    if (pdf_input_uri == NULL)
        C_ERROR(gerr->message);

    document = poppler_document_new_from_file(pdf_input_uri, NULL, &gerr);
    free(pdf_input_uri);
    if (document == NULL)
        C_ERROR(gerr->message);

    page = poppler_document_get_page(document, input_page_num - 1);
    /* This should not be possible as we check in R for valid pages */
    if (page == NULL)
        C_ERROR("poppler error: page not found");

    poppler_page_get_size(page, &width, &height);
    surface = cairo_ps_surface_create(ps_output_file, width, height);
    status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    cr = cairo_create(surface);
    status = cairo_status(cr);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    poppler_page_render_for_printing(page, cr);
    cairo_surface_show_page(surface);
    g_object_unref(page);
    status = cairo_status(cr);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    cairo_destroy(cr);
    cairo_surface_finish(surface);
    status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS)
        error(cairo_status_to_string(status));
    cairo_surface_destroy(surface);
    g_object_unref(document);
#endif
    return R_NilValue;
}

/*
 * Converts an arbitrary SVG image to a Cairo PS image via librsvg.
 */
SEXP svg_to_ps(SEXP svg_file, SEXP ps_file) {
#if HAVE_LIBRSVG
#if LIBRSVG_MAJOR_VERSION <= 2 && LIBRSVG_MINOR_VERSION < 36
    rsvg_init();
#endif
#if !defined(GLIB_VERSION_2_36)
    g_type_init();
#endif
    RsvgHandle *rsvg;
    GError *gerr = NULL;
    const char *svg_input_file, *ps_output_file;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;
    gboolean success;
    RsvgDimensionData dims;

    svg_input_file = CHAR(STRING_ELT(svg_file, 0)); 
    ps_output_file = CHAR(STRING_ELT(ps_file, 0)); 

    rsvg_set_default_dpi_x_y(-1, -1);

    rsvg = rsvg_handle_new_from_file(svg_input_file, &gerr);
    if (rsvg == NULL)
        C_ERROR(gerr->message);
    rsvg_handle_get_dimensions(rsvg, &dims);
    
    surface = cairo_ps_surface_create(ps_output_file,
                                      dims.width, dims.height);
    status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    cr = cairo_create(surface);
    status = cairo_status(cr);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    success = rsvg_handle_render_cairo(rsvg, cr);
    if (!success)
        C_ERROR("rsvg failed to draw to a cairo surface");
    success = rsvg_handle_close(rsvg, &gerr);
    if (! success)
        C_ERROR(gerr->message);
    g_object_unref(rsvg);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
#endif
    return R_NilValue;
}

/*
 * Converts a PDF document to a Cairo SVG image via libpoppler.
 */
SEXP pdf_to_svg(SEXP pdf_file, SEXP svg_file, SEXP page_num) {
#if HAVE_LIBPOPPLER
#if !defined(GLIB_VERSION_2_36)
    g_type_init();
#endif
    PopplerDocument *document;
    PopplerPage *page;
    GError *gerr = NULL;
    double width, height;
    const char *pdf_input_file, *svg_output_file;
    gchar *pdf_input_uri;
    int input_page_num;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;

    pdf_input_file = CHAR(STRING_ELT(pdf_file, 0)); 
    svg_output_file = CHAR(STRING_ELT(svg_file, 0)); 
    input_page_num = asInteger(page_num);

    pdf_input_uri = g_filename_to_uri(pdf_input_file, NULL, &gerr);
    if (pdf_input_uri == NULL)
        C_ERROR(gerr->message);

    document = poppler_document_new_from_file(pdf_input_uri, NULL, &gerr);
    free(pdf_input_uri);
    if (document == NULL)
        C_ERROR(gerr->message);

    page = poppler_document_get_page(document, input_page_num - 1);
    /* This should not be possible as we check in R for valid pages */
    if (page == NULL)
        C_ERROR("poppler error: page not found");

    poppler_page_get_size(page, &width, &height);
    surface = cairo_svg_surface_create(svg_output_file, width, height);
    status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    cr = cairo_create(surface);
    status = cairo_status(cr);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    poppler_page_render_for_printing(page, cr);
    cairo_surface_show_page(surface);
    g_object_unref(page);
    status = cairo_status(cr);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    cairo_destroy(cr);
    cairo_surface_finish(surface);
    status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    cairo_surface_destroy(surface);
    g_object_unref(document);
#endif
    return R_NilValue;
}

/*
 * Converts an arbitrary SVG image to a Cairo SVG image via librsvg.
 */
SEXP svg_to_svg(SEXP svg_file, SEXP cairo_svg_file) {
#if HAVE_LIBRSVG
#if LIBRSVG_MAJOR_VERSION <= 2 && LIBRSVG_MINOR_VERSION < 36
    rsvg_init();
#endif
#if !defined(GLIB_VERSION_2_36)
    g_type_init();
#endif
    RsvgHandle *rsvg;
    GError *gerr = NULL;
    const char *svg_input_file, *svg_output_file;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;
    gboolean success;
    RsvgDimensionData dims;

    svg_input_file = CHAR(STRING_ELT(svg_file, 0)); 
    svg_output_file = CHAR(STRING_ELT(cairo_svg_file, 0)); 

    rsvg_set_default_dpi_x_y(-1, -1);

    rsvg = rsvg_handle_new_from_file(svg_input_file, &gerr);
    if (rsvg == NULL)
        C_ERROR(gerr->message);
    rsvg_handle_get_dimensions(rsvg, &dims);
    
    surface = cairo_svg_surface_create(svg_output_file,
                                       dims.width, dims.height);
    status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    cr = cairo_create(surface);
    status = cairo_status(cr);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
    success = rsvg_handle_render_cairo(rsvg, cr);
    if (!success)
        C_ERROR("rsvg failed to draw to a cairo surface");
    success = rsvg_handle_close(rsvg, &gerr);
    if (!success)
        C_ERROR(gerr->message);
    g_object_unref(rsvg);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS)
        C_ERROR(cairo_status_to_string(status));
#endif
    return R_NilValue;
}
