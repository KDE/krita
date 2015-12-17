// Check windows
#ifdef Q_OS_WIN
   #if _WIN64
     #define ENV64BIT
  #else
    #define ENV32BIT
  #endif
#endif

// Check GCC
#if __GNUC__
  #if defined (__x86_64__) || defined (__ppc64__)
    #define ENV64BIT
  #else
    #define ENV32BIT
  #endif
#endif

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

/* Defines if we use lcms2 */
#cmakedefine HAVE_LCMS2 1

/* Defines if we use lcms2.4 */
#cmakedefine HAVE_LCMS24 1

/* Defines if KIO is present */
#cmakedefine HAVE_KIO 1

/* Defines if DBUS is present */
#cmakedefine HAVE_DBUS 1

/* Defines if KCrash is present */
#cmakedefine HAVE_KCRASH 1

/* This file contains all the paths that change when changing the installation prefix */
#define CALLIGRAPREFIX "${CMAKE_INSTALL_PREFIX}"

/* This variable contains the path to the current build directory */
#define KRITA_BUILD_DIR "${CMAKE_BINARY_DIR}"
