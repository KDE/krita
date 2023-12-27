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
#include "kis_abr_brush.h"
#include "kis_svg_brush.h"
#include <kis_dom_utils.h>
#include <KisResourcesInterface.h>
#include "kis_imagepipe_brush.h"

KisPredefinedBrushFactory::KisPredefinedBrushFactory(const QString &brushType)
    : m_id(brushType)
{
}

QString KisPredefinedBrushFactory::id() const
{
    return m_id;
}

KoResourceLoadResult KisPredefinedBrushFactory::createBrush(const KisBrushModel::BrushData &brushData, KisResourcesInterfaceSP resourcesInterface)
{
    auto resourceSourceAdapter = resourcesInterface->source<KisBrush>(ResourceType::Brushes);
    const QString brushFileName = brushData.predefinedBrush.resourceSignature.filename;
    const QString brushMD5Sum = brushData.predefinedBrush.resourceSignature.md5sum;
    KisBrushSP brush = resourceSourceAdapter.bestMatch(brushMD5Sum, brushFileName, "");
    if (!brush) {
        return KoResourceSignature(ResourceType::Brushes, brushMD5Sum, brushFileName, "");
    }

    // we always return a copy of the brush!
    brush = brush->clone().dynamicCast<KisBrush>();

    brush->setSpacing(brushData.common.spacing);
    brush->setAutoSpacing(brushData.common.useAutoSpacing, brushData.common.autoSpacingCoeff);
    brush->setAngle(brushData.common.angle);
    brush->setScale(brushData.predefinedBrush.scale);

    KisColorfulBrush *colorfulBrush = dynamic_cast<KisColorfulBrush*>(brush.data());
    if (colorfulBrush) {
        colorfulBrush->setAdjustmentMidPoint(brushData.predefinedBrush.adjustmentMidPoint);
        colorfulBrush->setBrightnessAdjustment(brushData.predefinedBrush.brightnessAdjustment);
        colorfulBrush->setContrastAdjustment(brushData.predefinedBrush.contrastAdjustment);
        colorfulBrush->setAutoAdjustMidPoint(brushData.predefinedBrush.autoAdjustMidPoint);
    }

    brush->setBrushApplication(brushData.predefinedBrush.application);

    return brush;
}

KoResourceLoadResult KisPredefinedBrushFactory::createBrush(const QDomElement& brushDefinition, KisResourcesInterfaceSP resourcesInterface)
{
    auto data = createBrushModelImpl(brushDefinition, resourcesInterface);

    if (std::holds_alternative<KisBrushModel::BrushData>(data)) {
        return createBrush(std::get<KisBrushModel::BrushData>(data), resourcesInterface);
    } else if (std::holds_alternative<KoResourceSignature>(data)) {
        return std::get<KoResourceSignature>(data);
    }

    // fallback, should never reach!
    return KoResourceSignature(ResourceType::Brushes, "", "", "");
}

std::optional<KisBrushModel::BrushData> KisPredefinedBrushFactory::createBrushModel(const QDomElement &element, KisResourcesInterfaceSP resourcesInterface)
{
    auto data = createBrushModelImpl(element, resourcesInterface);
    return std::holds_alternative<KisBrushModel::BrushData>(data) ?
        std::make_optional(std::get<KisBrushModel::BrushData>(data)) :
        std::nullopt;
}

