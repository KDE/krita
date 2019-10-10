/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_tool_path.h"
#include <KoPathShape.h>
#include <KoCanvasBase.h>
#include <kis_cursor.h>

KisToolPath::KisToolPath(KoCanvasBase * canvas)
    : DelegatedPathTool(canvas, Qt::ArrowCursor,
                        new __KisToolPathLocalTool(canvas, this))
{
}

void KisToolPath::resetCursorStyle()
{
    DelegatedPathTool::resetCursorStyle();
    overrideCursorIfNotEditable();
}

void KisToolPath::requestStrokeEnd()
{
    localTool()->endPathWithoutLastPoint();
}

void KisToolPath::requestStrokeCancellation()
{
    localTool()->cancelPath();
}

void KisToolPath::mousePressEvent(KoPointerEvent *event)
{
    if (!nodeEditable()) return;
    DelegatedPathTool::mousePressEvent(event);
}

// Install an event filter to catch right-click events.
// The simplest way to accommodate the popup palette binding.
// This code is duplicated in kis_tool_select_path.cc
bool KisToolPath::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            localTool()->removeLastPoint();
            return true;
        }
    } else if (event->type() == QEvent::TabletPress) {
        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        if (tabletEvent->button() == Qt::RightButton) {
            localTool()->removeLastPoint();
            return true;
        }
    }
    return false;
}

void KisToolPath::beginAlternateAction(KoPointerEvent *event, AlternateAction action) {
    Q_UNUSED(action)
    mousePressEvent(event);
}
void KisToolPath::continueAlternateAction(KoPointerEvent *event, AlternateAction action){
    Q_UNUSED(action)
    mouseMoveEvent(event);
}

void KisToolPath::endAlternateAction(KoPointerEvent *event, AlternateAction action) {
    Q_UNUSED(action)
    mouseReleaseEvent(event);
}

QList<QPointer<QWidget> > KisToolPath::createOptionWidgets()
{
    QList<QPointer<QWidget> > widgets = DelegatedPathTool::createOptionWidgets();
    return widgets;
}


__KisToolPathLocalTool::__KisToolPathLocalTool(KoCanvasBase * canvas, KisToolPath* parentTool)
    : KoCreatePathTool(canvas)
    , m_parentTool(parentTool) {}

void __KisToolPathLocalTool::paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    QTransform matrix;
    matrix.scale(m_parentTool->image()->xRes(), m_parentTool->image()->yRes());
    matrix.translate(pathShape.position().x(), pathShape.position().y());
    m_parentTool->paintToolOutline(&painter, m_parentTool->pixelToView(matrix.map(pathShape.outline())));
}

void __KisToolPathLocalTool::addPathShape(KoPathShape* pathShape)
{
    if (!KoCreatePathTool::tryMergeInPathShape(pathShape)) {
        m_parentTool->addPathShape(pathShape, kundo2_i18n("Draw Bezier Curve"));
    }
}
