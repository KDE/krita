/* This file is part of the KDE project
 * Copyright (  C ) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#ifndef KOPAVIEWMODENORMAL_H
#define KOPAVIEWMODENORMAL_H

#include "KoPAViewMode.h"

class KOPAGEAPP_EXPORT KoPAViewModeNormal : public KoPAViewMode
{
public:
	KoPAViewModeNormal( KoPAView * view, KoPACanvas * m_canvas );
    ~KoPAViewModeNormal();

    void paintEvent( QPaintEvent* event );
    void tabletEvent( QTabletEvent *event, const QPointF &point );
    void mousePressEvent( QMouseEvent *event, const QPointF &point );
    void mouseDoubleClickEvent( QMouseEvent *event, const QPointF &point );
    void mouseMoveEvent( QMouseEvent *event, const QPointF &point );
    void mouseReleaseEvent( QMouseEvent *event, const QPointF &point );
    void keyPressEvent( QKeyEvent *event );
    void keyReleaseEvent( QKeyEvent *event );
    void wheelEvent( QWheelEvent * event, const QPointF &point );
};

#endif /* KOPAVIEWMODENORMAL_H */