std::variant<KisBrushModel::BrushData, KoResourceSignature> KisPredefinedBrushFactory::createBrushModelImpl(const QDomElement &element, KisResourcesInterfaceSP resourcesInterface)
{
    KisBrushModel::BrushData brush;

    auto resourceSourceAdapter = resourcesInterface->source<KisBrush>(ResourceType::Brushes);
    const QString brushFileName = element.attribute("filename", "");
    const QString brushMD5Sum = element.attribute("md5sum", "");
    KisBrushSP brushResource = resourceSourceAdapter.bestMatch(brushMD5Sum, brushFileName, "");
    if (!brushResource) {
        return KoResourceSignature(ResourceType::Brushes, brushMD5Sum, brushFileName, "");
    }

    brush.type = KisBrushModel::Predefined;

    /// we should first initialize data from the brush, because it may
    /// change the spacing properties embedded into some brushes
    loadFromBrushResource(brush.common, brush.predefinedBrush, brushResource);

    brush.common.angle = KisDomUtils::toDouble(element.attribute("angle", "0.0"));
    brush.common.spacing = KisDomUtils::toDouble(element.attribute("spacing", "0.25"));
    brush.common.useAutoSpacing = KisDomUtils::toInt(element.attribute("useAutoSpacing", "0"));
    brush.common.autoSpacingCoeff = KisDomUtils::toDouble(element.attribute("autoSpacingCoeff", "1.0"));

    brush.predefinedBrush.scale = KisDomUtils::toDouble(element.attribute("scale", "1.0"));

    // legacy support...
    if (element.attribute("BrushVersion", "1") == "1") {
        brush.predefinedBrush.scale *= 2.0;
    }

    KisColorfulBrush *colorfulBrush = dynamic_cast<KisColorfulBrush*>(brushResource.data());
    if (colorfulBrush) {
        quint8 adjustmentMidPoint = element.attribute("AdjustmentMidPoint", "127").toInt();
        qreal brightnessAdjustment = element.attribute("BrightnessAdjustment").toDouble();
        qreal contrastAdjustment = element.attribute("ContrastAdjustment").toDouble();

        const int adjustmentVersion = element.attribute("AdjustmentVersion", "1").toInt();
        const bool autoAdjustMidPoint = element.attribute("AutoAdjustMidPoint", "0").toInt();
        const bool hasAutoAdjustMidPoint = element.hasAttribute("AutoAdjustMidPoint");

        /**
         * In Krita 4.x releases there was a bug that caused lightness
         * adjustments to be applied to the brush **twice**. It happened
         * due to the fact that copy-ctor called brushTipImage() virtual
         * method instead of just copying the image itself.
         *
         * In Krita 5 we should open these brushes in somewhat the same way.
         * The problem is that we cannot convert the numbers precisely, because
         * after applying a piecewise-linear function twice we get a
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

        brush.predefinedBrush.adjustmentMidPoint = adjustmentMidPoint;
        brush.predefinedBrush.brightnessAdjustment = brightnessAdjustment;
        brush.predefinedBrush.contrastAdjustment = contrastAdjustment;
        brush.predefinedBrush.autoAdjustMidPoint = autoAdjustMidPoint;
        brush.predefinedBrush.hasColorAndTransparency = colorfulBrush->hasColorAndTransparency();
    }

    auto legacyBrushApplication = [] (KisColorfulBrush *colorfulBrush, bool forceColorToAlpha) {
        /**
         * In Krita versions before 4.4 series "ColorAsMask" could
         * be overridden to false when the brush had no **color**
         * inside. That changed in Krita 4.4.x series, when
         * "brushApplication" replaced all the automatic heuristics
         */
        return (colorfulBrush && colorfulBrush->hasColorAndTransparency() && !forceColorToAlpha) ? IMAGESTAMP : ALPHAMASK;
    };


    if (element.hasAttribute("preserveLightness")) {
        const int preserveLightness = KisDomUtils::toInt(element.attribute("preserveLightness", "0"));
        const bool useColorAsMask = (bool)element.attribute("ColorAsMask", "1").toInt();
        brush.predefinedBrush.application = preserveLightness ? LIGHTNESSMAP : legacyBrushApplication(colorfulBrush, useColorAsMask);
    }
    else if (element.hasAttribute("brushApplication")) {
        enumBrushApplication brushApplication = static_cast<enumBrushApplication>(KisDomUtils::toInt(element.attribute("brushApplication", "0")));
        brush.predefinedBrush.application = brushApplication;
    }
    else if (element.hasAttribute("ColorAsMask")) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(colorfulBrush);

        const bool useColorAsMask = (bool)element.attribute("ColorAsMask", "1").toInt();
        brush.predefinedBrush.application =legacyBrushApplication(colorfulBrush, useColorAsMask);
    }
    else {
        /**
         * In Krita versions before 4.4 series we used to automatically select
         * the brush application depending on the presence of the color in the
         * brush, even when there was no "ColorAsMask" field.
         */
        brush.predefinedBrush.application = legacyBrushApplication(colorfulBrush, false);
    }

    return {brush};
}

