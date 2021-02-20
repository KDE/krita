/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_FREEHAND_HELPER_H
#define __KIS_TOOL_FREEHAND_HELPER_H

#include <QObject>
#include <QVector>

#include "kis_types.h"
#include "kritaui_export.h"
#include <brushengine/kis_paint_information.h>
#include "kis_default_bounds.h"
#include <brushengine/kis_paintop_settings.h>
#include "kis_smoothing_options.h"
#include "kundo2magicstring.h"


class KoPointerEvent;
class KoCanvasResourceProvider;
class KisPaintingInformationBuilder;
class KisStrokesFacade;
class KisPostExecutionUndoAdapter;
class KisPaintOp;
class KisFreehandStrokeInfo;


class KRITAUI_EXPORT KisToolFreehandHelper : public QObject
{
    Q_OBJECT

public:

    KisToolFreehandHelper(KisPaintingInformationBuilder *infoBuilder,
                          KoCanvasResourceProvider *resourceManager,
                          const KUndo2MagicString &transactionText = KUndo2MagicString(),
                          KisSmoothingOptions *smoothingOptions = 0);
    ~KisToolFreehandHelper() override;

    void setSmoothness(KisSmoothingOptionsSP smoothingOptions);
    KisSmoothingOptionsSP smoothingOptions() const;

    bool isRunning() const;

    void cursorMoved(const QPointF &cursorPos);

    /**
     * @param event The event
     * @param pixelCoords The position of the KoPointerEvent, in pixel coordinates.
     * @param resourceManager The canvas resource manager
     * @param image The image
     * @param currentNode The current node
     * @param strokesFacade The strokes facade
     * @param overrideNode The override node
     * @param bounds The bounds
     */
    void initPaint(KoPointerEvent *event,
                   const QPointF &pixelCoords,
                   KisImageWSP image,
                   KisNodeSP currentNode,
                   KisStrokesFacade *strokesFacade,
                   KisNodeSP overrideNode = 0,
                   KisDefaultBoundsBaseSP bounds = 0);
    void paintEvent(KoPointerEvent *event);
    void endPaint();

    QPainterPath paintOpOutline(const QPointF &savedCursorPos,
                                const KoPointerEvent *event,
                                const KisPaintOpSettingsSP globalSettings,
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

    void initPaintImpl(qreal startAngle,
                       const KisPaintInformation &pi,
                       KoCanvasResourceProvider *resourceManager,
                       KisImageWSP image,
                       KisNodeSP node,
                       KisStrokesFacade *strokesFacade,
                       KisNodeSP overrideNode = 0,
                       KisDefaultBoundsBaseSP bounds = 0);

    KoCanvasResourceProvider *resourceManager() const;

protected:

    virtual void createPainters(QVector<KisFreehandStrokeInfo*> &strokeInfos,
                                const KisDistanceInformation &startDist);

    // lo-level methods for painting primitives

    void paintAt(int strokeInfoId, const KisPaintInformation &pi);

    void paintLine(int strokeInfoId,
                   const KisPaintInformation &pi1,
                   const KisPaintInformation &pi2);

    void paintBezierCurve(int strokeInfoId,
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
    void paint(KisPaintInformation &info);
    void paintBezierSegment(KisPaintInformation pi1, KisPaintInformation pi2,
                                                   QPointF tangent1, QPointF tangent2);

    void stabilizerStart(KisPaintInformation firstPaintInfo);
    void stabilizerEnd();
    KisPaintInformation getStabilizedPaintInfo(const QQueue<KisPaintInformation> &queue,
                                               const KisPaintInformation &lastPaintInfo);
    int computeAirbrushTimerInterval() const;

    qreal currentZoom() const;

private Q_SLOTS:
    void finishStroke();
    void doAirbrushing();
    void stabilizerPollAndPaint();
    void slotSmoothingTypeChanged();

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_TOOL_FREEHAND_HELPER_H */
