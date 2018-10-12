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

#ifndef ExifCHECK_H
#define ExifCHECK_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_filter_registry_model.h>
#include <kis_exif_info_visitor.h>

class ExifCheck : public KisExportCheckBase
{
public:

    ExifCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "The image contains <b>Exif</b> metadata. The metadata will not be saved.");
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        KisExifInfoVisitor eIV;
        eIV.visit(image->rootLayer().data());
        return eIV.exifInfo();
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }

};

class ExifCheckFactory : public KisExportCheckFactory
{
public:

    ExifCheckFactory()
    {
    }

    ~ExifCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new ExifCheck(id(), level, customWarning);
    }

    QString id() const override {
        return "ExifCheck";
    }

};

#endif // ExifCHECK_H
