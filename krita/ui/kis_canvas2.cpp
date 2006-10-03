/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoUnit.h"
#include "kis_canvas2.h"
#include "kis_view_converter.h"

KisCanvas2::KisCanvas2(KisViewConverter * viewConverter, QWidget * canvasWidget)
    : KoCanvasBase()
    , m_viewConverter( viewConverter )
    , m_canvasWidget( canvasWidget )
{
}

void KisCanvas2::setCanvasWidget(QWidget * widget)
{
    m_canvasWidget = widget;
}



KisCanvas2::~KisCanvas2()
{
}

void KisCanvas2::gridSize(double *horizontal, double *vertical) const
{
    Q_UNUSED( horizontal );
    Q_UNUSED( vertical );

}

bool KisCanvas2::snapToGrid() const
{
    return true;
}

void KisCanvas2::addCommand(KCommand *command, bool execute)
{
    Q_UNUSED( command );
    Q_UNUSED( execute );
}

KoShapeManager* KisCanvas2::shapeManager() const
{
    return 0;
}


void KisCanvas2::updateCanvas(const QRectF& rc)
{
    Q_UNUSED( rc );
}

KoTool* KisCanvas2::tool()
{
    return 0;
}

void KisCanvas2::setTool(KoTool *tool)
{
    Q_UNUSED( tool );
}


KoViewConverter* KisCanvas2::viewConverter()
{
    return m_viewConverter;
}

QWidget* KisCanvas2::canvasWidget()
{
    return m_canvasWidget;
}


KoUnit::Unit KisCanvas2::unit()
{
    return KoUnit::U_PIXEL;
}
