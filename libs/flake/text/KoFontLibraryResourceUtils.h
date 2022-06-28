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

#include <hb.h>
#include <hb-ft.h>
#include <hb-ot.h>

#include <raqm.h>

#include <QSharedPointer>

/**
 * Shared pointer that holds a standard allocated resource.
 */
template<typename T, void (*P)(T *)>
struct KisLibraryResourcePointer : private QSharedPointer<T> {
public:
    KisLibraryResourcePointer()
        : QSharedPointer<T>(nullptr, P)
    {
    }

    KisLibraryResourcePointer(T *ptr)
        : QSharedPointer<T>(ptr, P)
    {
    }

    using QSharedPointer<T>::operator->;
    using QSharedPointer<T>::reset;

    void reset(T *ptr)
    {
        QSharedPointer<T>::reset(ptr, P);
    }

    auto data()
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

    void reset(T *ptr)
    {
        QSharedPointer<T>::reset(ptr, P);
    }

    auto data() const
    {
        return this->get();
    }
};

using FcCharSetUP = KisLibraryResourcePointer<FcCharSet, FcCharSetDestroy>;
using FcPatternUP = KisLibraryResourcePointer<FcPattern, FcPatternDestroy>;
using FcFontSetUP = KisLibraryResourcePointer<FcFontSet, FcFontSetDestroy>;
using FT_LibraryUP =
    KisFreeTypeResourcePointer<std::remove_pointer_t<FT_Library>,
                               FT_Done_FreeType>;
using FT_FaceUP =
    KisFreeTypeResourcePointer<std::remove_pointer_t<FT_Face>, FT_Done_Face>;

using hb_font_t_up = KisLibraryResourcePointer<hb_font_t, hb_font_destroy>;
using raqm_t_up = KisLibraryResourcePointer<raqm_t, raqm_destroy>;

#endif // KOFONTLIBRARYRESOURCEUTILS_H
