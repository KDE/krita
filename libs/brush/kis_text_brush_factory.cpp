/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_text_brush_factory.h"

#include <QString>
#include <QFont>
#include <kis_dom_utils.h>
#include "kis_text_brush.h"

KisBrushSP KisTextBrushFactory::createBrush(const QDomElement& brushDefinition)
{
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
