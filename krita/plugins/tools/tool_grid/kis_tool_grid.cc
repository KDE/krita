/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_tool_grid.h"

#include <KoViewConverter.h>
#include <KoCanvasController.h>
#include <KoPointerEvent.h>

#include <QPainter>

#include <kis_debug.h>
#include <klocale.h>

#include <canvas/kis_grid_manager.h>
#include <canvas/kis_canvas2.h>
#include <kis_config.h>
#include <kis_cursor.h>
#include <KisViewManager.h>


KisToolGrid::KisToolGrid(KoCanvasBase * canvas)
        : KisTool(canvas, KisCursor::moveCursor()), m_canvas(dynamic_cast<KisCanvas2*>(canvas))
{
    Q_ASSERT(m_canvas);
    setObjectName("tool_grid");
}

KisToolGrid::~KisToolGrid()
{
}

void KisToolGrid::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisTool::activate(toolActivation, shapes);
    m_canvas->updateCanvas();

    KisCanvasDecoration* decoration = m_canvas->decoration("grid");
    if (decoration && !decoration->visible()) {
        m_canvas->viewManager()->showFloatingMessage( "The grid is not visible. Press Return to show it.",
                                              koIcon("krita_tool_grid"));
    }
}

inline QPointF modPoints(const QPointF &x1, const QPointF &x2) {
    return QPointF(std::fmod(x1.x(), x2.x()), std::fmod(x1.y(), x2.y()));
}

inline QPointF divPoints(const QPointF &x1, const QPointF &x2) {
    return QPointF(x1.x() / x2.x(), x1.y() / x2.y());
}

inline QPointF mulPoints(const QPointF &x1, const QPointF &x2) {
    return QPointF(x1.x() * x2.x(), x1.y() * x2.y());
}

void KisToolGrid::beginPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::PAINT_MODE);

    m_dragStart = convertToPixelCoord(event);
    KisConfig cfg;
    m_initialSpacing = QPoint(cfg.getGridHSpacing(), cfg.getGridVSpacing());
    m_initialOffset = QPoint(cfg.getGridOffsetX(), cfg.getGridOffsetY());
}

void KisToolGrid::continuePrimaryAction(KoPointerEvent *event)
{
    KisConfig cfg;
    m_dragEnd = convertToPixelCoord(event);

    QPointF newOffset = m_initialOffset + m_dragEnd - m_dragStart;
    newOffset = modPoints(newOffset, qreal(cfg.getGridSubdivisions()) * m_initialSpacing);

    cfg.setGridOffsetX(newOffset.x());
    cfg.setGridOffsetY(newOffset.y());

    m_canvas->updateCanvas();
}

void KisToolGrid::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
    setMode(KisTool::HOVER_MODE);
}

void KisToolGrid::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action != Secondary &&
        action != PickFgNode &&
        action != PickFgImage) {

        KisTool::beginAlternateAction(event, action);
        return;
    }

    KisConfig cfg;
    m_initialSpacing = QPoint(cfg.getGridHSpacing(), cfg.getGridVSpacing());
    m_initialOffset = QPoint(cfg.getGridOffsetX(), cfg.getGridOffsetY());

    m_dragStart = convertToPixelCoord(event);
}

void KisToolGrid::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action != Secondary &&
        action != PickFgNode &&
        action != PickFgImage) {

        KisTool::continueAlternateAction(event, action);
        return;
    }

    KisConfig cfg;
    m_dragEnd = convertToPixelCoord(event);

    QPoint newSpacing = m_initialSpacing + (m_dragEnd - m_dragStart).toPoint();

    QPointF newOffset = m_dragStart - divPoints(mulPoints(modPoints(m_dragStart - m_initialOffset, m_initialSpacing), newSpacing), m_initialSpacing);
    newOffset = modPoints(newOffset, qreal(cfg.getGridSubdivisions()) * newSpacing);

    if (newSpacing.x() >= 1 && newSpacing.y() >= 1) {
        cfg.setGridHSpacing(newSpacing.x());
        cfg.setGridVSpacing(newSpacing.y());

        cfg.setGridOffsetX(newOffset.x());
        cfg.setGridOffsetY(newOffset.y());
    }

    m_canvas->updateCanvas();
}

void KisToolGrid::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action != Secondary &&
        action != PickFgNode &&
        action != PickFgImage) {

        KisTool::endAlternateAction(event, action);
        return;
    }

    setMode(KisTool::HOVER_MODE);
}

void KisToolGrid::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
}

void KisToolGrid::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return) {
        KisCanvasDecoration* decoration = m_canvas->decoration("grid");
        if (decoration) {
            decoration->setVisible(true);
        }
        m_canvas->viewManager()->gridManager()->checkVisibilityAction(true);
    }
    KoToolBase::keyPressEvent(event);
}

#include "kis_tool_grid.moc"
