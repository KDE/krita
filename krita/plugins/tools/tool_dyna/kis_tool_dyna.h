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

#include "KoToolFactory.h"
#include "KoPointerEvent.h"

#include <flake/kis_node_shape.h>

class QTimer;
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

    virtual void initPaint(KoPointerEvent *e);
    virtual void endPaint();


private slots:

    void timeoutPaint();
    void slotSetSmoothness(int smoothness);
    void slotSetMagnetism(int magnetism);
    void slotSetDynaWidth(double width);
    void slotSetMass(double mass);
    void slotSetDrag(double drag);
    void slotSetXangle(double angle);
    void slotSetYangle(double angle);
    void slotSetWidthRange(double widthRange);
    void slotSetFixedAngle(bool fixedAngle);

private:
    qint32 m_rate;
    QTimer * m_timer;
    QGridLayout* m_optionLayout;
    QCheckBox * m_chkSmooth;
    QCheckBox * m_chkAssistant;
    QSlider * m_sliderMagnetism;
    QSlider * m_sliderSmoothness;

    // dyna gui
    QCheckBox * m_chkFixedAngle;
    QDoubleSpinBox * m_initWidthSPBox;
    QDoubleSpinBox * m_massSPBox;
    QDoubleSpinBox * m_dragSPBox;
    QDoubleSpinBox * m_xAngleSPBox;
    QDoubleSpinBox * m_yAngleSPBox;
    QDoubleSpinBox * m_widthRangeSPBox;

    qreal m_previousPressure;

    // dyna algorithm
    QVector<QPointF> m_prevPosition;
    qreal m_odelx, m_odely;

    // mouse info
    QPointF m_mousePos;
    bool first;

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
    void initMouse(const QPointF &point) {
        m_mousePos.setX(point.x() / currentImage()->width());
        m_mousePos.setY(point.y() / currentImage()->height());
    }

    void initDyna();
    int applyFilter(qreal mx, qreal my);
    void drawSegment(KoPointerEvent * event);

};


class KisToolDynaFactory : public KoToolFactory
{

public:
    KisToolDynaFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KritaShape/KisToolDyna", i18n("Paint")) {

        setToolTip(i18n("Paint with brushes using dynamic movements"));

        // Temporarily
        setToolType(TOOL_TYPE_SHAPE);
        setIcon("krita_tool_dyna");
        // TODO
        //setShortcut(KShortcut(Qt::Key_F));
        setPriority(10);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setInputDeviceAgnostic(false);
    }

    virtual ~KisToolDynaFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolDyna(canvas);
    }

};


#endif // KIS_TOOL_DYNA_H_

