/*
 *  Copyright (c) 2003-2008 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_FREEHAND_H_
#define KIS_TOOL_FREEHAND_H_

#include "kis_types.h"
#include "kis_tool_paint.h"
#include "kis_paint_information.h"
#include "kis_resources_snapshot.h"
#include "kis_paintop_settings.h"
#include "kis_distance_information.h"

#include "krita_export.h"

// OpenGL
#include <opengl/kis_opengl.h>

class KAction;

class KoPointerEvent;
class KoCanvasBase;

class KisPainter;


class KisPaintingInformationBuilder;
class KisToolFreehandHelper;
class KisRecordingAdapter;


class KRITAUI_EXPORT KisToolFreehand : public KisToolPaint
{

    Q_OBJECT

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const QString & transactionText);
    virtual ~KisToolFreehand();
    virtual int flags() const;

protected:
    void gesture(const QPointF &offsetInDocPixels,
                 const QPointF &initialDocPoint);

    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual bool wantsAutoScroll() const;
    void activate(ToolActivation activation, const QSet<KoShape*> &shapes);
    void deactivate();

    virtual void initStroke(KoPointerEvent *event);
    virtual void doStroke(KoPointerEvent *event);
    virtual void endStroke();

    virtual void paint(QPainter& gc, const KoViewConverter &converter);


    KisPaintingInformationBuilder* paintingInformationBuilder() const;
    KisRecordingAdapter* recordingAdapter() const;
    void resetHelper(KisToolFreehandHelper *helper);
    void requestUpdateOutline(const QPointF &outlineDocPoint);

protected slots:

    void setSmooth(bool smooth);
    void setAssistant(bool assistant);

private:
    friend class KisToolPaintingInformationBuilder;

    /**
     * Adjusts a coordinates according to a KisPaintingAssitant,
     * if available.
     */
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);

    /**
     * Calculates a coefficient for KisPaintInformation
     * according to perspective grid values
     */
    qreal calculatePerspective(const QPointF &documentPoint);

    void showOutlineTemporary();

    QPainterPath getOutlinePath(const QPointF &documentPos,
                                KisPaintOpSettings::OutlineMode outlineMode);



private slots:
    void increaseBrushSize();
    void decreaseBrushSize();
    void hideOutline();

protected:
    bool m_smooth;
    double m_smoothness;
    bool m_assistant;
    double m_magnetism;

private:
#if defined(HAVE_OPENGL)
    qreal m_xTilt;
    qreal m_yTilt;

    qreal m_prevxTilt;
    qreal m_prevyTilt;

    GLuint m_displayList;
    QString m_brushModelName;
#endif

    QPointF m_outlineDocPoint;
    QTimer m_outlineTimer;
    QRectF m_oldOutlineRect;
    QPainterPath m_currentOutline;
    bool m_explicitShowOutline;

    KisPaintingInformationBuilder *m_infoBuilder;
    KisToolFreehandHelper *m_helper;
    KisRecordingAdapter *m_recordingAdapter;
};



#endif // KIS_TOOL_FREEHAND_H_

