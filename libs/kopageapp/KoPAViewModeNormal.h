/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOPAVIEWMODENORMAL_H
#define KOPAVIEWMODENORMAL_H

#include "KoPAViewMode.h"

class KoPAPage;
class KoPACanvasBase;
class QPainter;
class QRectF;
class KoViewConverter;

class KOPAGEAPP_EXPORT KoPAViewModeNormal : public KoPAViewMode
{
public:
    KoPAViewModeNormal( KoPAViewBase * view, KoPACanvasBase * m_canvas );
    ~KoPAViewModeNormal();

    void paintEvent( KoPACanvas * canvas, QPaintEvent* event );
    void tabletEvent( QTabletEvent *event, const QPointF &point );
    void mousePressEvent( QMouseEvent *event, const QPointF &point );
    void mouseDoubleClickEvent( QMouseEvent *event, const QPointF &point );
    void mouseMoveEvent( QMouseEvent *event, const QPointF &point );
    void mouseReleaseEvent( QMouseEvent *event, const QPointF &point );
    void keyPressEvent( QKeyEvent *event );
    void keyReleaseEvent( QKeyEvent *event );
    void wheelEvent( QWheelEvent * event, const QPointF &point );

    /**
     * @brief Switch the active view mode to work on master/normal pages
     *
     * When it is switched to master mode the master page of the current active page
     * is selected. If it switches back the page which was shown before going into
     * the master mode is shown. If the mode is the same nothing happens.
     *
     * @param master if true work on master pages, if false work on normal pages
     */
    virtual void setMasterMode( bool master );

    virtual bool masterMode();

    void addShape( KoShape *shape );

    void removeShape( KoShape *shape );

    virtual void changePageLayout( const KoPageLayout &pageLayout, bool applyToDocument, QUndoCommand *parent = 0 );

private:
    void paintMargins( QPainter &painter, const KoViewConverter &converter );

    /// if true it works on master pages, if false on normal pages
    bool m_masterMode;
    /// the page which was active before entering the master mode
    KoPAPage * m_savedPage;
};

#endif /* KOPAVIEWMODENORMAL_H */
