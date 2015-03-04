/*
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TOOL_MULTIHAND_H
#define __KIS_TOOL_MULTIHAND_H

#include "kis_tool_brush.h"
#include <KoIcon.h>

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
    ~KisToolMultihand();
    void beginPrimaryAction(KoPointerEvent *event);
    void continuePrimaryAction(KoPointerEvent *event);
    void endPrimaryAction(KoPointerEvent *event);


protected:
    void paint(QPainter& gc, const KoViewConverter &converter);

    QWidget* createOptionWidget();

private:
    void initTransformations();
    void finishAxesSetup();
    void updateCanvas();

private Q_SLOTS:
    void activateAxesPointModeSetup();
    void slotSetHandsCount(int count);
    void slotSetAxesAngle(qreal angle);
    void slotSetTransformMode(int qcomboboxIndex);
    void slotSetAxesVisible(bool vis);
    void slotSetMirrorVertically(bool mirror);
    void slotSetMirrorHorizontally(bool mirror);
    void slotSetTranslateRadius(int radius);

private:
    KisToolMultihandHelper *m_helper;

    enum enumTransforModes { SYMMETRY, MIRROR, TRANSLATE };
    enumTransforModes m_transformMode;
    QPointF m_axesPoint;
    qreal m_angle;
    int m_handsCount;
    bool m_mirrorVertically;
    bool m_mirrorHorizontally;
    bool m_showAxes;
    int m_translateRadius;

    bool m_setupAxesFlag;
    QComboBox * m_transformModesComboBox;
    KisSliderSpinBox *m_handsCountSlider;
    KisDoubleSliderSpinBox *m_axesAngleSlider;
    QCheckBox *m_axesChCkBox;
    QStackedWidget *m_modeCustomOption;
    QCheckBox *m_mirrorVerticallyChCkBox;
    QCheckBox *m_mirrorHorizontallyChCkBox;
    KisSliderSpinBox *m_translateRadiusSlider;
    QPushButton *m_axesPointBtn;
};


class KisToolMultiBrushFactory : public KoToolFactoryBase
{

public:
    KisToolMultiBrushFactory(const QStringList&)
            : KoToolFactoryBase("KritaShape/KisToolMultiBrush") {

        setToolTip(i18n("Multibrush Tool"));

        // Temporarily
        setToolType(TOOL_TYPE_SHAPE);
        setIconName(koIconNameCStr("krita_tool_multihand"));
        setShortcut(KShortcut(Qt::Key_Q));
        setPriority(11);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    virtual ~KisToolMultiBrushFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolMultihand(canvas);
    }

};

#endif /* __KIS_TOOL_MULTIHAND_H */
