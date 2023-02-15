/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOFONTLIBRARYRESOURCEUTILS_H
#define KOFONTLIBRARYRESOURCEUTILS_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H
#include <fontconfig/fontconfig.h>

#include <hb-ft.h>
#include <hb-ot.h>
#include <hb.h>

#include <QSharedPointer>

// Helper to clean up only if the pointer is non-null.
template<typename T, void (*d)(T *)>
inline void deleter(T *ptr)
{
    if (ptr) {
        d(ptr);
    }
}

/**
 * Shared pointer that holds a standard allocated resource.
 * We use a wrapper because by C++ standards it calls the deleter
 * unconditionally. This leads to crashes on FontConfig:
 * https://invent.kde.org/graphics/krita/-/merge_requests/1607#note_567848
 */
template<typename T, void (*P)(T *)>
struct KisLibraryResourcePointer : private QSharedPointer<T> {
public:
    KisLibraryResourcePointer()
        : QSharedPointer<T>(nullptr, deleter<T, P>)
    {
    }

    KisLibraryResourcePointer(T *ptr)
        : QSharedPointer<T>(ptr, deleter<T, P>)
    {
    }

    using QSharedPointer<T>::operator->;
    using QSharedPointer<T>::reset;

    auto data() const
    {
        return this->get();
    }
};

/**
 * Shared pointer that holds a standard allocated resource.
 * The only difference is in the signature of the deleter.
 */
template<typename T, int (*P)(T *)>
struct KisFreeTypeResourcePointer : private QSharedPointer<T> {
public:
    KisFreeTypeResourcePointer()
        : QSharedPointer<T>(nullptr, P)
    {
    }

    KisFreeTypeResourcePointer(T *ptr)
        : QSharedPointer<T>(ptr, P)
    {
    }

    using QSharedPointer<T>::operator->;
    using QSharedPointer<T>::reset;

    auto data() const
    {
        return this->get();
    }
};

using FcConfigUP = KisLibraryResourcePointer<FcConfig, FcConfigDestroy>;
using FcCharSetUP = KisLibraryResourcePointer<FcCharSet, FcCharSetDestroy>;
using FcPatternUP = KisLibraryResourcePointer<FcPattern, FcPatternDestroy>;
using FcFontSetUP = KisLibraryResourcePointer<FcFontSet, FcFontSetDestroy>;
using FT_LibraryUP = KisFreeTypeResourcePointer<std::remove_pointer_t<FT_Library>, FT_Done_FreeType>;
using FT_FaceUP = KisFreeTypeResourcePointer<std::remove_pointer_t<FT_Face>, FT_Done_Face>;

using hb_font_t_up = KisLibraryResourcePointer<hb_font_t, hb_font_destroy>;

#endif // KOFONTLIBRARYRESOURCEUTILS_H
