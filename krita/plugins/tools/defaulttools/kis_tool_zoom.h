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

#include <qtimer.h>

#include "kis_tool_non_paint.h"

#include "kis_tool_factory.h"

class KisCanvasSubject;

class KisToolZoom : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolZoom();
    virtual ~KisToolZoom();

public:
    virtual void update(KisCanvasSubject *subject);

public:
    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_VIEW; }
    virtual Q_UINT32 priority() { return 2; }

    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);

    virtual void paint(KisCanvasPainter& gc);
    virtual void paint(KisCanvasPainter& gc, const QRect& rc);

private:
    void paintOutline();
    void paintOutline(KisCanvasPainter& gc, const QRect& rc);


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


class KisToolZoomFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolZoomFactory() : super() {};
    virtual ~KisToolZoomFactory(){};
    
    virtual KisTool * createTool(KActionCollection * ac) { 
        KisTool * t = new KisToolZoom(); 
        Q_CHECK_PTR(t);
        t->setup(ac); 
        return t; 
    }
    virtual KisID id() { return KisID("zoom", i18n("Zoom Tool")); }
};


#endif // KIS_ZOOM_TOOL_H_
