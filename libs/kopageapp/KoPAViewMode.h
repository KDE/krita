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

#ifndef KOPAVIEWMODE_H
#define KOPAVIEWMODE_H

#include "kopageapp_export.h"

class KoPAView;
class KoPACanvas;
class KoToolProxy;
class QPaintEvent;
class QTabletEvent;
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class QEvent;
class QPointF;

class KOPAGEAPP_EXPORT KoPAViewMode
{
public:
	KoPAViewMode( KoPAView * view, KoPACanvas * canvas );
    virtual ~KoPAViewMode();

    virtual void paintEvent( QPaintEvent* event ) = 0;
    virtual void tabletEvent( QTabletEvent *event, const QPointF &point ) = 0;
    virtual void mousePressEvent( QMouseEvent *event, const QPointF &point ) = 0;
    virtual void mouseDoubleClickEvent( QMouseEvent *event, const QPointF &point ) = 0;
    virtual void mouseMoveEvent( QMouseEvent *event, const QPointF &point ) = 0;
    virtual void mouseReleaseEvent( QMouseEvent *event, const QPointF &point ) = 0;
    virtual void keyPressEvent( QKeyEvent *event ) = 0;
    virtual void keyReleaseEvent( QKeyEvent *event ) = 0;
    virtual void wheelEvent( QWheelEvent * event, const QPointF &point ) = 0;
    
protected:
    KoPAView * m_view;
    KoToolProxy * m_toolProxy;
};

#endif /* KOPAVIEWMODE_H */
