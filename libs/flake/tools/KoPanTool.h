/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
#ifndef KOPANTOOL_H
#define KOPANTOOL_H

#include "KoToolBase.h"

#include <QPointF>

class KoCanvasController;

#define KoPanTool_ID "PanTool"

/**
 * This is the tool that allows you to move the canvas by dragging it and 'panning' around.
 */
class KoPanTool : public KoToolBase
{
public:
    /**
     * Constructor.
     * @param canvas the canvas this tool works on.
     */
    explicit KoPanTool(KoCanvasBase *canvas);

    bool wantsAutoScroll() const override;
    void mousePressEvent(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;
    void mouseReleaseEvent(KoPointerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paint(QPainter &, const KoViewConverter &) override {}
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    void mouseDoubleClickEvent(KoPointerEvent *event) override;

    /// set the canvasController this tool works on.
    void setCanvasController(KoCanvasController *controller) {
        m_controller = controller;
    }

private:
    QPointF documentToViewport(const QPointF &p);
    KoCanvasController *m_controller;
    QPointF m_lastPosition;
    bool m_temporary;
    Q_DECLARE_PRIVATE(KoToolBase)
};

#endif
