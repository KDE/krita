/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushModel.h"
#include "kis_brush_registry.h"

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
        result = autoBrush.generator.diameter * 2;
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

BrushModel::BrushModel(lager::cursor<BrushData> source)
    : m_source(source),
      LAGER_QT(brushType) {m_source[&BrushData::type]},
      LAGER_QT(autoBrush) {m_source[&BrushData::autoBrush]},
      LAGER_QT(predefinedBrush) {m_source[&BrushData::predefinedBrush]},
      LAGER_QT(textBrush) {m_source[&BrushData::textBrush]},
      LAGER_QT(userEffectiveSize) {
          lager::with(LAGER_QT(brushType),
                      LAGER_QT(autoBrush),
                      LAGER_QT(predefinedBrush),
                      LAGER_QT(textBrush))
                  .xform(zug::map(&detail::effectiveSizeForBrush))}
{
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

}
