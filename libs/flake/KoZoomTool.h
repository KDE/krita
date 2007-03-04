/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
  *  Copyright (c) 2007 Casper Boemann <cbr@boemann.dk>
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

#ifndef ZOOM_TOOL_H_
#define ZOOM_TOOL_H_

#include <QTimer>

#include <KoToolFactory.h>

#define KoZoomTool_ID "flake/zoomTool"

class KoCanvasController;

class KoZoomTool : public KoTool {

    typedef KoTool super;
    Q_OBJECT

public:
    KoZoomTool(KoCanvasBase *canvas);
    virtual ~KoZoomTool();

public:
    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseDoubleClickEvent( KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint(QPainter &painter, KoViewConverter &converter);

    void setCanvasController(KoCanvasController *controller) { m_controller = controller; }

public slots:
    void activate();
    void deactivate();

private slots:
    void slotTimer();

private:
    QPointF m_startPos;
    QPointF m_endPos;
    bool m_dragging;
    QCursor m_plusCursor;
    QCursor m_minusCursor;
    QTimer m_timer;
    KoCanvasController *m_controller;
};


class KoZoomToolFactory : public KoToolFactory {

public:
    KoZoomToolFactory(QObject *parent)
        : KoToolFactory(parent,  KoZoomTool_ID, i18n( "Zoom Tool") )
        {
            setToolTip( i18n( "Zoom" ) );
            //setToolType( TOOL_TYPE_VIEW );
            setToolType( mainToolType() );

            setIcon( "viewmag" );
            setPriority( 5 );
//            setShortcut( KShortcut( Qt:Key_Z ) );
        }

    virtual ~KoZoomToolFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KoZoomTool(canvas);
    }

};


#endif // ZOOM_TOOL_H_
