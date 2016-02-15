/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_predefined_brush_factory.h"

#include <QDomDocument>
#include "kis_brush_server.h"
#include "kis_gbr_brush.h"


KisPredefinedBrushFactory::KisPredefinedBrushFactory(const QString &brushType)
    : m_id(brushType)
{
}

QString KisPredefinedBrushFactory::id() const
{
    return m_id;
}

KisBrushSP KisPredefinedBrushFactory::getOrCreateBrush(const QDomElement& brushDefinition)
{
    KisBrushResourceServer *rServer = KisBrushServer::instance()->brushServer();
    QString brushFileName = brushDefinition.attribute("filename", "");
    KisBrushSP brush = rServer->resourceByFilename(brushFileName);

    //Fallback for files that still use the old format
    if (!brush) {
        QFileInfo info(brushFileName);
        brush = rServer->resourceByFilename(info.fileName());
    }

    if (!brush) {
        brush = rServer->resources().first();
    }

    Q_ASSERT(brush);

    double spacing = brushDefinition.attribute("spacing", "0.25").toDouble();
    brush->setSpacing(spacing);

    bool useAutoSpacing = brushDefinition.attribute("useAutoSpacing", "0").toInt();
    qreal autoSpacingCoeff = brushDefinition.attribute("autoSpacingCoeff", "1.0").toDouble();
    brush->setAutoSpacing(useAutoSpacing, autoSpacingCoeff);

    double angle = brushDefinition.attribute("angle", "0.0").toDouble();
    brush->setAngle(angle);

    double scale = brushDefinition.attribute("scale", "1.0").toDouble();
    brush->setScale(scale);

    if (m_id == "gbr_brush") {
        KisGbrBrush *gbrbrush = dynamic_cast<KisGbrBrush*>(brush.data());
        if (gbrbrush) {
            /**
             * WARNING: see comment in KisGbrBrush::setUseColorAsMask()
             */
            gbrbrush->setUseColorAsMask((bool)brushDefinition.attribute("ColorAsMask").toInt());
        }
    }

    return brush;
}
