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
