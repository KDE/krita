/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2008-10-09
 * @brief  internal private container for KDcraw
 *
 * @author Copyright (C) 2008-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef KDCRAWPRIVATE_H
#define KDCRAWPRIVATE_H

// Qt includes

#include <QByteArray>

// Pragma directives to reduce warnings from LibRaw header files.
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

// LibRaw includes

#include <libraw.h>

// Restore warnings
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic pop
#endif

// Local includes

#include "dcrawinfocontainer.h"
#include "kdcraw.h"

namespace KDcrawIface
{

extern "C"
{
    int callbackForLibRaw(void* data, enum LibRaw_progress p, int iteration, int expected);
}

class Q_DECL_HIDDEN KDcraw::Private
{

public:

    Private(KDcraw* const p);
    ~Private();

public:

    int    progressCallback(enum LibRaw_progress p, int iteration, int expected);

    void   setProgress(double value);
    double progressValue() const;

    bool   loadFromLibraw(const QString& filePath, QByteArray& imageData,
                          int& width, int& height, int& rgbmax);

public:

    static void createPPMHeader(QByteArray& imgData, libraw_processed_image_t* const img);

    static void fillIndentifyInfo(LibRaw* const raw, DcrawInfoContainer& identify);

    static bool loadEmbeddedPreview(QByteArray&, LibRaw&);

    static bool loadHalfPreview(QImage&, LibRaw&);

private:

    double  m_progress;

    KDcraw* m_parent;

    friend class KDcraw;
};

}  // namespace KDcrawIface

#endif /* KDCRAWPRIVATE_H */
