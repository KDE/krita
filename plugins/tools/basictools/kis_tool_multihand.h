/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_MULTIHAND_H
#define __KIS_TOOL_MULTIHAND_H

#include "kis_tool_brush.h"
#include <kis_icon.h>
#include "kis_tool_multihand_config.h"

class QPushButton;
class QCheckBox;
class QComboBox;
class QStackedWidget;
class KisSliderSpinBox;
class KisToolMultihandHelper;

class KisToolMultihand : public KisToolBrush
{
    Q_OBJECT
public:
    KisToolMultihand(KoCanvasBase *canvas);
    ~KisToolMultihand() override;
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void endAlternateAction(KoPointerEvent *event, AlternateAction action) override;

    void mouseMoveEvent(KoPointerEvent* event) override;


protected:
    void paint(QPainter& gc, const KoViewConverter &converter) override;
    
    QWidget* createOptionWidget() override;

private:
    void initTransformations();
    void finishAxesSetup();
    void updateCanvas();

private Q_SLOTS:
    void activateAxesPointModeSetup();
    void resetAxes();
    void slotSetHandsCount(int count);
    void slotSetAxesAngle(qreal angle);
    void slotSetTransformMode(int qcomboboxIndex);
    void slotSetAxesVisible(bool vis);
    void slotSetMirrorVertically(bool mirror);
    void slotSetMirrorHorizontally(bool mirror);
    void slotSetTranslateRadius(int radius);
    void slotAddSubbrushesMode(bool checked);
    void slotRemoveAllSubbrushes();

private:
    KisToolMultihandHelper *m_helper;

    enum enumTransforModes:int { SYMMETRY=0, MIRROR, TRANSLATE, SNOWFLAKE, COPYTRANSLATE };
    enumTransforModes m_transformMode;
    QPointF m_axesPoint;
    qreal m_angle;
    int m_handsCount;
    bool m_mirrorVertically;
    bool m_mirrorHorizontally;
    bool m_showAxes;
    int m_translateRadius;

    bool m_setupAxesFlag;
    bool m_addSubbrushesMode;
    QPointF m_lastToolPos;
    QVector<QPointF> m_subbrOriginalLocations;

    KisToolMultiHandConfigWidget* customUI;
};


class KisToolMultiBrushFactory : public KisToolBrushFactory
{

public:
    KisToolMultiBrushFactory()
        : KisToolBrushFactory("KritaShape/KisToolMultiBrush") {

        setToolTip(i18n("Multibrush Tool"));

        // Temporarily
        setSection(TOOL_TYPE_SHAPE);
        setIconName(koIconNameCStr("krita_tool_multihand"));
        setShortcut(QKeySequence(Qt::Key_Q));
        setPriority(11);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolMultiBrushFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolMultihand(canvas);
    }

};

#endif /* __KIS_TOOL_MULTIHAND_H */
