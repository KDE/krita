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

#ifndef sRGBProfileCheck_H
#define sRGBProfileCheck_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>

/**
 * Check whether the image is sRGB, so the loss of the profile is not a problem
 */
class sRGBProfileCheck : public KisExportCheckBase
{
public:

    sRGBProfileCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "The image is not tagged as <b>non-linear gamma sRGB</b>. The image will be converted to sRGB.");
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        bool sRGB = image->colorSpace()->profile()->name().contains(QLatin1String("srgb"), Qt::CaseInsensitive);

        // XXX: add an isLinear function to KoColorProfile that uses the information already available through lcms
        bool linear = image->colorSpace()->profile()->name().contains(QLatin1String("g10"), Qt::CaseInsensitive);

        return (!sRGB || linear);
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }

};

class sRGBProfileCheckFactory : public KisExportCheckFactory
{
public:

    sRGBProfileCheckFactory()
    {
    }

    ~sRGBProfileCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new sRGBProfileCheck(id(), level, customWarning);
    }

    QString id() const override {
        return "sRGBProfileCheck";
    }
};

#endif // sRGBProfileCheck_H
