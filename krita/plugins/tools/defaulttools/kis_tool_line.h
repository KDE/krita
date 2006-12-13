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
#include "KoToolFactory.h"
#include "kis_layer_shape.h"

class KisBrush;
class KisPainter;

class QPoint;
class QWidget;

class KoCanvasBase;

class KisToolLine : public KisToolPaint {

    Q_OBJECT
    typedef KisToolPaint super;

 public:
    KisToolLine(KoCanvasBase * canvas);
    virtual ~KisToolLine();

    virtual quint32 priority() { return 1; }

    virtual void buttonPress(KoPointerEvent *event);
    virtual void move(KoPointerEvent *event);
    virtual void buttonRelease(KoPointerEvent *event);

    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);

    virtual QString quickHelp() const;

 private:
    void paintLine();
    void paintLine(QPainter& gc, const QRect& rc);

    QPointF straightLine(QPointF point);

 private:

    bool m_dragging;

    QPointF m_startPos;
    QPointF m_endPos;

    KisPainter *m_painter;
};


class KisToolLineFactory : public KoToolFactory {

public:

    KisToolLineFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolLine", i18n( "Line" ))
        {
            setToolTip(i18n("Draw a line with the current brush"));
            // Temporarily
            setToolType(TOOL_TYPE_SHAPE);
            //setToolType( dynamicToolType() );
            setPriority(0);
            setIcon("tool_line");
            setActivationShapeID( KIS_LAYER_SHAPE_ID );
        }

    virtual ~KisToolLineFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolLine(canvas);
    }

};




#endif //KIS_TOOL_LINE_H_

