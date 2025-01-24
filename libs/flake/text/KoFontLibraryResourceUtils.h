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

#include <kis_debug.h>

namespace detail {

template <typename T, int (*P)(T *)>
void checkCStyleResultWrapper(T *ptr)
{
    const int result = P(ptr);
    if (result != 0) {
        qWarning() << "WARNING: failed to release a library resource";
#ifdef __GNUC__
        qWarning() << "    source:" << __PRETTY_FUNCTION__;
#endif
    }
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
        : QSharedPointer<T>(nullptr)
    {
    }

    KisLibraryResourcePointer(T *ptr)
        : QSharedPointer<T>(ptr, ptr ? P : &KisLibraryResourcePointer::noDestroy)
    {
    }

    using QSharedPointer<T>::operator->;

    void reset(T *ptr) {
        QSharedPointer<T>::reset(ptr, ptr ? P : &KisLibraryResourcePointer::noDestroy);
    }

    void reset() {
        QSharedPointer<T>::reset();
    }

    auto data() const
    {
        return this->get();
    }

private:
    static void noDestroy(T *ptr) {
        Q_UNUSED(ptr);
    }
};

/**
 * A special version of a class for resource types whose destructor
 * returns integer value reporting if destruction was successful or
 * now. We wrap this destructor to spit a warning in case of a trouble
 * to release the type.
 */
template<typename T, int (*P)(T *)>
using KisLibraryResourcePointerWithSanityCheck = KisLibraryResourcePointer<T, detail::checkCStyleResultWrapper<T, P>>;


using FcConfigSP = KisLibraryResourcePointer<FcConfig, FcConfigDestroy>;
using FcCharSetSP = KisLibraryResourcePointer<FcCharSet, FcCharSetDestroy>;
using FcPatternSP = KisLibraryResourcePointer<FcPattern, FcPatternDestroy>;
using FcFontSetSP = KisLibraryResourcePointer<FcFontSet, FcFontSetDestroy>;

using FT_LibrarySP =
    KisLibraryResourcePointerWithSanityCheck<std::remove_pointer_t<FT_Library>,
                                             FT_Done_FreeType>;
using FT_FaceSP = KisLibraryResourcePointerWithSanityCheck<std::remove_pointer_t<FT_Face>,
                                                           FT_Done_Face>;

using hb_font_t_sp = KisLibraryResourcePointer<hb_font_t, hb_font_destroy>;

#endif // KOFONTLIBRARYRESOURCEUTILS_H
