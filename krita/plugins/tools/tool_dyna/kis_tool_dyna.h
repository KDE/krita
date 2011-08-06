/*
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_TOOL_DYNA_H_
#define KIS_TOOL_DYNA_H_

#include "kis_tool_freehand.h"

#include "KoToolFactoryBase.h"
#include "KoPointerEvent.h"

#include <flake/kis_node_shape.h>

class QCheckBox;
class QComboBox;
class QGridLayout;
class QSlider;

class QDoubleSpinBox;

class KoCanvasBase;

class DynaFilter
{
public:
    DynaFilter() {
        curx = 0;
        cury = 0;
        lastx = 0;
        lasty = 0;
        velx = 0.0;
        vely = 0.0;
        accx = 0.0;
        accy = 0.0;
    }

    void init(qreal x, qreal y) {
        curx = x;
        cury = y;
        lastx = x;
        lasty = y;
        velx = 0.0;
        vely = 0.0;
        accx = 0.0;
        accy = 0.0;
    }

    ~DynaFilter() {}

public:
    qreal curx, cury;
    qreal velx, vely, vel;
    qreal accx, accy, acc;
    qreal angx, angy;
    qreal mass, drag;
    qreal lastx, lasty;
    bool fixedangle;
};


class KisToolDyna : public KisToolFreehand
{
    Q_OBJECT

public:
    KisToolDyna(KoCanvasBase * canvas);
    virtual ~KisToolDyna();

    // TODO vymyslet GUI
    QWidget * createOptionWidget();

    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);

protected:
    virtual void initStroke(KoPointerEvent *event);

private slots:

    void slotSetDynaWidth(double width);
    void slotSetMass(double mass);
    void slotSetDrag(double drag);
    void slotSetXangle(double angle);
    void slotSetYangle(double angle);
    void slotSetWidthRange(double widthRange);
    void slotSetFixedAngle(bool fixedAngle);

private:
    // FIXME: investigate why this variable is needed
    double m_dragDist;

    QGridLayout* m_optionLayout;

    // dyna gui
    QCheckBox * m_chkFixedAngle;
    QDoubleSpinBox * m_initWidthSPBox;
    QDoubleSpinBox * m_massSPBox;
    QDoubleSpinBox * m_dragSPBox;
    QDoubleSpinBox * m_xAngleSPBox;
    QDoubleSpinBox * m_yAngleSPBox;
    QDoubleSpinBox * m_widthRangeSPBox;

    // dyna algorithm
    QVector<QPointF> m_prevPosition;
    qreal m_odelx, m_odely;

    // mouse info
    QPointF m_mousePos;

    qreal m_surfaceWidth;
    qreal m_surfaceHeight;
    
    // settings variables
    qreal m_width;
    qreal m_curmass;
    qreal m_curdrag;
    DynaFilter m_mouse;
    qreal m_xangle;
    qreal m_yangle;
    qreal m_widthRange;

    // methods
    qreal flerp(qreal f0, qreal f1, qreal p) {
        return ((f0 *(1.0 - p)) + (f1 * p));
    }
    void setMousePosition(const QPointF &point) {
        m_mousePos.setX(point.x() / m_surfaceWidth );
        m_mousePos.setY(point.y() / m_surfaceHeight);
    }

    void initDyna();
    int applyFilter(qreal mx, qreal my);
    KoPointerEvent filterEvent(KoPointerEvent * event);

};


class KisToolDynaFactory : public KoToolFactoryBase
{

public:
    KisToolDynaFactory(const QStringList&)
            : KoToolFactoryBase("KritaShape/KisToolDyna") {

        setToolTip(i18n("Paint with brushes using dynamic movements"));

        // Temporarily
        setToolType(TOOL_TYPE_SHAPE);
        setIcon("krita_tool_dyna");
        // TODO
        //setShortcut(KShortcut(Qt::Key_F));
        setPriority(10);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setInputDeviceAgnostic(false);
    }

    virtual ~KisToolDynaFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolDyna(canvas);
    }

};


#endif // KIS_TOOL_DYNA_H_

