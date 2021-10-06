/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_predefined_brush_factory.h"

#include <QApplication>
#include <QThread>
#include <QDomDocument>
#include <QFileInfo>
#include "kis_gbr_brush.h"
#include "kis_png_brush.h"
#include <kis_dom_utils.h>
#include <KisResourcesInterface.h>

KisPredefinedBrushFactory::KisPredefinedBrushFactory(const QString &brushType)
    : m_id(brushType)
{
}

QString KisPredefinedBrushFactory::id() const
{
    return m_id;
}

KisBrushSP KisPredefinedBrushFactory::createBrush(const QDomElement& brushDefinition, KisResourcesInterfaceSP resourcesInterface)
{
    auto resourceSourceAdapter = resourcesInterface->source<KisBrush>(ResourceType::Brushes);
    const QString brushFileName = brushDefinition.attribute("filename", "");
    const QString brushMD5Sum = brushDefinition.attribute("md5sum", "");
    KisBrushSP brush = resourceSourceAdapter.bestMatch(brushMD5Sum, brushFileName, "");
    if (!brush) {
        return nullptr;
    }

    // we always return a copy of the brush!
    brush = brush->clone().dynamicCast<KisBrush>();

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
        quint8 adjustmentMidPoint = brushDefinition.attribute("AdjustmentMidPoint", "127").toInt();
        qreal brightnessAdjustment = brushDefinition.attribute("BrightnessAdjustment").toDouble();
        qreal contrastAdjustment = brushDefinition.attribute("ContrastAdjustment").toDouble();

        const int adjustmentVersion = brushDefinition.attribute("AdjustmentVersion", "1").toInt();
        const bool autoAdjustMidPoint = brushDefinition.attribute("AutoAdjustMidPoint", "0").toInt();
        const bool hasAutoAdjustMidPoint = brushDefinition.hasAttribute("AutoAdjustMidPoint");

        /**
         * In Krita 4.x releases there was a bug that caused lightness
         * adjustments to be applied to the brush **twice**. It happened
         * due to the fact that copy-ctor called brushTipImage() virtual
         * method instead of just copying the image itself.
         *
         * In Krita 5 we should open these brushes in somewhat the same way.
         * The problem is that we cannot convert the numbers precisely, because
         * after applying a piecewice-linear function twice we get a
         * quadratic function. So we fall-back to a blunt parameters scaling,
         * which gives result that is just "good enough".
         *
         * NOTE: AutoAdjustMidPoint option appeared only in Krita 5, so it
         * automatically means the adjustments should be applied in the new way.
         */
        if (adjustmentVersion < 2 && !hasAutoAdjustMidPoint) {
            adjustmentMidPoint = qBound(0, 127 + (int(adjustmentMidPoint) - 127) * 2, 255);
            brightnessAdjustment *= 2.0;
            contrastAdjustment *= 2.0;

            /**
             * In Krita we also changed formula for contrast calculation in
             * negative part, so we need to convert that as well.
             */
            if (contrastAdjustment < 0) {
                contrastAdjustment = 1.0 / (1.0 - contrastAdjustment) - 1.0;
            }
        }

        colorfulBrush->setAdjustmentMidPoint(adjustmentMidPoint);
        colorfulBrush->setBrightnessAdjustment(brightnessAdjustment);
        colorfulBrush->setContrastAdjustment(contrastAdjustment);
        colorfulBrush->setAutoAdjustMidPoint(autoAdjustMidPoint);
    }

    auto legacyBrushApplication = [] (KisColorfulBrush *colorfulBrush, bool forceColorToAlpha) {
        /**
         * In Krita versions before 4.4 series "ColorAsMask" could
         * be overridden to false when the brush had no **color**
         * inside. That changed in Krita 4.4.x series, when
         * "brushApplication" replaced all the automatic heuristics
         */
        return colorfulBrush && colorfulBrush->hasColorAndTransparency() && !forceColorToAlpha ? IMAGESTAMP : ALPHAMASK;
    };


    if (brushDefinition.hasAttribute("preserveLightness")) {
        const int preserveLightness = KisDomUtils::toInt(brushDefinition.attribute("preserveLightness", "0"));
        const bool useColorAsMask = (bool)brushDefinition.attribute("ColorAsMask", "1").toInt();
        brush->setBrushApplication(preserveLightness ? LIGHTNESSMAP : legacyBrushApplication(colorfulBrush, useColorAsMask));
    }
    else if (brushDefinition.hasAttribute("brushApplication")) {
        enumBrushApplication brushApplication = static_cast<enumBrushApplication>(KisDomUtils::toInt(brushDefinition.attribute("brushApplication", "0")));
        brush->setBrushApplication(brushApplication);
    }
    else if (brushDefinition.hasAttribute("ColorAsMask")) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(colorfulBrush);

        const bool useColorAsMask = (bool)brushDefinition.attribute("ColorAsMask", "1").toInt();
        brush->setBrushApplication(legacyBrushApplication(colorfulBrush, useColorAsMask));
    }
    else {
        /**
         * In Krita versions before 4.4 series we used to automatrically select
         * the brush application depending on the presence of the color in the
         * brush, even when there was no "ColorAsMask" field.
         */
        brush->setBrushApplication(legacyBrushApplication(colorfulBrush, false));
    }

    return brush;
}
