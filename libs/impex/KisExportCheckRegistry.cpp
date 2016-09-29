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

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>

class ColorModelCheck : public KisExportCheckBase
{
public:

    ColorModelCheck(const KoID &colorModelID, const KoID &colorDepthId, const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning)
        ,  m_colorModelID(colorModelID)
        , m_colorDepthID(colorDepthId)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "The color model %1 or channel depth %2 cannot be saved to this format. Your image will be converted.").arg(m_colorModelID.name()).arg(m_colorDepthID.name());
        }
    }

    bool checkNeeded(KisImageSP image) const
    {
        return (image->colorSpace()->colorModelId() == m_colorModelID && image->colorSpace()->colorDepthId() == m_colorDepthID);
    }

    Level check(KisImageSP /*image*/) const
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

    virtual ~ColorModelCheckFactory() {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning)
    {
        return new ColorModelCheck(m_colorModelID, m_colorDepthID, id(), level, customWarning);
    }

    QString id() const {
        return "ColorModelCheck/" + m_colorModelID.id() + "/" + m_colorDepthID.id();
    }

    const KoID m_colorModelID;
    const KoID m_colorDepthID;
};


#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisExportCheckRegistry, s_instance)

KisExportCheckRegistry::KisExportCheckRegistry ()
{
    KisExportCheckFactory *check = 0;

    check = new ColorModelCheckFactory(AlphaColorModelID, Integer8BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(AlphaColorModelID, Integer16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(AlphaColorModelID, Float16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(AlphaColorModelID, Float32BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(AlphaColorModelID, Float64BitsColorDepthID);
    add(check->id(), check);

    check = new ColorModelCheckFactory(RGBAColorModelID, Integer8BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(RGBAColorModelID, Integer16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(RGBAColorModelID, Float16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(RGBAColorModelID, Float32BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(RGBAColorModelID, Float64BitsColorDepthID);
    add(check->id(), check);

    check = new ColorModelCheckFactory(XYZAColorModelID, Integer8BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(XYZAColorModelID, Integer16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(XYZAColorModelID, Float16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(XYZAColorModelID, Float32BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(XYZAColorModelID, Float64BitsColorDepthID);
    add(check->id(), check);

    check = new ColorModelCheckFactory(LABAColorModelID, Integer8BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(LABAColorModelID, Integer16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(LABAColorModelID, Float16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(LABAColorModelID, Float32BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(LABAColorModelID, Float64BitsColorDepthID);
    add(check->id(), check);

    check = new ColorModelCheckFactory(CMYKAColorModelID, Integer8BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(CMYKAColorModelID, Integer16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(CMYKAColorModelID, Float16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(CMYKAColorModelID, Float32BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(CMYKAColorModelID, Float64BitsColorDepthID);
    add(check->id(), check);

    check = new ColorModelCheckFactory(GrayAColorModelID, Integer8BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(GrayAColorModelID, Integer16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(GrayAColorModelID, Float16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(GrayAColorModelID, Float32BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(GrayAColorModelID, Float64BitsColorDepthID);
    add(check->id(), check);

    check = new ColorModelCheckFactory(GrayColorModelID, Integer8BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(GrayColorModelID, Integer16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(GrayColorModelID, Float16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(GrayColorModelID, Float32BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(GrayColorModelID, Float64BitsColorDepthID);
    add(check->id(), check);

    check = new ColorModelCheckFactory(YCbCrAColorModelID, Integer8BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(YCbCrAColorModelID, Integer16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(YCbCrAColorModelID, Float16BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(YCbCrAColorModelID, Float32BitsColorDepthID);
    add(check->id(), check);
    check = new ColorModelCheckFactory(YCbCrAColorModelID, Float64BitsColorDepthID);
    add(check->id(), check);

}

KisExportCheckRegistry::~KisExportCheckRegistry ()
{
    qDeleteAll(values());
}

KisExportCheckRegistry *KisExportCheckRegistry ::instance()
{
    return s_instance;
}

