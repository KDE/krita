/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_text_brush_factory.h"

#include <QString>
#include <QFont>
#include <kis_dom_utils.h>
#include "kis_text_brush.h"
#include <KoResourceLoadResult.h>


KoResourceLoadResult KisTextBrushFactory::createBrush(const QDomElement& brushDefinition, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

    QString text = brushDefinition.attribute("text", "The quick brown fox ate your text");
    QFont font;
    font.fromString(brushDefinition.attribute("font"));
    double spacing = KisDomUtils::toDouble(brushDefinition.attribute("spacing", "1.0"));
    QString pipeMode = brushDefinition.attribute("pipe", "false");
    bool pipe = (pipeMode == "true") ? true : false;

    KisTextBrushSP brush = KisTextBrushSP(new KisTextBrush());

    brush->setText(text);
    brush->setFont(font);
    brush->setPipeMode(pipe);
    brush->setSpacing(spacing);
    brush->updateBrush();

    return brush;
}

std::optional<KisBrushModel::BrushData> KisTextBrushFactory::createBrushModel(const QDomElement &element, KisResourcesInterfaceSP resourcesInterface)
{
    KisBrushModel::BrushData brush;

    brush.type = KisBrushModel::Text;
    brush.subtype = id();

    brush.common.spacing = KisDomUtils::toDouble(element.attribute("spacing", "1.0"));

    // TODO: these options are not yet present in the file format for the text brush!
    //brush.common.angle = KisDomUtils::toDouble(brushDefinition.attribute("angle", "0.0"));
    //brush.common.useAutoSpacing = KisDomUtils::toInt(brushDefinition.attribute("useAutoSpacing", "0"));
    //brush.common.autoSpacingCoeff = KisDomUtils::toDouble(brushDefinition.attribute("autoSpacingCoeff", "1.0"));

    brush.textBrush.text = element.attribute("text", "The quick brown fox ate your text");
    brush.textBrush.font = element.attribute("font");
    brush.textBrush.usePipeMode = element.attribute("pipe", "false") == "true";

    // TODO: how to get the size of the brush without its creation? lazy evaluation?
    // brush.textBrush.baseSize = {66, 77};

    brush.textBrush.scale = 1.0;

    return {brush};
}

void KisTextBrushFactory::toXML(QDomDocument &doc, QDomElement &e, const KisBrushModel::BrushData &model)
{
    Q_UNUSED(doc);

    e.setAttribute("type", id());
    e.setAttribute("BrushVersion", "2");

    e.setAttribute("spacing", KisDomUtils::toString(model.common.spacing));
    e.setAttribute("text", model.textBrush.text);
    e.setAttribute("font", model.textBrush.font);
    e.setAttribute("pipe", model.textBrush.usePipeMode ? "true" : "false");
    // TODO: scale is not saved
}
