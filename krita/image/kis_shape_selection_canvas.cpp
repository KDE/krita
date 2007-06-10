/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_shape_selection_canvas.h"

#include <KoShapeManager.h>
#include <KoViewConverter.h>

KisShapeSelectionCanvas::KisShapeSelectionCanvas(KisSelection *selection, KoViewConverter * viewConverter)
    : QObject(0)
    , KoCanvasBase( 0 )
    , m_viewConverter( viewConverter )
    , m_shapeManager( new KisSelectionShapeManager( this ) )
{
}

KisShapeSelectionCanvas::~KisShapeSelectionCanvas()
{
}

void KisShapeSelectionCanvas::gridSize(double *horizontal, double *vertical) const
{
    Q_ASSERT(false);
    Q_UNUSED( horizontal );
    Q_UNUSED( vertical );
}

bool KisShapeSelectionCanvas::snapToGrid() const
{
    Q_ASSERT(false);
    return false;
}

void KisShapeSelectionCanvas::addCommand(QUndoCommand *)
{
    Q_ASSERT(false);
}

KoShapeManager *KisShapeSelectionCanvas::shapeManager() const
{
    return m_shapeManager;
}

KoToolProxy * KisShapeSelectionCanvas::toolProxy() const
{
    return 0;
}

const KoViewConverter *KisShapeSelectionCanvas::viewConverter() const
{
    return m_viewConverter;
}

QWidget* KisShapeSelectionCanvas::canvasWidget()
{
    return 0;
}

KoUnit KisShapeSelectionCanvas::unit() const
{
    Q_ASSERT(false);
    return KoUnit(KoUnit::Point);
}

#include "kis_shape_selection_canvas.moc"
