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
#include "kis_paintop_settings.h"
#include "kis_smoothing_options.h"

class KoPointerEvent;
class KoCanvasResourceManager;
class KisPaintingInformationBuilder;
class KisRecordingAdapter;
class KisStrokesFacade;
class KisPostExecutionUndoAdapter;
class KisPaintOp;


class KRITAUI_EXPORT KisToolFreehandHelper : public QObject
{
    Q_OBJECT

protected:

    typedef FreehandStrokeStrategy::PainterInfo PainterInfo;

public:

    KisToolFreehandHelper(KisPaintingInformationBuilder *infoBuilder,
                          const KUndo2MagicString &transactionText = KUndo2MagicString(),
                          KisRecordingAdapter *recordingAdapter = 0);
    ~KisToolFreehandHelper();

    void setSmoothness(KisSmoothingOptionsSP smoothingOptions);
    KisSmoothingOptionsSP smoothingOptions() const;

    bool isRunning() const;

    void initPaint(KoPointerEvent *event,
                   KoCanvasResourceManager *resourceManager,
                   KisImageWSP image,
                   KisNodeSP currentNode,
                   KisStrokesFacade *strokesFacade,
                   KisPostExecutionUndoAdapter *undoAdapter,
                   KisNodeSP overrideNode = 0,
                   KisDefaultBoundsBaseSP bounds = 0);
    void paint(KoPointerEvent *event);
    void endPaint();

    const KisPaintOp* currentPaintOp() const;
    QPainterPath paintOpOutline(const QPointF &savedCursorPos,
                                const KoPointerEvent *event,
                                const KisPaintOpSettings *globalSettings,
                                KisPaintOpSettings::OutlineMode mode) const;

Q_SIGNALS:
    /**
     * The signal is emitted when the outline should be updated
     * explicitly by the tool. Used by Stabilizer option, because it
     * paints on internal timer events instead of the on every paint()
     * event
     */
    void requestExplicitUpdateOutline();

protected:
    void cancelPaint();
    int elapsedStrokeTime() const;

    void initPaintImpl(const KisPaintInformation &previousPaintInformation,
                       KoCanvasResourceManager *resourceManager,
                       KisImageWSP image,
                       KisNodeSP node,
                       KisStrokesFacade *strokesFacade,
                       KisPostExecutionUndoAdapter *undoAdapter,
                       KisNodeSP overrideNode = 0,
                       KisDefaultBoundsBaseSP bounds = 0);

protected:

    virtual void createPainters(QVector<PainterInfo*> &painterInfos,
                                const QPointF &lastPosition,
                                int lastTime);

    // lo-level methods for painting primitives

    void paintAt(int painterInfoId, const KisPaintInformation &pi);

    void paintLine(int painterInfoId,
                   const KisPaintInformation &pi1,
                   const KisPaintInformation &pi2);

    void paintBezierCurve(int painterInfoId,
                          const KisPaintInformation &pi1,
                          const QPointF &control1,
                          const QPointF &control2,
                          const KisPaintInformation &pi2);

    // hi-level methods for painting primitives

    virtual void paintAt(const KisPaintInformation &pi);

    virtual void paintLine(const KisPaintInformation &pi1,
                           const KisPaintInformation &pi2);

    virtual void paintBezierCurve(const KisPaintInformation &pi1,
                                  const QPointF &control1,
                                  const QPointF &control2,
                                  const KisPaintInformation &pi2);

private:
    void paintBezierSegment(KisPaintInformation pi1, KisPaintInformation pi2,
                                                   QPointF tangent1, QPointF tangent2);

    void stabilizerStart(KisPaintInformation firstPaintInfo);
    void stabilizerEnd();

private Q_SLOTS:

    void finishStroke();
    void doAirbrushing();
    void stabilizerPollAndPaint();

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_TOOL_FREEHAND_HELPER_H */
