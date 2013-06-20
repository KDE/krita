/*
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

#ifndef __KIS_TOOL_FREEHAND_HELPER_H
#define __KIS_TOOL_FREEHAND_HELPER_H

#include <QObject>

#include "kis_types.h"
#include "krita_export.h"
#include "kis_paint_information.h"
#include "strokes/freehand_stroke.h"
#include "kis_default_bounds.h"

class KoPointerEvent;
class KoCanvasResourceManager;
class KisPaintingInformationBuilder;
class KisRecordingAdapter;
class KisStrokesFacade;
class KisPostExecutionUndoAdapter;
class KisPaintOp;
class KisPainter;
struct KisSmoothingOptions;

class KRITAUI_EXPORT KisToolFreehandHelper : public QObject
{
    Q_OBJECT

protected:

    typedef FreehandStrokeStrategy::PainterInfo PainterInfo;

public:

    KisToolFreehandHelper(KisPaintingInformationBuilder *infoBuilder,
                          KisRecordingAdapter *recordingAdapter = 0);
    ~KisToolFreehandHelper();

    void setSmoothness(const KisSmoothingOptions &smoothingOptions);

    void initPaint(KoPointerEvent *event,
                   KoCanvasResourceManager *resourceManager,
                   KisImageWSP image,
                   KisStrokesFacade *strokesFacade,
                   KisPostExecutionUndoAdapter *undoAdapter,
                   KisNodeSP overrideNode = 0,
                   KisDefaultBoundsBaseSP bounds = 0);
    void paint(KoPointerEvent *event);
    void endPaint();

    const KisPaintOp* currentPaintOp() const;

protected:

    virtual void createPainters(QVector<PainterInfo*> &painterInfos);

    virtual void paintAt(const QVector<PainterInfo*> &painterInfos,
                         const KisPaintInformation &pi);

    virtual void paintLine(const QVector<PainterInfo*> &painterInfos,
                           const KisPaintInformation &pi1,
                           const KisPaintInformation &pi2);

    virtual void paintBezierCurve(const QVector<PainterInfo*> &painterInfos,
                                  const KisPaintInformation &pi1,
                                  const QPointF &control1,
                                  const QPointF &control2,
                                  const KisPaintInformation &pi2);


    void paintAt(PainterInfo *painterInfo, const KisPaintInformation &pi);

    void paintLine(PainterInfo *painterInfo,
                   const KisPaintInformation &pi1,
                   const KisPaintInformation &pi2);

    void paintBezierCurve(PainterInfo *painterInfo,
                          const KisPaintInformation &pi1,
                          const QPointF &control1,
                          const QPointF &control2,
                          const KisPaintInformation &pi2);

private slots:

    void finishStroke();
    void doAirbrushing();

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_TOOL_FREEHAND_HELPER_H */
