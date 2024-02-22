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
 * The only difference is that the deleter supports being called
 * for null pointer, so we can call it directly without any wrapper.
 */
template<typename T, int (*P)(T *)>
struct KisLibraryResourcePointerAllowsNull : private QSharedPointer<T> {
public:
    KisLibraryResourcePointerAllowsNull()
        : QSharedPointer<T>(nullptr, P)
    {
    }

    KisLibraryResourcePointerAllowsNull(T *ptr)
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

using FcConfigSP = KisLibraryResourcePointer<FcConfig, FcConfigDestroy>;
using FcCharSetSP = KisLibraryResourcePointer<FcCharSet, FcCharSetDestroy>;
using FcPatternSP = KisLibraryResourcePointer<FcPattern, FcPatternDestroy>;
using FcFontSetSP = KisLibraryResourcePointer<FcFontSet, FcFontSetDestroy>;
using FT_LibrarySP = KisLibraryResourcePointerAllowsNull<std::remove_pointer_t<FT_Library>, FT_Done_FreeType>;
using FT_FaceSP = KisLibraryResourcePointerAllowsNull<std::remove_pointer_t<FT_Face>, FT_Done_Face>;

using hb_font_t_sp = KisLibraryResourcePointer<hb_font_t, hb_font_destroy>;

#endif // KOFONTLIBRARYRESOURCEUTILS_H
