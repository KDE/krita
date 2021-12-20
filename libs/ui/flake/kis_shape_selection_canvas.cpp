/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    m_shapeManager->explicitlyIssueShapeChangedSignals();
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

