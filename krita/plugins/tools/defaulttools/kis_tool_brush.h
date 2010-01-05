/*
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_BRUSH_H_
#define KIS_TOOL_BRUSH_H_

#include "kis_tool_freehand.h"

#include "KoToolFactory.h"

#include <flake/kis_node_shape.h>

class QTimer;
class QCheckBox;
class QComboBox;
class QGridLayout;
class QSlider;

class KoCanvasBase;
class KoSliderCombo;

class KisToolBrush : public KisToolFreehand
{
    Q_OBJECT

public:
    KisToolBrush(KoCanvasBase * canvas);
    virtual ~KisToolBrush();

    QWidget * createOptionWidget();

    virtual void mouseMoveEvent(KoPointerEvent *e);

protected:

    virtual void initPaint(KoPointerEvent *e);
    virtual void endPaint();


private slots:

    void timeoutPaint();
    void slotSetRate(qreal rate);
    void slotSetSmoothness(int smoothness);
    void slotSetMagnetism(int magnetism);

private:

    qint32 m_rate;
    QTimer * m_timer;
    QGridLayout* m_optionLayout;
    QCheckBox * m_chkSmooth;
    QCheckBox * m_chkAssistant;
    QSlider * m_sliderMagnetism;
    QSlider * m_sliderSmoothness;
    KoSliderCombo * m_sliderRate;
};


class KisToolBrushFactory : public KoToolFactory
{

public:
    KisToolBrushFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KritaShape/KisToolBrush") {

        setToolTip(i18n("Paint with brushes"));

        // Temporarily
        setToolType(TOOL_TYPE_SHAPE);
        setIcon("krita_tool_freehand");
        setShortcut(KShortcut(Qt::Key_B));
        setPriority(0);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setInputDeviceAgnostic(false);
    }

    virtual ~KisToolBrushFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolBrush(canvas);
    }

};


#endif // KIS_TOOL_BRUSH_H_

