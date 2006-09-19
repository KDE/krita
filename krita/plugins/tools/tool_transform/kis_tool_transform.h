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

#include <kis_tool_non_paint.h>
#include <kis_tool_factory.h>
#include <kis_undo_adapter.h>

#include "ui_wdg_tool_transform.h"

class KisTransaction;
class KoID;
class KisFilterStrategy;

class WdgToolTransform : public QWidget, public Ui::WdgToolTransform
{
    Q_OBJECT

    public:
        WdgToolTransform(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

/**
 * Transform tool
 *
 */
class KisToolTransform : public KisToolNonPaint, KisCommandHistoryListener {

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolTransform();
    virtual ~KisToolTransform();

    virtual QWidget* createOptionWidget(QWidget* parent);
    virtual QWidget* optionWidget();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_TRANSFORM; }
    virtual quint32 priority() { return 0; }
    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);
    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);
    void setScaleX(double sx) { m_scaleX = sx; }
    void setScaleY(double sy) { m_scaleY = sy; }
    void setTranslateX(double tx) { m_translateX = tx; }
    void setTranslateY(double ty) { m_translateY = ty; }
    void setAngle(double a) { m_a = a; }
    void paintOutline();

public:

    void notifyCommandAdded(KCommand *);
    void notifyCommandExecuted(KCommand *);

public:
    virtual void deactivate();

private:

    void paintOutline(QPainter& gc, const QRect& rc);
    void transform();
    void recalcOutline();
    double rotX(double x, double y) { return m_cosa*x - m_sina*y;};
    double rotY(double x, double y) { return m_sina*x + m_cosa*y;};
    double invrotX(double x, double y) { return m_cosa*x + m_sina*y;};
    double invrotY(double x, double y) { return -m_sina*x + m_cosa*y;};
    int det(QPoint v,QPoint w);
    int distsq(QPoint v,QPoint w);
    void setFunctionalCursor();
    void initHandles();

private slots:

    void slotSetFilter(const KoID &);
    void setStartX(int x) { m_startPos.setX(x); }
    void setStartY(int y) { m_startPos.setY(y); }
    void setEndX(int x) { m_endPos.setX(x); }
    void setEndY(int y) { m_endPos.setY(y); }

protected slots:
    virtual void activate();

private:
    enum function {ROTATE,MOVE,TOPLEFTSCALE,TOPSCALE,TOPRIGHTSCALE,RIGHTSCALE,
                BOTTOMRIGHTSCALE, BOTTOMSCALE,BOTTOMLEFTSCALE, LEFTSCALE};
    QCursor m_sizeCursors[8];
    function m_function;
    QPoint m_startPos;
    QPoint m_endPos;
    bool m_selecting;
    QPoint m_topleft;
    QPoint m_topright;
    QPoint m_bottomleft;
    QPoint m_bottomright;
    double m_scaleX;
    double m_scaleY;
    double m_translateX;
    double m_translateY;
    QPoint m_clickoffset;
    double m_org_cenX;
    double m_org_cenY;
    double m_cosa;
    double m_sina;
    double m_a;
    double m_clickangle;
    KisFilterStrategy *m_filter;

    WdgToolTransform *m_optWidget;

    KisPaintDeviceSP m_origDevice;
    KisSelectionSP m_origSelection;

};

class KisToolTransformFactory : public KisToolFactory {
    typedef KisToolFactory super;

public:
    KisToolTransformFactory() : super() {};
    virtual ~KisToolTransformFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t = new KisToolTransform();
        Q_CHECK_PTR(t);
        t->setup(ac); return t;
    }
    virtual KoID id() { return KoID("transform", i18n("Transform Tool")); }
};



#endif // KIS_TOOL_TRANSFORM_H_

