/*
 *  kis_tool_transform.h - part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  Based on the transform tool from :
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#ifndef KIS_TOOL_PERSPECTIVETRANSFORM_H_
#define KIS_TOOL_PERSPECTIVETRANSFORM_H_

#include <qpoint.h>

#include <kis_layer.h>
#include <kis_point.h>
#include <kis_tool_non_paint.h>
#include <kis_tool_factory.h>
#include <kis_undo_adapter.h>
#include <kis_perspective_math.h>

class KisTransaction;
class WdgToolPerspectiveTransform;
class KisID;
class KisFilterStrategy;

/**
 * PerspectiveTransform tool
 *
 */
class KisToolPerspectiveTransform : public KisToolNonPaint, KisCommandHistoryListener {

    typedef KisToolNonPaint super;
    Q_OBJECT
    enum InterractionMode { DRAWRECTINTERRACTION, EDITRECTINTERRACTION };
    enum HandleSelected { NOHANDLE, TOPHANDLE, BOTTOMHANDLE, RIGHTHANDLE, LEFTHANDLE, MIDDLEHANDLE };
public:
    KisToolPerspectiveTransform();
    virtual ~KisToolPerspectiveTransform();

    virtual QWidget* createOptionWidget(QWidget* parent);
    virtual QWidget* optionWidget();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_TRANSFORM; }
    virtual Q_UINT32 priority() { return 0; }
    virtual void paint(KisCanvasPainter& gc);
    virtual void paint(KisCanvasPainter& gc, const QRect& rc);
    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);
    void paintOutline();

public:

    void notifyCommandAdded(KCommand *);
    void notifyCommandExecuted(KCommand *);

public:
    virtual void deactivate();

private:

    bool mouseNear(const QPoint& mousep, const QPoint point);
    void paintOutline(KisCanvasPainter& gc, const QRect& rc);
    void transform();
    void initHandles();

private slots:
    void slotLayerActivated(KisLayerSP);

protected slots:
    virtual void activate();

private:
    bool m_dragging;
    InterractionMode m_interractionMode;
    QRect m_initialRect;
    KisPoint m_dragStart, m_dragEnd;
    KisPoint m_topleft, m_topright, m_bottomleft, m_bottomright;
    KisPoint* m_currentSelectedPoint;
    bool m_actualyMoveWhileSelected;
    
    WdgToolPerspectiveTransform *m_optWidget;
    
    KisPaintDeviceSP m_origDevice;
    KisSelectionSP m_origSelection;
    int m_handleHalfSize, m_handleSize;
    
    // The following variables are used in during the draw rect interraction mode
    typedef QValueVector<KisPoint> KisPointVector;
    KisPointVector m_points;
    // The following variables are used when moving a middle handle
    HandleSelected m_handleSelected;
    
};

class KisToolPerspectiveTransformFactory : public KisToolFactory {
    typedef KisToolFactory super;

public:
    KisToolPerspectiveTransformFactory() : super() {};
    virtual ~KisToolPerspectiveTransformFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t = new KisToolPerspectiveTransform();
        Q_CHECK_PTR(t);
        t->setup(ac); return t;
    }
    virtual KisID id() { return KisID("perspective transform", i18n("Perspective transform Tool")); }
};



#endif // KIS_TOOL_TRANSFORM_H_

