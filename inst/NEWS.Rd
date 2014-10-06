\name{NEWS}
\title{NEWS file for the grConvert package}
\encoding{UTF-8}

\section{Changes in version 0.1-0}{
  \itemize{

    \item Initial release. Intended to generate PostScript and SVG
    images for use with the the 'grImport' and 'grImport2' packages
    respectively.

    \item Currently supports PostScript, PDF and SVG inputs, assuming
    that the platform that the package is installed on includes the
    libraries *and* headers for libspectre (PS), libpoppler (PDF), and
    librsvg (SVG). Cairo is mandatory.

  }
}
