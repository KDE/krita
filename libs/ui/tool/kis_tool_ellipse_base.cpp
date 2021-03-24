/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_tool_ellipse_base.h"

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>
#include "KisViewManager.h"

#include "kis_canvas2.h"

KisToolEllipseBase::KisToolEllipseBase(KoCanvasBase * canvas, KisToolEllipseBase::ToolType type, const QCursor & cursor)
    : KisToolRectangleBase(canvas, type, cursor)
{
}

void KisToolEllipseBase::paintRectangle(QPainter &gc, const QRectF &imageRect)
{
    KIS_ASSERT_RECOVER_RETURN(canvas());

    QRect viewRect = pixelToView(imageRect).toRect();

    QPainterPath path;
    path.addEllipse(viewRect);
    paintToolOutline(&gc, path);
    KisCanvas2 *kisCanvas =dynamic_cast<KisCanvas2*>(canvas());
    kisCanvas->viewManager()->showFloatingMessage(i18n("Width: %1 px\nHeight: %2 px", createRect(m_dragStart, m_dragEnd).width()
                                                       , createRect(m_dragStart, m_dragEnd).height()), QIcon(), 1000
                                                       , KisFloatingMessage::High,  Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter);
}

bool KisToolEllipseBase::showRoundCornersGUI() const
{
    return false;
}
