/*
 *
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_MEASURE_H_
#define KIS_TOOL_MEASURE_H_

#include <QLabel>

#include <KoUnit.h>

#include "kis_tool.h"
#include "kis_global.h"
#include "kis_types.h"
#include "KoToolFactoryBase.h"
#include "flake/kis_node_shape.h"
#include <kis_icon.h>

#include <QVector2D>

class QPointF;
class QWidget;
class QVector2D;

class KoCanvasBase;


class KisToolMeasureOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    KisToolMeasureOptionsWidget(QWidget* parent, KisImageWSP image);

public Q_SLOTS:
    void slotSetDistance(double distance);
    void slotSetAngle(double angle);
    void slotUnitChanged(int index);
    void slotResolutionChanged(double xRes, double yRes);

private:
    void updateDistance();

    QLabel* m_angleLabel {nullptr};
    double m_distance {0.0};
public:
    QLabel* m_distanceLabel {nullptr};
    double m_resolution;
    KoUnit m_unit;
};

class KisToolMeasure : public KisTool
{

    Q_OBJECT

public:
    KisToolMeasure(KoCanvasBase * canvas);
    ~KisToolMeasure() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void showDistanceAngleOnCanvas();

    QPointF lockedAngle(QPointF pos);

    void paint(QPainter& gc, const KoViewConverter &converter) override;

    QWidget * createOptionWidget() override;

Q_SIGNALS:
    void sigDistanceChanged(double distance);
    void sigAngleChanged(double angle);

private:
    QRectF boundingRect();
    double angle();
    double distance();

private:
    KisToolMeasureOptionsWidget *m_optionsWidget {nullptr};

    QPointF m_startPos {QPointF(0, 0)};
    QPointF m_endPos {QPointF(0, 0)}; 
    QVector2D m_baseLineVec {QPointF(1, 0)};
    bool m_chooseBaseLineVec {false};
};


class KisToolMeasureFactory : public KoToolFactoryBase
{

public:

    KisToolMeasureFactory()
            : KoToolFactoryBase("KritaShape/KisToolMeasure") {
        setSection(ToolBoxSection::View);
        setToolTip(i18n("Measure Tool"));
        setIconName(koIconNameCStr("krita_tool_measure"));
        setPriority(1);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolMeasureFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolMeasure(canvas);
    }

};




#endif //KIS_TOOL_MEASURE_H_
