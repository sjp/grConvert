\name{grConvert Capabilities}
\alias{grConvert.capabilities}
\title{
  Query the supported input formats for grConvert
}
\description{
  In order to read in an image, grConvert requires that an associated
  renderer is available for use. This is determined when grConvert is
  installed and by default it will attempt to be able to read in all
  vector image formats. This function shows how many vector images
  formats can actually be read in by grConvert so that it may be
  converted to either PostScript or SVG.
}
\usage{
grConvert.capabilities(print = TRUE)
}
\arguments{
  \item{print}{
    If \code{TRUE}, prints out the supported input formats for the
    \code{\link{convertPicture}} function.
  }
}
\value{
  A list keyed by input formats (PS, PDF and SVG), whose associated
  values are \code{TRUE} if grConvert can read in the format, or
  \code{FALSE} if not.

  This list is always returned, but it is returned invisibly when
  \code{print} is \code{FALSE}.
}
\author{
  Simon Potter
}
\seealso{
  \code{\link{convertPicture}}
}
