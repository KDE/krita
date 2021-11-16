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
