#ifdef __APPLE__
# ifdef __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# else
#  undef WORDS_BIGENDIAN
# endif
#else
/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#cmakedefine WORDS_BIGENDIAN ${CMAKE_WORDS_BIGENDIAN}
#endif

/* Defines if you Get Hot New Stuff support */
#cmakedefine GHNS 1

/* Defines if you have Nepomuk support */
#cmakedefine NEPOMUK 1

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Defines if your system has the OpenEXR library */
#cmakedefine HAVE_OPENEXR 1

/* Defines if you have GL (Mesa, OpenGL, ...) and Qt GL support */
#cmakedefine HAVE_OPENGL 1
/* This file contains all the paths that change when changing the installation prefix */

#define CALLIGRAPREFIX "${CMAKE_INSTALL_PREFIX}"

