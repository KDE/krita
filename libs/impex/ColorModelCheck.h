/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef COLORMODELCHECK_H
#define COLORMODELCHECK_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>

class ColorModelCheck : public KisExportCheckBase
{
public:

    ColorModelCheck(const KoID &colorModelID, const KoID &colorDepthID, const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning)
        , m_colorModelID(colorModelID)
        , m_colorDepthID(colorDepthID)
    {
        Q_ASSERT(!colorModelID.name().isEmpty());
        Q_ASSERT(!colorDepthID.name().isEmpty());

        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning",
                              "The color model <b>%1</b> or channel depth <b>%2</b> cannot be saved to this format. Your image will be converted.",
                              m_colorModelID.name(),
                              m_colorDepthID.name());
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        return (image->colorSpace()->colorModelId() == m_colorModelID && image->colorSpace()->colorDepthId() == m_colorDepthID);
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }

    const KoID m_colorModelID;
    const KoID m_colorDepthID;
};

class ColorModelCheckFactory : public KisExportCheckFactory
{
public:

    ColorModelCheckFactory(const KoID &colorModelID, const KoID &colorDepthId)
        : m_colorModelID(colorModelID)
        , m_colorDepthID(colorDepthId)
    {
    }

    ~ColorModelCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new ColorModelCheck(m_colorModelID, m_colorDepthID, id(), level, customWarning);
    }

    QString id() const override {
        return "ColorModelCheck/" + m_colorModelID.id() + "/" + m_colorDepthID.id();
    }

    const KoID m_colorModelID;
    const KoID m_colorDepthID;
};

#endif // COLORMODELCHECK_H
