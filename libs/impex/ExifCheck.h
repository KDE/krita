/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef ExifCHECK_H
#define ExifCHECK_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_assert.h>
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

class TiffExifCheck : public KisExportCheckBase
{
public:
    TiffExifCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning",
                              "The image has <b>Exif</b> metadata and multiple layers. Only metadata <b>in the first "
                              "layer</b> will be saved.");
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        KIS_ASSERT_RECOVER_RETURN_VALUE(image->rootLayer(), false);
        KisExifInfoVisitor eIV;
        eIV.visit(image->rootLayer().data());
        return eIV.exifInfo() && image->rootLayer()->childCount() > 1;
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }
};

class TiffExifCheckFactory : public KisExportCheckFactory
{
public:
    TiffExifCheckFactory()
    {
    }

    ~TiffExifCheckFactory() override
    {
    }

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new TiffExifCheck(id(), level, customWarning);
    }

    QString id() const override
    {
        return "TiffExifCheck";
    }
};

#endif // ExifCHECK_H
