/*
 * SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef HDRMETADATACHECK_H
#define HDRMETADATACHECK_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <kis_hdr_metadata.h>

class HdrMetaDataCheck : public KisExportCheckBase
{
public:
    HdrMetaDataCheck(const QString &id, Level level, const QString &customWarning = QString())
         : KisExportCheckBase(id, level, customWarning, true)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "This image has <b>HDR metadata</b>. HDR metadata cannot be saved to this format.");
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        return image->relativeContentLightLevelInformation() || image->colorVolumeInformation() || image->hdrReferenceWhiteLightLevel();
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }
};

class HdrMetaDataCheckFactory : public KisExportCheckFactory
{
public:

    HdrMetaDataCheckFactory() {}

    ~HdrMetaDataCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new HdrMetaDataCheck(id(), level, customWarning);
    }

    QString id() const override {
        return "HdrMetaDataCheckFactory";
    }
};

#endif // HDRMETADATACHECK_H
