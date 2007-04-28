/*
 *  kis_tool_select_elliptical.h - part of Krayon^WKrita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
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

#ifndef __KIS_TOOL_SELECT_ELLIPTICAL_H__
#define __KIS_TOOL_SELECT_ELLIPTICAL_H__

#include <QPoint>

#include "kis_tool.h"
#include "kis_selection.h"
#include "KoToolFactory.h"
#include "kis_layer_shape.h"

class KisSelectionOptions;

class KisToolSelectElliptical : public KisTool {

    typedef KisTool super;
    Q_OBJECT

public:
    KisToolSelectElliptical(KoCanvasBase * canvas);
    virtual ~KisToolSelectElliptical();

//     virtual quint32 priority() { return 4; }
    virtual QWidget * createOptionWidget();
        virtual QWidget* optionWidget();
//     virtual enumToolType toolType() { return TOOL_SELECT; }

    virtual void paint(QPainter& gc, KoViewConverter &converter);

    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);

public slots:
    virtual void slotSetAction(int);
    virtual void activate();


private:
    void clearSelection();

private:
    QPointF m_centerPos;
    QPointF m_startPos;
    QPointF m_endPos;
    bool m_selecting;
    KisSelectionOptions * m_optWidget;
    enumSelectionMode m_selectAction;
};

class KisToolSelectEllipticalFactory : public KoToolFactory {

public:
    KisToolSelectEllipticalFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolSelectElliptical", i18n( "Elliptical Selection" ))
        {
            setToolTip( i18n( "Select an elliptical area" ) );
            setToolType( TOOL_TYPE_SELECTED);
//            setToolType( dynamicToolType() );
            setActivationShapeId( KIS_LAYER_SHAPE_ID );
            setIcon( "tool_elliptical_selection" );
            setShortcut( KShortcut(Qt::Key_J) );
            setPriority( 0 );
        }

    virtual ~KisToolSelectEllipticalFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectElliptical(canvas);
    }

};





#endif //__KIS_TOOL_SELECT_ELLIPTICAL_H__

