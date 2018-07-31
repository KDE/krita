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
#include <KoSelectedShapesProxySimple.h>
#include <KoUnit.h>
#include <kis_shape_controller.h>

KisShapeSelectionCanvas::KisShapeSelectionCanvas(KoShapeControllerBase *shapeController)
    : KoCanvasBase(shapeController)
    , m_shapeManager(new KoShapeManager(this))
    , m_selectedShapesProxy(new KoSelectedShapesProxySimple(m_shapeManager.data()))
{
}

KisShapeSelectionCanvas::~KisShapeSelectionCanvas()
{
}

void KisShapeSelectionCanvas::gridSize(QPointF *offset, QSizeF *spacing) const
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    Q_UNUSED(offset);
    Q_UNUSED(spacing);
}

bool KisShapeSelectionCanvas::snapToGrid() const
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    return false;
}

void KisShapeSelectionCanvas::addCommand(KUndo2Command *)
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
}

KoShapeManager *KisShapeSelectionCanvas::shapeManager() const
{
    return m_shapeManager.data();
}

KoSelectedShapesProxy *KisShapeSelectionCanvas::selectedShapesProxy() const
{
    return m_selectedShapesProxy.data();
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

KoViewConverter *KisShapeSelectionCanvas::viewConverter() const
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

