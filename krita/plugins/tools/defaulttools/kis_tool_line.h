/*
 *  kis_tool_line.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@comuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_LINE_H_
#define KIS_TOOL_LINE_H_

#include "kis_tool_paint.h"

#include "kis_global.h"
#include "kis_types.h"
#include "KoToolFactoryBase.h"
#include "flake/kis_node_shape.h"
#include "kis_paint_information.h"
#include <KoIcon.h>

class KisPainter;
class QPoint;
class KoCanvasBase;
class KisRecordedPolyLinePaintAction;
class QCheckBox;
class QPushButton;

class KisToolLine : public KisToolPaint
{
    Q_OBJECT

public:
    KisToolLine(KoCanvasBase * canvas);
    virtual ~KisToolLine();

    virtual int flags() const;

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

    virtual QString quickHelp() const;

private:
    void paintLine(QPainter& gc, const QRect& rc);
    QPointF straightLine(QPointF point);
    void updatePreview();
    virtual QWidget* createOptionWidget();

private:
    KisPaintInformation m_startPos;
    KisPaintInformation m_endPos;
    KisPainter*         m_painter;
    QCheckBox*          m_cbPressure;
    QCheckBox*          m_cbTilt;
    QCheckBox*          m_cbRotation;
    QCheckBox*          m_cbTangPressure;
    QPushButton*        m_bnVaryingEnds;
    float               m_maxPressure;
};


class KisToolLineFactory : public KoToolFactoryBase
{

public:

    KisToolLineFactory(const QStringList&)
            : KoToolFactoryBase("KritaShape/KisToolLine") {
        setToolTip(i18n("Draw a straight line with the current brush"));
        // Temporarily
        setToolType(TOOL_TYPE_SHAPE);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setPriority(1);
        setIconName(koIconNameCStr("krita_tool_line"));
    }

    virtual ~KisToolLineFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolLine(canvas);
    }

};




#endif //KIS_TOOL_LINE_H_

