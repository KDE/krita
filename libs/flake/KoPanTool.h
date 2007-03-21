/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoTool.h"

#include <QPointF>

class KoCanvasController;

class KoPanTool : public KoTool {
public:
    explicit KoPanTool(KoCanvasBase *canvas);

    /// reimplemented from superclass
    virtual bool wantsAutoScroll();
    /// reimplemented from superclass
    virtual void mousePressEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented from superclass
    virtual void paint(QPainter &, KoViewConverter &) {}
    /// reimplemented from superclass
    virtual void activate(bool temporary = false);

    void setCanvasController(KoCanvasController *controller) { m_controller = controller; }

private:
    KoCanvasController *m_controller;
    QPointF m_lastPosition;
    bool m_temporary;
};

#endif