void KisPredefinedBrushFactory::loadFromBrushResource(KisBrushModel::CommonData &commonData, KisBrushModel::PredefinedBrushData &predefinedBrushData, KisBrushSP brushResource)
{
    commonData.spacing = brushResource->spacing();
    commonData.useAutoSpacing = brushResource->autoSpacingActive();
    commonData.autoSpacingCoeff = brushResource->autoSpacingCoeff();
    predefinedBrushData.resourceSignature = brushResource->signature();
    predefinedBrushData.brushType = brushResource->brushType();
    predefinedBrushData.baseSize = { brushResource->width(), brushResource->height() };

    if (brushResource.dynamicCast<KisGbrBrush>()) {
        predefinedBrushData.subtype = "gbr_brush";
    } else if (brushResource.dynamicCast<KisAbrBrush>()) {
        predefinedBrushData.subtype = "abr_brush";
    } else if (brushResource.dynamicCast<KisPngBrush>()) {
        predefinedBrushData.subtype = "png_brush";
    } else if (brushResource.dynamicCast<KisSvgBrush>()) {
        predefinedBrushData.subtype = "svg_brush";
    } else {
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "unknown brush type");
    }

    KisImagePipeBrushSP imagepipeBrush = brushResource.dynamicCast<KisImagePipeBrush>();
    if (imagepipeBrush) {
        predefinedBrushData.parasiteSelection = imagepipeBrush->parasiteSelection();
    }
}

void KisPredefinedBrushFactory::toXML(QDomDocument &doc, QDomElement &e, const KisBrushModel::BrushData &model)
{
    Q_UNUSED(doc);

    e.setAttribute("type", id());
    e.setAttribute("BrushVersion", "2");

    e.setAttribute("filename", model.predefinedBrush.resourceSignature.filename);
    e.setAttribute("md5sum", model.predefinedBrush.resourceSignature.md5sum);
    e.setAttribute("spacing", QString::number(model.common.spacing));
    e.setAttribute("useAutoSpacing", QString::number(model.common.useAutoSpacing));
    e.setAttribute("autoSpacingCoeff", QString::number(model.common.autoSpacingCoeff));
    e.setAttribute("angle", QString::number(model.common.angle));
    e.setAttribute("scale", QString::number(model.predefinedBrush.scale));
    e.setAttribute("brushApplication", QString::number((int)model.predefinedBrush.application));

    if (id() == "abr_brush") {
        e.setAttribute("name", model.predefinedBrush.resourceSignature.name);

    } else {
        // all other brushes are derived from KisColorfulBrush

        // legacy setting, now 'brushApplication' is used instead
        e.setAttribute("ColorAsMask", QString::number((int)(model.predefinedBrush.application != IMAGESTAMP)));

        e.setAttribute("AdjustmentMidPoint", QString::number(model.predefinedBrush.adjustmentMidPoint));
        e.setAttribute("BrightnessAdjustment", QString::number(model.predefinedBrush.brightnessAdjustment));
        e.setAttribute("ContrastAdjustment", QString::number(model.predefinedBrush.contrastAdjustment));
        e.setAttribute("AutoAdjustMidPoint", QString::number(model.predefinedBrush.autoAdjustMidPoint));
        e.setAttribute("AdjustmentVersion", QString::number(2));
    }
}
