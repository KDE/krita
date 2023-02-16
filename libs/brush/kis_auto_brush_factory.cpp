/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2010-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_auto_brush_factory.h"

#include <QDomDocument>

#include "kis_auto_brush.h"
#include "kis_mask_generator.h"
#include <kis_dom_utils.h>
#include <KoResourceLoadResult.h>
#include "kis_mask_generator.h"


KoResourceLoadResult KisAutoBrushFactory::createBrush(const QDomElement &brushDefinition, KisResourcesInterfaceSP resourcesInterface)
{
    std::optional<KisBrushModel::BrushData> data =
        createBrushModel(brushDefinition, resourcesInterface);

    if (data) {
        return createBrush(*data, resourcesInterface);
    }

    // fallback, should never reach!
    return KoResourceSignature(ResourceType::Brushes, "", "", "");
}

KoResourceLoadResult KisAutoBrushFactory::createBrush(const KisBrushModel::BrushData &data, KisResourcesInterfaceSP resourcesInterface)
{
    return createBrush(data.common, data.autoBrush, resourcesInterface);
}

std::optional<KisBrushModel::BrushData>
KisAutoBrushFactory::createBrushModel(const QDomElement &element, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

    KisBrushModel::BrushData brush;

    brush.type = KisBrushModel::Auto;
    brush.common.angle = KisDomUtils::toDouble(element.attribute("angle", "0.0"));
    brush.common.spacing = KisDomUtils::toDouble(element.attribute("spacing", "1.0"));
    brush.common.useAutoSpacing = KisDomUtils::toInt(element.attribute("useAutoSpacing", "0"));
    brush.common.autoSpacingCoeff = KisDomUtils::toDouble(element.attribute("autoSpacingCoeff", "1.0"));

    // auto brush settings

    brush.autoBrush.randomness = KisDomUtils::toDouble(element.attribute("randomness", "0.0"));
    brush.autoBrush.density = KisDomUtils::toDouble(element.attribute("density", "1.0"));

    // generator settings

    QDomElement generatorEl = element.firstChildElement("MaskGenerator");

    // backward compatibility -- it was mistakenly named radius for 2.2
    brush.autoBrush.generator.diameter =
            generatorEl.hasAttribute("radius") ?
                KisDomUtils::toDouble(generatorEl.attribute("radius", "1.0")) :
                KisDomUtils::toDouble(generatorEl.attribute("diameter", "1.0"));

    brush.autoBrush.generator.ratio = KisDomUtils::toDouble(generatorEl.attribute("ratio", "1.0"));
    brush.autoBrush.generator.horizontalFade = KisDomUtils::toDouble(generatorEl.attribute("hfade", "0.0"));
    brush.autoBrush.generator.verticalFade = KisDomUtils::toDouble(generatorEl.attribute("vfade", "0.0"));
    brush.autoBrush.generator.antialiasEdges = generatorEl.attribute("antialiasEdges", "0").toInt();
    brush.autoBrush.generator.spikes = generatorEl.attribute("spikes", "2").toInt();

    const QString shape = generatorEl.attribute("type", "circle");
    brush.autoBrush.generator.shape =
            shape == "circle" ? KisBrushModel::Circle : KisBrushModel::Rectangle;

    const QString type = generatorEl.attribute("id", DefaultId.id());
    brush.autoBrush.generator.type =
            type == DefaultId.id() ? KisBrushModel::Default :
            type == SoftId.id() ? KisBrushModel::Soft :
            KisBrushModel::Gaussian;

    if (generatorEl.hasAttribute("softness_curve")) {
        brush.autoBrush.generator.curveString = generatorEl.attribute("softness_curve","");
    }

    return {brush};
}

