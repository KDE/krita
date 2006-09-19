/*
 *  kis_tool_line.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@comuzone.net>
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

#ifndef KIS_TOOL_LINE_H_
#define KIS_TOOL_LINE_H_

#include "kis_tool_paint.h"

#include "kis_global.h"
#include "kis_types.h"
#include "kis_tool_factory.h"

class KisBrush;
class KisPainter;

class QPoint;
class QWidget;


class KisToolLine : public KisToolPaint {

    Q_OBJECT
    typedef KisToolPaint super;

 public:
    KisToolLine();
    virtual ~KisToolLine();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }
    virtual quint32 priority() { return 1; }
    virtual void update(KisCanvasSubject *subject);

    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);

    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);

    virtual QString quickHelp() const;

 private:
    void paintLine();
    void paintLine(QPainter& gc, const QRect& rc);

    KisPoint straightLine(KisPoint point);


    bool m_dragging;

    KisPoint m_startPos;
    KisPoint m_endPos;

    KisCanvasSubject *m_subject;
    KisImageSP m_currentImage;
    KisPainter *m_painter;
};


class KisToolLineFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolLineFactory() : super() {};
    virtual ~KisToolLineFactory(){};
    
    virtual KisTool * createTool(KActionCollection * ac) { 
        KisTool * t =  new KisToolLine(); 
        Q_CHECK_PTR(t);
        t->setup(ac); 
        return t; 
    }
    virtual KoID id() { return KoID("line", i18n("Line Tool")); }
};




#endif //KIS_TOOL_LINE_H_

