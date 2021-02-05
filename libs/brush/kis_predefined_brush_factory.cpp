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
#include "kis_png_brush.h"
#include <kis_dom_utils.h>

KisPredefinedBrushFactory::KisPredefinedBrushFactory(const QString &brushType)
    : m_id(brushType)
{
}

QString KisPredefinedBrushFactory::id() const
{
    return m_id;
}

KisBrushSP KisPredefinedBrushFactory::createBrush(const QDomElement& brushDefinition)
{
    KisBrushResourceServer *rServer = KisBrushServer::instance()->brushServer();
    QString brushFileName = brushDefinition.attribute("filename", "");
    KisBrushSP brush = rServer->resourceByFilename(brushFileName);

    bool brushtipFound = true;
    //Fallback for files that still use the old format
    if (!brush) {
        QFileInfo info(brushFileName);
        brush = rServer->resourceByFilename(info.fileName());
    }

    if (!brush) {
        brush = rServer->resources().first();
        brushtipFound = false;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(brush, 0);

    // we always return a copy of the brush!

    brush = brush->clone();

    double spacing = KisDomUtils::toDouble(brushDefinition.attribute("spacing", "0.25"));
    brush->setSpacing(spacing);

    bool useAutoSpacing = KisDomUtils::toInt(brushDefinition.attribute("useAutoSpacing", "0"));
    qreal autoSpacingCoeff = KisDomUtils::toDouble(brushDefinition.attribute("autoSpacingCoeff", "1.0"));
    brush->setAutoSpacing(useAutoSpacing, autoSpacingCoeff);

    double angle = KisDomUtils::toDouble(brushDefinition.attribute("angle", "0.0"));
    brush->setAngle(angle);

    double scale = KisDomUtils::toDouble(brushDefinition.attribute("scale", "1.0"));
    brush->setScale(scale);

    KisColorfulBrush *colorfulBrush = dynamic_cast<KisColorfulBrush*>(brush.data());
    if (colorfulBrush) {
        /**
         * WARNING: see comment in KisColorfulBrush::setUseColorAsMask()
         */
        colorfulBrush->setAdjustmentMidPoint(brushDefinition.attribute("AdjustmentMidPoint", "127").toInt());
        colorfulBrush->setBrightnessAdjustment(brushDefinition.attribute("BrightnessAdjustment").toDouble());
        colorfulBrush->setContrastAdjustment(brushDefinition.attribute("ContrastAdjustment").toDouble());
    }
    if (!brushtipFound) {
        brush->setBrushApplication(ALPHAMASK);
    } 
    else if (brushDefinition.hasAttribute("preserveLightness")) {
        const int preserveLightness = KisDomUtils::toInt(brushDefinition.attribute("preserveLightness", "0"));
        const bool useColorAsMask = (bool)brushDefinition.attribute("ColorAsMask", "1").toInt();

        brush->setBrushApplication(preserveLightness ? LIGHTNESSMAP : 
                    colorfulBrush && colorfulBrush->hasColorAndTransparency() && !useColorAsMask ? IMAGESTAMP : ALPHAMASK);
    }
    else if (brushDefinition.hasAttribute("brushApplication")) {
        enumBrushApplication brushApplication = static_cast<enumBrushApplication>(KisDomUtils::toInt(brushDefinition.attribute("brushApplication", "0")));
        brush->setBrushApplication(brushApplication);
    }
    else if (brushDefinition.hasAttribute("ColorAsMask")) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(colorfulBrush);

        const bool useColorAsMask = (bool)brushDefinition.attribute("ColorAsMask", "1").toInt();
        brush->setBrushApplication(colorfulBrush && colorfulBrush->hasColorAndTransparency() && !useColorAsMask ? IMAGESTAMP : ALPHAMASK);
    } else {
        brush->setBrushApplication(ALPHAMASK);
    }

    return brush;
}
