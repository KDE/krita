/**
  @mainpage Krita Image manipulation and paint application

  Krita is an advanced and modular paint and image manipulation
  application.

  Krita is built around two core libraries: kritacolor and kritaimage.

  The kritacolor library abstracts colorspaces and color
  transformations. Colorspaces provide functions to manipulate pixels. The
  kritcolor library loads colorspace plugins to extend the range of
  available colorspaces.

  The kritaimage library abstracts the storage, creation, inspection
  and manipulation of pixels stored in a rectangular area. It provides
  layers, filters, iterators and painters. Filters and paint operations
  are provided as service plugins loaded through the appropriate trader
  queries.

  Both libraries are used by the user interface, which is a KOffice
  part. the user interface loads tools and other plugins.

 */
#ifndef DESIGN
#define DESIGN
// Let's keep icefox.net/kde/tests.headerincluded_koffice.html happy
#endif