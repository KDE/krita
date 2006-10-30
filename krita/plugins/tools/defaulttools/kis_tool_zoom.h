/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_ZOOM_TOOL_H_
#define KIS_ZOOM_TOOL_H_

#include <QTimer>

#include "kis_tool_non_paint.h"

#include "KoToolFactory.h"

class KisCanvasSubject;

class KisToolZoom : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolZoom();
    virtual ~KisToolZoom();

public:
    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_VIEW; }
    virtual quint32 priority() { return 3; }

    virtual void buttonPress(KoPointerEvent *e);
    virtual void move(KoPointerEvent *e);
    virtual void buttonRelease(KoPointerEvent *e);

    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);

private:
    void paintOutline();
    void paintOutline(QPainter& gc, const QRect& rc);


public slots:

    void activate();
    void deactivate();


private slots:
    void slotTimer();

private:
    KisCanvasSubject *m_subject;
    QPoint m_startPos;
    QPoint m_endPos;
    bool m_dragging;
    QCursor m_plusCursor;
    QCursor m_minusCursor;
    QTimer m_timer;
};


class KisToolZoomFactory : public KoToolFactory {

public:
    KisToolZoomFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent,  "KisToolZoom", i18n( "Zoom tool") )
        {
            setToolTip( i18n( "Zoom" ) );
            setToolType( TOOL_TYPE_VIEW );
            setIcon( "viewmag" );
            setPriority( 0 );
            setShortcut( Qt:Key_Z );
        };

    virtual ~KisToolZoomFactory(){};

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolZoom(canvas);
    }

};


#endif // KIS_ZOOM_TOOL_H_
