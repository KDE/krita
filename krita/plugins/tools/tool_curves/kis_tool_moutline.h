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

#include "KoToolFactory.h"
#include "kis_curve_framework.h"
#include "kis_tool_curve.h"

class QSlider;
class KisToolMagnetic;
class KisVector2D;
class Node;

typedef Q3ValueVector<Node> NodeCol;
typedef Q3ValueVector<NodeCol> NodeMatrix;
typedef Q3ValueVector<qint16> GrayCol;
typedef Q3ValueVector<GrayCol> GrayMatrix;

class KisCurveMagnetic : public KisCurve {

    typedef KisCurve super;

    KisToolMagnetic *m_parent;

    void reduceMatrix (QRect&, GrayMatrix&, int, int, int, int);
    void findEdge (int, int, const GrayMatrix&, Node&);
    void detectEdges (const QRect&, KisPaintDeviceSP, GrayMatrix&);

    void gaussianBlur (const QRect&, KisPaintDeviceSP, KisPaintDeviceSP);
    void toGrayScale (const QRect&, KisPaintDeviceSP, GrayMatrix&);
    void getDeltas (const GrayMatrix&, GrayMatrix&, GrayMatrix&);
    void getMagnitude (const GrayMatrix&, const GrayMatrix&, GrayMatrix&);
    void nonMaxSupp (const GrayMatrix&, const GrayMatrix&, const GrayMatrix&, GrayMatrix&);

public:

    KisCurveMagnetic (KisToolMagnetic *parent);
    ~KisCurveMagnetic ();

    virtual KisCurve::iterator addPivot (iterator, const QPointF&);
    virtual KisCurve::iterator pushPivot (const QPointF&);
    virtual void calculateCurve (iterator, iterator, iterator);

};

class KisToolMagnetic : public KisToolCurve {

    typedef KisToolCurve super;
    Q_OBJECT

    friend class KisCurveMagnetic;

public:

    KisToolMagnetic();
    ~KisToolMagnetic();

    virtual void setup (KActionCollection*);
    virtual enumToolType toolType() { return TOOL_SELECT; }
    virtual Q_UINT32 priority() { return 9; }

    virtual void keyPress(QKeyEvent*);
    virtual void buttonPress(KoPointerEvent*);
    virtual void buttonRelease(KoPointerEvent*);
    virtual void move(KoPointerEvent*);

    virtual KisCurve::iterator selectByMouse(KisCurve::iterator it);

    bool editingMode() {return m_editingMode;}
    virtual QWidget* createOptionWidget();

public slots:

    virtual void activate ();
    virtual void deactivate ();

    void slotCommitCurve ();
    void slotSetDistance (int);

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

class KisToolMagneticFactory : public KoToolFactory {

public:
    KisToolMagneticFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolMagnetic", i18n( "Magnetic Outline Selection")
        {
            setToolTip( i18n( "Magnetic Selection: move around an edge to select it. Hit Ctrl to enter/quit manual mode, and double click to finish." ) );
            setToolType( TOOL_TYPE_SELECTED );
            setIcon( "tool_moutline" );
            setPriority( 0 );
        };

    virtual ~KisToolMagneticFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolMagnetic(canvas);
    }

};

#endif // KIS_TOOL_MOUTLINE_H_
