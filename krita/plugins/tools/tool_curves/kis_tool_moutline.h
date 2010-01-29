/*
 *  kis_tool_moutline.h -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KIS_TOOL_MOUTLINE_H_
#define KIS_TOOL_MOUTLINE_H_

#include "KoToolFactoryBase.h"
#include "kis_curve_framework.h"
#include "kis_tool_curve.h"

class QSlider;
class KisToolMagnetic;
class Node;

typedef QVector<Node> NodeCol;
typedef QVector<NodeCol> NodeMatrix;
typedef QVector<qint16> GrayCol;
typedef QVector<GrayCol> GrayMatrix;

class KisToolMagnetic : public KisToolCurve
{

    Q_OBJECT

    friend class KisCurveMagnetic;

public:

    KisToolMagnetic(KoCanvasBase *canvas);
    ~KisToolMagnetic();

    virtual void setup(KActionCollection*);

    virtual void keyPress(QKeyEvent*);
    virtual void buttonPress(KoPointerEvent*);
    virtual void buttonRelease(KoPointerEvent*);
    virtual void move(KoPointerEvent*);

    virtual KisCurve::iterator selectByMouse(KisCurve::iterator it);

    bool editingMode() {
        return m_editingMode;
    }
    virtual QWidget* createOptionWidget();

public slots:

    virtual void activate();
    virtual void deactivate();

    void slotCommitCurve();
    void slotSetDistance(int);

private:

    KisCurveMagnetic *m_derived;
    QWidget* m_optWidget;
    QLabel* m_mode;
    QLabel* m_lbDistance;
    QSlider* m_slDistance;
    bool m_editingMode;
    bool m_editingCursor;
    bool m_draggingCursor;
    bool m_needNewPivot;

    int m_distance;

};

class KisToolMagneticFactory : public KoToolFactoryBase
{

public:
    KisToolMagneticFactory(QObject *parent, const QStringList&)
            : KoToolFactoryBase(parent, "KisToolMagnetic") {
        setToolTip(i18n("Magnetic Selection: move around an edge to select it. Hit Ctrl to enter/quit manual mode, and double click to finish."));
        setToolType(TOOL_TYPE_SELECTED);
        setIcon("tool_moutline");
        setPriority(0);
    };

    virtual ~KisToolMagneticFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolMagnetic(canvas);
    }

};

#endif // KIS_TOOL_MOUTLINE_H_
