/*
 *  SPDX-FileCopyrightText: 2009-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_DYNA_H_
#define KIS_TOOL_DYNA_H_

#include "kis_tool_freehand.h"

#include "KisToolPaintFactoryBase.h"
#include "KoPointerEvent.h"

#include <flake/kis_node_shape.h>
#include <kis_icon.h>
#include <kconfig.h>
#include <kconfiggroup.h>


class KisDoubleSliderSpinBox;
class QCheckBox;
class QGridLayout;
class KisAngleSelector;

class KoCanvasBase;

class DynaFilter
{
public:
    DynaFilter() {}

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
    qreal curx {0.0}, cury {0.0};
    qreal velx {0.0}, vely {0.0}, vel {0.0};
    qreal accx {0.0}, accy {0.0}, acc {0.0};
    qreal angx {0.0}, angy {0.0};
    qreal mass {0.0}, drag {0.0};
    qreal lastx {0.0}, lasty {0.0};
    bool fixedangle {false};
};


class KisToolDyna : public KisToolFreehand
{
    Q_OBJECT

public:
    KisToolDyna(KoCanvasBase * canvas);
    ~KisToolDyna() override;

    QWidget * createOptionWidget() override;
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;

protected:
    void initStroke(KoPointerEvent *event) override;

protected Q_SLOTS:
    void resetCursorStyle() override;

private Q_SLOTS:

    void slotSetDynaWidth(double width);
    void slotSetMass(qreal mass);
    void slotSetDrag(qreal drag);
    void slotSetAngle(qreal angle);
    void slotSetWidthRange(double widthRange);
    void slotSetFixedAngle(bool fixedAngle);

private:
    QGridLayout* m_optionLayout;

    // dyna gui
    QCheckBox * m_chkFixedAngle;
    KisDoubleSliderSpinBox * m_massSPBox;
    KisDoubleSliderSpinBox * m_dragSPBox;
    KisAngleSelector * m_angleSelector;

    // dyna algorithm
    QVector<QPointF> m_prevPosition;
    qreal m_odelx, m_odely;

    // mouse info
    QPointF m_mousePos;

    qreal m_surfaceWidth;
    qreal m_surfaceHeight;

    // settings variables
    KConfigGroup m_configGroup;
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


class KisToolDynaFactory : public KisToolPaintFactoryBase
{

public:
    KisToolDynaFactory()
            : KisToolPaintFactoryBase("KritaShape/KisToolDyna") {

        setToolTip(i18n("Dynamic Brush Tool"));
        setSection(TOOL_TYPE_SHAPE);
        setIconName(koIconNameCStr("krita_tool_dyna"));
        //setShortcut(QKeySequence(Qt::Key_F));
        setPriority(10);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolDynaFactory() override {}

    KoToolBase *createTool(KoCanvasBase *canvas) override {
        return new KisToolDyna(canvas);
    }

};


#endif // KIS_TOOL_DYNA_H_

