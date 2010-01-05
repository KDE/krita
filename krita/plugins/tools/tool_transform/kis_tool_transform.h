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
#include <KoToolFactory.h>

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
    virtual void activate(bool temporary);
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
    double distsq(const QPointF & v, const QPointF & w);
    void setFunctionalCursor();
    void initHandles();

private slots:

    void slotSetFilter(const KoID &);

private:
    enum function {ROTATE, MOVE, TOPLEFTSCALE, TOPSCALE, TOPRIGHTSCALE, RIGHTSCALE,
                   BOTTOMRIGHTSCALE, BOTTOMSCALE, BOTTOMLEFTSCALE, LEFTSCALE
                  };
    QCursor m_sizeCursors[8];
    function m_function;
    QPoint m_originalTopLeft;  //in image coords
    QPoint m_originalBottomRight;  //in image coords
    QPointF m_originalCenter;   //in image coords
    QPointF m_translate;   //in image coords
    bool m_selecting;
    bool m_actualyMoveWhileSelected;
    QPointF m_topleft;  //in image coords
    QPointF m_topright;  //in image coords
    QPointF m_bottomleft;  //in image coords
    QPointF m_bottomright;  //in image coords
    double m_scaleX;
    double m_scaleY;
    QPointF m_clickoffset;
    double m_cosa;
    double m_sina;
    double m_a;
    double m_clickangle;
    KisFilterStrategy *m_filter;

    WdgToolTransform *m_optWidget;

    KisPaintDeviceSP m_origDevice;
    KisSelectionSP m_origSelection;
    KoCanvasBase *m_canvas;
};

class KisToolTransformFactory : public KoToolFactory
{


public:

    KisToolTransformFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolTransform") {
        setToolTip(i18n("Transform a layer or a selection"));
        setToolType(TOOL_TYPE_TRANSFORM);
        setIcon("krita_tool_transform");
        setPriority(11);

        //setActivationShapeId( KIS_NODE_SHAPE_ID );
    }

    virtual ~KisToolTransformFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolTransform(canvas);
    }

};



#endif // KIS_TOOL_TRANSFORM_H_

