/*
 *  kis_tool_polyline.h - part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael Thaler@physik.tu-muenchen.de>
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

#ifndef KIS_TOOL_POLYLINE_H_
#define KIS_TOOL_POLYLINE_H_

#include <q3valuevector.h>
#include <QString>
//Added by qt3to4:
#include <QKeyEvent>

#include "kis_tool_paint.h"
#include "kis_layer_shape.h"


class KisCanvas;
class KisDoc;
class KisPainter;
class KisView;
class KisRect;


class KisToolPolyline : public KisToolPaint {

    typedef KisToolPaint super;
    Q_OBJECT

public:
    KisToolPolyline(KoCanvasBase * canvas);
    virtual ~KisToolPolyline();

//     virtual enumToolType toolType() { return TOOL_SHAPE; }
    virtual quint32 priority() { return 5; }

    virtual void mouseDoubleClickEvent(KoPointerEvent *event);
    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

    virtual QString quickHelp() const;

    virtual void paint(QPainter& gc, KoViewConverter &converter);

public slots:
    void finish();
    void cancel();
    void deactivate();

protected:
    QPointF m_dragStart;
    QPointF m_dragEnd;

    bool m_dragging;
private:
    typedef Q3ValueVector<QPointF> KoPointVector;
    KoPointVector m_points;
    QRectF m_boundingRect;
};


#include "KoToolFactory.h"

class KisToolPolylineFactory : public KoToolFactory {

public:
    KisToolPolylineFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolPolyline", i18n( "Polyline" ))
        {
            setToolTip( i18n( "Draw a polyline. Shift-mouseclick ends the polyline." ) );
//             setToolType( TOOL_TYPE_SHAPE );
            setToolType( dynamicToolType() );
            setActivationShapeID( KIS_LAYER_SHAPE_ID );
            setIcon( "polyline" );
            setPriority( 0 );
        }

    virtual ~KisToolPolylineFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolPolyline(canvas);
    }

};


#endif //__KIS_TOOL_POLYLINE_H__
