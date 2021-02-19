// Check windows
#ifdef Q_OS_WIN
   #ifdef _WIN64
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

/* Defines if the Dr. Mingw crash handler should be used */
#cmakedefine USE_DRMINGW 1

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Defines if your system has the OpenEXR library */
#cmakedefine HAVE_OPENEXR 1

/* Defines if we use lcms2 */
#cmakedefine HAVE_LCMS2 1

/* Defines if we use lcms2.4 */
#cmakedefine HAVE_LCMS24 1

/* Defines if we can use LittleCMS's fast float plugin. (GPLv3) */
#cmakedefine HAVE_LCMS_FAST_FLOAT_PLUGIN 1

/* Defines if DBUS is present */
#cmakedefine HAVE_DBUS 1

/* Defines if KCrash is present */
#cmakedefine HAVE_KCRASH 1

/* This variable contains the path to the root of the build directory */
#define KRITA_BUILD_DIR "${CMAKE_BINARY_DIR}"

/* This variable contains the path to the root of the source directory */
#define KRITA_SOURCE_DIR "${CMAKE_SOURCE_DIR}"

/* This variable contains the path to the data install dir */
#define KRITA_EXTRA_RESOURCE_DIRS "${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}:${CMAKE_SOURCE_DIR}/krita/data"
