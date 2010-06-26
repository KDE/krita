/*
 *  kis_tool_transform.h - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2010 Marc Pegon
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

#include <complex>

#include <QPoint>
#include <QPointF>
#include <QVector3D>

#include <KoInteractionTool.h>
#include <KoToolFactoryBase.h>

#include <kis_shape_selection.h>
#include <kis_undo_adapter.h>
#include <kis_types.h>
#include <flake/kis_node_shape.h>
#include <kis_tool.h>

#include "ui_wdg_tool_transform.h"
#include "tool_transform_args.h"

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
    void recalcOutline();
	//update the boundrect of the current transformed pixels
	void updateCanvas();

public:

    void notifyCommandAdded(const QUndoCommand *);
    void notifyCommandExecuted(const QUndoCommand *);

public slots:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();

private:

    void transform();
    QVector3D rotX(double x, double y, double z) {
        return QVector3D(x, m_cosaX * y - m_sinaX * z, m_cosaX * z + m_sinaX * y);
    }
    QVector3D invrotX(double x, double y, double z) {
        return QVector3D(x, m_cosaX * y + m_sinaX * z, m_cosaX * z - m_sinaX * y);
    }
    QVector3D rotY(double x, double y, double z) {
        return QVector3D(m_cosaY * x + m_sinaY * z, y, - m_sinaY * x + m_cosaY * z);
    }
    QVector3D invrotY(double x, double y, double z) {
        return QVector3D(m_cosaY * x + m_sinaY * z, y, - m_sinaY * x + m_cosaY * z);
    }
    QVector3D rotZ(double x, double y, double z) {
        return QVector3D(m_cosaZ*x - m_sinaZ*y, m_sinaZ*x + m_cosaZ*y, z);
    }
    QVector3D invrotZ(double x, double y, double z) {
        return QVector3D(m_cosaZ*x + m_sinaZ*y, -m_sinaZ*x + m_cosaZ*y, z);
    }
	QVector3D shear(double x, double y, double z) {
		double x2 = x + y * m_currentArgs.shearX();
		return QVector3D(x2, y + x2 * m_currentArgs.shearY(), z);
	}
	QVector3D invshear(double x, double y, double z) {
		y -= x * m_currentArgs.shearY();
		x -= y * m_currentArgs.shearX();

		return QVector3D(x, y, z);
	}
	QVector3D scale(double x, double y, double z) {
		return QVector3D(x * m_currentArgs.scaleX(), y * m_currentArgs.scaleY(), z);
	}
	QVector3D invscale(double x, double y, double z) {
		return QVector3D(x / m_currentArgs.scaleX(), y / m_currentArgs.scaleY(), z);
	}

    int det(const QPointF & v, const QPointF & w);
    double distsq(const QPointF & v, const QPointF & w); //square of the euclidian distance
	int octant(double x, double y); //the octant of the director given by vector (x,y)
    void setFunctionalCursor();
    void initHandles();
	void storeArgs(ToolTransformArgs &args);
	void restoreArgs(ToolTransformArgs args);

private slots:

    void slotSetFilter(const KoID &);

private:
    enum function {ROTATE = 0, MOVE, RIGHTSCALE, TOPRIGHTSCALE, TOPSCALE, TOPLEFTSCALE,
				   LEFTSCALE, BOTTOMLEFTSCALE, BOTTOMSCALE, BOTTOMRIGHTSCALE, 
				   BOTTOMSHEAR, RIGHTSHEAR, TOPSHEAR, LEFTSHEAR,
				   MOVECENTER
                  };
	QPointF m_handleDir[8];

    QCursor m_sizeCursors[8]; //cursors for the 8 directions
    function m_function; //current transformation function

	ToolTransformArgs m_currentArgs;
	ToolTransformArgs m_clickArgs;

	int m_handleRadius;
	int m_rotationCenterRadius;
	int m_maxRadius;
    
    //informations on the original selection (before any transformation)
	double m_originalWidth2, m_originalHeight2; //'2' for half
    QPoint m_originalTopLeft;  //in image coords
    QPoint m_originalBottomRight;  //in image coords
    QPointF m_originalCenter; //original center of the selection

	//informations on the bounding rect of the selection just after the LAST transformation
	//thus they are initially equal to original points
	//(used to know dirty rects more precisely)
    QPoint m_previousTopLeft;  //in image coords
    QPoint m_previousBottomRight;  //in image coords

	//center used for rotation (calculated from rotationCenterOffset (in currentArgs))
	QPointF m_rotationCenter;
	QPointF m_clickRotationCenter; //the rotation center at click
    
    bool m_selecting; // true <=> selection has been clicked
    bool m_actuallyMoveWhileSelected; // true <=> selection has been moved while clicked
    
    //informations on the current selection
    QPointF m_topLeft;  //in image coords
    QPointF m_topRight;  //in image coords
    QPointF m_bottomLeft;  //in image coords
    QPointF m_bottomRight;  //in image coords
    
    //current scale factors
	//wOutModifier don't take shift modifier into account
    double m_scaleX_wOutModifier;
    double m_scaleY_wOutModifier;

	QPointF m_prevMousePos;
    QPointF m_clickPoint; //position of the mouse when click occured
    
    double m_cosaZ; //cos and sin of transformArgs.aZ()
    double m_sinaZ;
    double m_cosaX;
    double m_sinaX;
    double m_cosaY;
    double m_sinaY;
    double m_clickangle; //angle made at click, from the rotationCenter

    KisFilterStrategy *m_filter;

    WdgToolTransform *m_optWidget;

	KisPaintDeviceSP m_target;
    KisPaintDeviceSP m_origDevice;
    KisSelectionSP m_origSelection; //contains the original selection
	//KisShapeSelection *m_previousShapeSelection;
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

