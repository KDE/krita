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


#include <QPainter>

#include <KoShapeManager.h>

KisShapeSelectionCanvas::KisShapeSelectionCanvas()
        : KoCanvasBase(0)
        , m_shapeManager(new KoShapeManager(this))
{
}

KisShapeSelectionCanvas::~KisShapeSelectionCanvas()
{
    delete m_shapeManager;
}

void KisShapeSelectionCanvas::gridSize(qreal *horizontal, qreal *vertical) const
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    Q_UNUSED(horizontal);
    Q_UNUSED(vertical);
}

bool KisShapeSelectionCanvas::snapToGrid() const
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    return false;
}

void KisShapeSelectionCanvas::addCommand(QUndoCommand *)
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
}

KoShapeManager *KisShapeSelectionCanvas::shapeManager() const
{
    return m_shapeManager;
}

void KisShapeSelectionCanvas::updateCanvas(const QRectF& rc)
{
    Q_UNUSED(rc);
}

KoToolProxy * KisShapeSelectionCanvas::toolProxy() const
{
//     Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    return 0;
}

const KoViewConverter *KisShapeSelectionCanvas::viewConverter() const
{
    return 0;
}

QWidget* KisShapeSelectionCanvas::canvasWidget()
{
    return 0;
}

const QWidget* KisShapeSelectionCanvas::canvasWidget() const
{
    return 0;
}

KoUnit KisShapeSelectionCanvas::unit() const
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    return KoUnit(KoUnit::Point);
}

#include "kis_shape_selection_canvas.moc"
