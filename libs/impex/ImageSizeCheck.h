/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CHECKIMAGESIZE_H
#define CHECKIMAGESIZE_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include "kritaimpex_export.h"

class KRITAIMPEX_EXPORT ImageSizeCheck : public KisExportCheckBase
{
public:

    ImageSizeCheck(int maxWidth, int maxHeight, const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning, true)
        , m_maxW(maxWidth)
        , m_maxH(maxHeight)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "This image is larger than <b>%1 x %2</b>. Images this size cannot be saved to this format.", m_maxW, m_maxH);
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        return image->width() >= m_maxW && image->height() >= m_maxH;
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }

    int m_maxW;
    int m_maxH;
};

class KRITAIMPEX_EXPORT ImageSizeCheckFactory : public KisExportCheckFactory
{
public:

    ImageSizeCheckFactory() {}

    ~ImageSizeCheckFactory() override {}

    KisExportCheckBase *create( KisExportCheckBase::Level level, const QString &customWarning = QString()) override
    {
        return new ImageSizeCheck(100000000, 100000000, id(), level, customWarning);
    }

    KisExportCheckBase *create(int maxWidth, int maxHeight, KisExportCheckBase::Level level, const QString &customWarning = QString())
    {
        return new ImageSizeCheck(maxWidth, maxHeight, id(), level, customWarning);
    }

    QString id() const override {
        return "ImageSizeCheck";
    }
};


#endif // CHECKIMAGESIZE_H
