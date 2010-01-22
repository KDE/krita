/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_complexop_settings.h"
#include "kis_complexop_settings_widget.h"

#include <kis_brush_option_widget.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>

#include <kis_image.h>
#include <KoViewConverter.h>
#include <kis_boundary.h>
#include <kis_boundary_painter.h>
#include <kis_paint_device.h> // TODO remove me when KisBoundary is used as pointers

#include "kis_complexop_settings_widget.h"

KisComplexOpSettings::KisComplexOpSettings()
        : m_options(0)
{
}

KisComplexOpSettings::~KisComplexOpSettings()
{
}

bool KisComplexOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

void KisComplexOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const
{
    KisComplexOpSettingsWidget* options = dynamic_cast<KisComplexOpSettingsWidget*>(optionsWidget());
    if(!options)
        return;

    if (_mode != CURSOR_IS_OUTLINE) return;
    KisBrushSP brush = options->m_brushOption->brush();
    QPointF hotSpot = brush->hotSpot(1.0, 1.0);
    painter.setPen(Qt::black);
    painter.setBackground(Qt::black);
    painter.translate(converter.documentToView(pos - image->pixelToDocument(hotSpot + QPointF(0.5, 0.5))));
    KisBoundaryPainter::paint(brush->boundary(), image, painter, converter);
}
