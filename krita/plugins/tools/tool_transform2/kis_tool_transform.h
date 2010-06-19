/*
 *  kis_tool_transform.h - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#ifndef KIS_TOOL_TRANSFORM_H_
#define KIS_TOOL_TRANSFORM_H_

#include <QPoint>
#include <QPointF>

#include <KoInteractionTool.h>
#include <KoToolFactoryBase.h>

#include <kis_undo_adapter.h>
#include <kis_types.h>
#include <flake/kis_node_shape.h>
#include <kis_tool.h>

#include "ui_wdg_tool_transform.h"

class KoID;
class KisFilterStrategy;

class WdgToolTransform : public QWidget, public Ui::WdgToolTransform
{
    Q_OBJECT

public:
    WdgToolTransform(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * Transform tool
 *
 */
class KisToolTransform : public KisTool, KisCommandHistoryListener
{

    Q_OBJECT

public:
    KisToolTransform(KoCanvasBase * canvas);
    virtual ~KisToolTransform();

    virtual QWidget* createOptionWidget();
    virtual QWidget* optionWidget();

    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);
    void paint(QPainter& gc, const KoViewConverter &converter);

public:

    void notifyCommandAdded(const QUndoCommand *);
    void notifyCommandExecuted(const QUndoCommand *);

public slots:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();

private:

    void transform();
    void recalcOutline();
    QPointF rot(double x, double y) {
        return QPointF(m_cosa*x - m_sina*y, m_sina*x + m_cosa*y);
    }
    QPointF invrot(double x, double y) {
        return QPointF(m_cosa*x + m_sina*y, -m_sina*x + m_cosa*y);
    }
    int det(const QPointF & v, const QPointF & w);
    double distsq(const QPointF & v, const QPointF & w); //square of the euclidian distance
    void setFunctionalCursor();
    void initHandles();
    void updateOriginal();

private slots:

    void slotSetFilter(const KoID &);

private:
    enum function {ROTATE, MOVE, TOPLEFTSCALE, TOPSCALE, TOPRIGHTSCALE, RIGHTSCALE,
                   BOTTOMRIGHTSCALE, BOTTOMSCALE, BOTTOMLEFTSCALE, LEFTSCALE
                  };
    QCursor m_sizeCursors[8]; //cursors for the 8 directions
    function m_function; //current transformation function

	int m_handleRadius;
    
    //informations on the original selection (before any transformation)
	double m_originalWidth2, m_originalHeight2; //'2' for half
    QPoint m_originalTopLeft;  //in image coords
    QPoint m_originalBottomRight;  //in image coords
    QPointF m_originalCenter;   //in image coords
    QPointF m_translate;   //in image coords

	//informations on the bounding rect of the selection just after the LAST transformation
	//thus they are initially equal to original points
	//(used to know dirty rects more precisely)
    QPoint m_previousTopLeft;  //in image coords
    QPoint m_previousBottomRight;  //in image coords
    
    bool m_selecting; // true <=> selection has been clicked
    bool m_actuallyMoveWhileSelected; // true <=> selection has been moved while clicked
    
    //informations on the current selection (selection in transformation)
    QPointF m_topLeft;  //in image coords
    QPointF m_topRight;  //in image coords
    QPointF m_bottomLeft;  //in image coords
    QPointF m_bottomRight;  //in image coords
    
    //current scale factors
	//wOutModifier don't take shift modifier into account
    double m_scaleX, m_scaleX_wOutModifier;
    double m_scaleY, m_scaleY_wOutModifier;
    
	QPointF m_prevMousePos;
	QPointF m_refPoint; //the position of the reference point for the used function (ex : center of the selection for move, middle of the top side for topscale..)
    QPointF m_clickoffset; //the position of the click relative to the refPoint
    
    double m_cosa;
    double m_sina;
    double m_a;
    double m_clickangle;

    KisFilterStrategy *m_filter;

    WdgToolTransform *m_optWidget;

	KisPaintDeviceSP m_target;
    KisPaintDeviceSP m_origDevice;
    KisSelectionSP m_origSelection;
    KoCanvasBase *m_canvas;
};

class KisToolTransformFactory : public KoToolFactoryBase
{


public:

    KisToolTransformFactory(QObject *parent, const QStringList&)
            : KoToolFactoryBase(parent, "KisToolTransform") {
        setToolTip(i18n("Transform a layer or a selection"));
        setToolType(TOOL_TYPE_TRANSFORM);
        setIcon("krita_tool_transform");
        setPriority(11);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    virtual ~KisToolTransformFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolTransform(canvas);
    }

};



#endif // KIS_TOOL_TRANSFORM_H_

