/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef CONNECTIONTOOL_H
#define CONNECTIONTOOL_H

#include <KoTool.h>
#include <QPointF>

class KoShape;

class ConnectionTool : public KoTool {
    Q_OBJECT
public:
    explicit ConnectionTool(KoCanvasBase *canvas);
    ~ConnectionTool();

    /// reimplemented from superclass
    void paint( QPainter &painter, KoViewConverter &converter );

    /// reimplemented from superclass
    void mousePressEvent( KoPointerEvent *event ) ;
    /// reimplemented from superclass
    void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    void mouseReleaseEvent( KoPointerEvent *event );

    /// reimplemented from superclass
    void activate (bool temporary=false);
    /// reimplemented from superclass
    void deactivate();

private:
    void createConnection(KoShape *shape1, QPointF point1, KoShape *shape2, QPointF point2);

private:
    KoShape *m_startShape;
    QPointF m_startPoint;
    QList<KoShape*> m_shapesPaintedWithConnections;
};

#endif
