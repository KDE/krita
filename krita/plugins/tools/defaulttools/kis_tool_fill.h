/*
 *  kis_tool_fill.h - part of Krayon^Krita
 *
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#ifndef KIS_TOOL_FILL_H_
#define KIS_TOOL_FILL_H_

#include <QPoint>
//Added by qt3to4:
#include <QLabel>


#include "kis_tool_paint.h"
#include <flake/kis_node_shape.h>

class KisPainter;
class KisFillPainter;
class QWidget;
class QLabel;
class QCheckBox;
class KIntNumInput;
//class KActionCollection;

class KoCanvasBase;

class KisToolFill : public KisToolPaint
{

    Q_OBJECT

public:

    KisToolFill(KoCanvasBase * canvas);
    virtual ~KisToolFill();

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    bool flood(int startX, int startY);

    virtual QWidget * createOptionWidget();

public slots:
    virtual void slotSetThreshold(int);
    virtual void slotSetUsePattern(bool);
    virtual void slotSetSampleMerged(bool);
    virtual void slotSetFillSelection(bool);

private:
    QPointF m_startPos;
    int m_threshold;
    qint32 m_depth;
    quint8* m_oldColor, *m_color;
    KisPainter *m_painter;

    bool *m_map, m_unmerged, m_usePattern, m_fillOnlySelection;
    KisSelectionSP m_selection;

    QLabel *m_lbThreshold;
    KIntNumInput *m_slThreshold;
    QCheckBox *m_checkUsePattern;
    QCheckBox *m_checkSampleMerged;
    QCheckBox *m_checkFillSelection;
};


#include "KoToolFactory.h"

class KisToolFillFactory : public KoToolFactory
{

public:
    KisToolFillFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KritaFill/KisToolFill") {
        setToolTip(i18n("Fill a contiguous area of color with a color, or fill a selection."));
        setToolType(TOOL_TYPE_FILL);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setIcon("krita_tool_color_fill");
        //setShortcut( QKeySequence( Qt::Key_F ) );
        setPriority(14);
        setInputDeviceAgnostic(false);
    }

    virtual ~KisToolFillFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolFill(canvas);
    }

};

#endif //__filltool_h__