void KisAutoBrushFactory::toXML(QDomDocument &doc, QDomElement &e, const KisBrushModel::BrushData &model)
{
    e.setAttribute("type", id());
    e.setAttribute("BrushVersion", "2");

    e.setAttribute("spacing", QString::number(model.common.spacing));
    e.setAttribute("useAutoSpacing", QString::number(model.common.useAutoSpacing));
    e.setAttribute("autoSpacingCoeff", QString::number(model.common.autoSpacingCoeff));
    e.setAttribute("angle", QString::number(model.common.angle));
    e.setAttribute("randomness", QString::number(model.autoBrush.randomness));
    e.setAttribute("density", QString::number(model.autoBrush.density));

    {
        QDomElement shapeElt = doc.createElement("MaskGenerator");

        shapeElt.setAttribute("diameter", QString::number(model.autoBrush.generator.diameter));
        shapeElt.setAttribute("ratio", QString::number(model.autoBrush.generator.ratio));
        shapeElt.setAttribute("hfade", QString::number(model.autoBrush.generator.horizontalFade));
        shapeElt.setAttribute("vfade", QString::number(model.autoBrush.generator.verticalFade));
        shapeElt.setAttribute("spikes", model.autoBrush.generator.spikes);
        shapeElt.setAttribute("type", model.autoBrush.generator.shape == KisBrushModel::Circle ? "circle" : "rct");
        shapeElt.setAttribute("antialiasEdges", model.autoBrush.generator.antialiasEdges);


        QString idString;
        if (model.autoBrush.generator.type == KisBrushModel::Default) {
            idString = DefaultId.id();
        } else if (model.autoBrush.generator.type == KisBrushModel::Soft) {
            idString = SoftId.id();
        } else {
            idString = GaussId.id();
        }

        shapeElt.setAttribute("id", idString);

        if (!model.autoBrush.generator.curveString.isEmpty()) {
            shapeElt.setAttribute("softness_curve", model.autoBrush.generator.curveString);
        }

        e.appendChild(shapeElt);
    }
}

KoResourceLoadResult KisAutoBrushFactory::createBrush(const KisBrushModel::CommonData &commonData, const KisBrushModel::AutoBrushData &autoBrushData, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

    KisMaskGenerator *generator = 0;

    if (autoBrushData.generator.type == KisBrushModel::Default &&
            autoBrushData.generator.shape == KisBrushModel::Circle) {

        generator = new KisCircleMaskGenerator(autoBrushData.generator.diameter,
                                               autoBrushData.generator.ratio,
                                               autoBrushData.generator.horizontalFade,
                                               autoBrushData.generator.verticalFade,
                                               autoBrushData.generator.spikes,
                                               autoBrushData.generator.antialiasEdges);

    } else if (autoBrushData.generator.type == KisBrushModel::Default &&
               autoBrushData.generator.shape == KisBrushModel::Rectangle) {

        generator = new KisRectangleMaskGenerator(autoBrushData.generator.diameter,
                                                  autoBrushData.generator.ratio,
                                                  autoBrushData.generator.horizontalFade,
                                                  autoBrushData.generator.verticalFade,
                                                  autoBrushData.generator.spikes,
                                                  autoBrushData.generator.antialiasEdges);


    } else  if (autoBrushData.generator.type == KisBrushModel::Soft) {

        QString curveString = autoBrushData.generator.curveString;
        if (curveString.isEmpty()) {
            curveString = "0,1;1,0";
        }

        const KisCubicCurve curve(curveString);

        if (autoBrushData.generator.shape == KisBrushModel::Circle) {
            generator = new KisCurveCircleMaskGenerator(autoBrushData.generator.diameter,
                                                        autoBrushData.generator.ratio,
                                                        autoBrushData.generator.horizontalFade,
                                                        autoBrushData.generator.verticalFade,
                                                        autoBrushData.generator.spikes,
                                                        curve,
                                                        autoBrushData.generator.antialiasEdges);
        } else {
            generator = new KisCurveRectangleMaskGenerator(autoBrushData.generator.diameter,
                                                           autoBrushData.generator.ratio,
                                                           autoBrushData.generator.horizontalFade,
                                                           autoBrushData.generator.verticalFade,
                                                           autoBrushData.generator.spikes,
                                                           curve,
                                                           autoBrushData.generator.antialiasEdges);
        }

    } else  if (autoBrushData.generator.type == KisBrushModel::Gaussian &&
                autoBrushData.generator.shape == KisBrushModel::Circle) {

        generator = new KisGaussCircleMaskGenerator(autoBrushData.generator.diameter,
                                                    autoBrushData.generator.ratio,
                                                    autoBrushData.generator.horizontalFade,
                                                    autoBrushData.generator.verticalFade,
                                                    autoBrushData.generator.spikes,
                                                    autoBrushData.generator.antialiasEdges);

    } else if (autoBrushData.generator.type == KisBrushModel::Gaussian &&
               autoBrushData.generator.shape == KisBrushModel::Rectangle) {

        generator = new KisGaussRectangleMaskGenerator(autoBrushData.generator.diameter,
                                                       autoBrushData.generator.ratio,
                                                       autoBrushData.generator.horizontalFade,
                                                       autoBrushData.generator.verticalFade,
                                                       autoBrushData.generator.spikes,
                                                       autoBrushData.generator.antialiasEdges);
    }

    KisBrushSP brush = KisBrushSP(new KisAutoBrush(generator, commonData.angle, autoBrushData.randomness, autoBrushData.density));
    brush->setSpacing(commonData.spacing);
    brush->setAutoSpacing(commonData.useAutoSpacing, commonData.autoSpacingCoeff);

    return {brush};
}
