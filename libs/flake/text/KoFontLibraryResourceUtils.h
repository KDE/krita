#ifndef KOFONTLIBRARYRESOURCEUTILS_H
#define KOFONTLIBRARYRESOURCEUTILS_H

#include <KisLibraryResourcePointer.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H
#include <fontconfig/fontconfig.h>

#include <hb.h>
#include <hb-ft.h>
#include <hb-ot.h>

#include <raqm.h>

namespace resource_detail {

template <>
inline void freeResource<FcCharSet>(FcCharSet *res) {
    FcCharSetDestroy(res);
}

template <>
inline void freeResource<FcPattern>(FcPattern *res) {
    FcPatternDestroy(res);
}

template <>
inline void freeResource<FcFontSet>(FcFontSet *res) {
    FcFontSetDestroy(res);
}

template <>
inline void freeResource<std::remove_pointer_t<FT_Library>>(FT_Library res) {
    FT_Done_FreeType(res);
}

template <>
inline void freeResource<std::remove_pointer_t<FT_Face>>(FT_Face res) {
    FT_Done_Face(res);
}

template <>
inline void freeResource<hb_font_t>(hb_font_t *res) {
    hb_font_destroy(res);
}

template <>
inline void freeResource<raqm_t>(raqm_t *res) {
    raqm_destroy(res);
}


}

/**
 * unique-pointer-like data types to hold the font
 * library resources
 */

using FcCharSetUP = KisLibraryResourcePointer<FcCharSet>;
using FcPatternUP = KisLibraryResourcePointer<FcPattern>;
using FcFontSetUP = KisLibraryResourcePointer<FcFontSet>;
using FT_LibraryUP = KisLibraryResourcePointer<std::remove_pointer_t<FT_Library>>;
using FT_FaceUP = KisLibraryResourcePointer<std::remove_pointer_t<FT_Face>>;

using hb_font_t_up = KisLibraryResourcePointer<hb_font_t>;
using raqm_t_up = KisLibraryResourcePointer<raqm_t>;

inline FT_LibraryUP toLibraryResource(FT_Library resource) {
    return FT_LibraryUP(resource);
}

inline FT_FaceUP toLibraryResource(FT_Face resource) {
    return FT_FaceUP(resource);
}

#endif // KOFONTLIBRARYRESOURCEUTILS_H
