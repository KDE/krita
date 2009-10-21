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

#include "kis_text_brush.h"

KisBrushSP KisTextBrushFactory::getOrCreateBrush(const QDomElement& brushDefinition)
{
    QString text = brushDefinition.attribute("text_brush_text", "The quick brown fox ate your text");
    QFont font;
    font.fromString(brushDefinition.attribute("text_brush_font"));
    double spacing = brushDefinition.attribute("brush_spacing", "1.0").toDouble();

    KisBrushSP brush = new KisTextBrush(text, font);
    brush->setSpacing(spacing);

    return brush;

}
