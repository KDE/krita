/*
 *  kis_tool_transform.h - part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
 *
 *  Based on the transform tool from :
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_TOOL_PERSPECTIVETRANSFORM_H_
#define KIS_TOOL_PERSPECTIVETRANSFORM_H_

#include <QPoint>
#include <QVector>

#include <kactioncollection.h>

#include <KoToolFactory.h>

#include <kis_layer.h>
#include "kis_tool.h"
#include <kis_undo_adapter.h>
#include <kis_perspective_math.h>


class WdgToolPerspectiveTransform;

/**
 * PerspectiveTransform tool
 *
 */
class KisToolPerspectiveTransform : public KisTool, KisCommandHistoryListener
{

    Q_OBJECT

    enum InterractionMode { DRAWRECTINTERRACTION, EDITRECTINTERRACTION };
    enum HandleSelected { NOHANDLE, TOPHANDLE, BOTTOMHANDLE, RIGHTHANDLE, LEFTHANDLE, MIDDLEHANDLE };
public:
    KisToolPerspectiveTransform(KoCanvasBase * canvas);
    virtual ~KisToolPerspectiveTransform();
#if 0
    virtual QWidget* createOptionWidget();
    virtual QWidget* optionWidget();
#endif
    virtual void setup(KActionCollection *collection);
    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    virtual void paint(QPainter &painter, const QRect& rc);
    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);
    void paintOutline();

public:

    void notifyCommandAdded(const QUndoCommand *);
    void notifyCommandExecuted(const QUndoCommand *);

private:

    bool mouseNear(const QPoint& mousep, const QPoint point);
    void paintOutline(QPainter& gc, const QRect& rc);
    void transform();
    void initHandles();

protected slots:
    virtual void activate(bool);
    virtual void deactivate();

private:
    bool m_dragging;
    InterractionMode m_interractionMode;
    QRect m_initialRect;
    QPointF m_dragStart, m_dragEnd;
    QPointF m_topleft, m_topright, m_bottomleft, m_bottomright;
    QPointF* m_currentSelectedPoint;
    bool m_actualyMoveWhileSelected;

    WdgToolPerspectiveTransform *m_optWidget;

    KisPaintDeviceSP m_origDevice;
    KisSelectionSP m_origSelection;
    int m_handleHalfSize, m_handleSize;

    // The following variables are used in during the draw rect interraction mode
    typedef QVector<QPointF> QPointFVector;
    QPointFVector m_points;
    // The following variables are used when moving a middle handle
    HandleSelected m_handleSelected;

};

class KisToolPerspectiveTransformFactory : public KoToolFactory
{

public:
    KisToolPerspectiveTransformFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolPerspectiveTransform") {
        setToolTip(i18n("Transform the perspective appearance of a layer or a selection"));
        setToolType(TOOL_TYPE_TRANSFORM);
        setIcon("tool_perspectivetransform");
        setPriority(12);
        setActivationShapeId("flake/edit");
    };

    virtual ~KisToolPerspectiveTransformFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolPerspectiveTransform(canvas);
    }

};



#endif // KIS_TOOL_TRANSFORM_H_

