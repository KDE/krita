
#ifndef KRITATELEMETRY_EXPORT_H
#define KRITATELEMETRY_EXPORT_H

#ifdef KRITATELEMETRY_STATIC_DEFINE
#  define KRITATELEMETRY_EXPORT
#  define KRITATELEMETRY_NO_EXPORT
#else
#  ifndef KRITATELEMETRY_EXPORT
#    ifdef kritaTELEMETRY_EXPORTS
        /* We are building this library */
#      define KRITATELEMETRY_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define KRITATELEMETRY_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef KRITATELEMETRY_NO_EXPORT
#    define KRITATELEMETRY_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef KRITATELEMETRY_DEPRECATED
#  define KRITATELEMETRY_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef KRITATELEMETRY_DEPRECATED_EXPORT
#  define KRITATELEMETRY_DEPRECATED_EXPORT KRITATELEMETRY_EXPORT KRITATELEMETRY_DEPRECATED
#endif

#ifndef KRITATELEMETRY_DEPRECATED_NO_EXPORT
#  define KRITATELEMETRY_DEPRECATED_NO_EXPORT KRITATELEMETRY_NO_EXPORT KRITATELEMETRY_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define KRITATELEMETRY_NO_DEPRECATED
#endif

#endif

