/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_TOOL_PAN_H_
#define KIS_TOOL_PAN_H_


#include "kis_tool_non_paint.h"
#include "KoToolFactory.h"
#include <koffice_export.h>
#include <kis_layer_shape.h>
class KisCanvasSubject;
class QPointF;

class KRITATOOL_EXPORT KisToolPan : public KisToolNonPaint
{

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolPan();
    virtual ~KisToolPan();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_VIEW; }

    virtual void buttonPress(KoPointerEvent *e);
    virtual void move(KoPointerEvent *e);
    virtual void buttonRelease(KoPointerEvent *e);

    virtual bool wantsAutoScroll() const { return false; }

private:

    QPointF m_dragPos;
    qint32 m_origScrollX;
    qint32 m_origScrollY;
    bool m_dragging;
    QCursor m_openHandCursor;
    QCursor m_closedHandCursor;
};

class KisToolPanFactory : public KoToolFactory {

public:
    KisToolPanFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KritaView/KisToolPan", i18n( "Pan" ))
        {
            setToolTip( i18n( "Pan the current view" ) );
            //setToolType( TOOL_TYPE_VIEW );
            setToolType( dynamicToolType() );
            setActivationShapeID( KIS_LAYER_SHAPE_ID );
            setIcon( "tool_pan" );
            setPriority( 0 );
            setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_H ) );
        }

    virtual ~KisToolPanFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolPan(canvas);
    }

};


#endif // KIS_TOOL_PAN_H_

