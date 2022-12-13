/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushModel.h"
#include "kis_brush_registry.h"
#include "brushengine/kis_paintop_lod_limitations.h"
#include "KisGlobalResourcesInterface.h"
#include <kis_predefined_brush_factory.h>

namespace KisBrushModel {

namespace detail {

qreal effectiveSizeForBrush(BrushType type,
                            const AutoBrushData &autoBrush,
                            const PredefinedBrushData &predefinedBrush,
                            const TextBrushData &textBrush)
{
    qreal result = 42;

    switch (type) {
    case Auto:
        result = autoBrush.generator.diameter;
        break;
    case Predefined:
        result = predefinedBrush.baseSize.width() * predefinedBrush.scale;
        break;
    case Text:
        result = textBrush.baseSize.width() * textBrush.scale;
        break;
    }

    return result;
}

QDomElement getBrushXMLElement(const KisPropertiesConfiguration *setting)
{
    QDomElement element;

    QString brushDefinition = setting->getString("brush_definition");

    if (!brushDefinition.isEmpty()) {
        QDomDocument d;
        d.setContent(brushDefinition, false);
        element = d.firstChildElement("Brush");
    }

    return element;
}

}

void BrushData::write(KisPropertiesConfiguration *settings) const
{
    QDomDocument d;
    QDomElement e = d.createElement("Brush");
    KisBrushRegistry::instance()->toXML(d, e, *this);
    d.appendChild(e);
    settings->setProperty("brush_definition", d.toString());
}

std::optional<BrushData> BrushData::read(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface)
{
    QDomElement element = detail::getBrushXMLElement(settings);
    return KisBrushRegistry::instance()->createBrushModel(element, resourcesInterface);
}

KisPaintopLodLimitations brushLodLimitations(const BrushData &data)
{
    KisPaintopLodLimitations l;

    if (!data.common.useAutoSpacing && data.common.spacing > 0.5) {
        l.limitations << KoID("huge-spacing", i18nc("PaintOp instant preview limitation", "Spacing > 0.5, consider disabling Instant Preview"));
    }

    if (data.type == Auto) {
        if (!qFuzzyCompare(data.autoBrush.density, 1.0)) {
            l.limitations << KoID("auto-brush-density", i18nc("PaintOp instant preview limitation", "Brush Density recommended value 100.0"));
        }

        if (!qFuzzyCompare(data.autoBrush.randomness, 0.0)) {
            l.limitations << KoID("auto-brush-randomness", i18nc("PaintOp instant preview limitation", "Brush Randomness recommended value 0.0"));
        }
    }

    return l;
}

qreal effectiveSizeForBrush(BrushType type,
                            const AutoBrushData &autoBrush,
                            const PredefinedBrushData &predefinedBrush,
                            const TextBrushData &textBrush)
{
    qreal result = 42;

    switch (type) {
    case Auto:
        result = autoBrush.generator.diameter;
        break;
    case Predefined:
        result = predefinedBrush.baseSize.width() * predefinedBrush.scale;
        break;
    case Text:
        result = textBrush.baseSize.width() * textBrush.scale;
        break;
    }

    return result;
}

qreal lightnessModeActivated(BrushType type, const PredefinedBrushData &predefinedBrush)
{
    // TODO: use effectiveApplication instead!!!
    return type == Predefined && predefinedBrush.application == LIGHTNESSMAP;
}

void setEffectiveSizeForBrush(const BrushType type,
                               AutoBrushData &autoBrush,
                               PredefinedBrushData &predefinedBrush,
                               TextBrushData &textBrush,
                               qreal value)
{
    switch (type) {
    case Auto:
        autoBrush.generator.diameter = value;
        break;
    case Predefined:
        predefinedBrush.scale = value / predefinedBrush.baseSize.width();
        break;
    case Text:
        textBrush.scale = value / textBrush.baseSize.width();
        break;
    }
}

qreal effectiveSizeForBrush(const BrushData &brush)
{
    return effectiveSizeForBrush(brush.type, brush.autoBrush, brush.predefinedBrush, brush.textBrush);
}

}
