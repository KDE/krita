/*
 *  SPDX-FileCopyrightText: 2019 Kuntal Majumder <hellozee@disroot.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisToolSelectMagnetic.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QVBoxLayout>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <KoPointerEvent.h>
#include <KoShapeController.h>
#include <KoPathShape.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoViewConverter.h>

#include <kis_layer.h>
#include <kis_selection_options.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_default_bounds.h>

#include "canvas/kis_canvas2.h"
#include "kis_painter.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include <brushengine/kis_paintop_registry.h>
#include <kis_command_utils.h>
#include <kis_selection_filters.h>
#include <KisCursorOverrideLock.h>

#include "kis_algebra_2d.h"

#include "KisHandlePainterHelper.h"
#include <KisOptimizedBrushOutline.h>
#include <kis_slider_spin_box.h>

#define FEEDBACK_LINE_WIDTH 2

KisToolSelectMagnetic::KisToolSelectMagnetic(KoCanvasBase *canvas)
    : KisToolSelect(canvas,
                    KisCursor::load("tool_magnetic_selection_cursor.png", 6, 6),
                    i18n("Magnetic Selection"))
  , m_worker(nullptr)
  , m_mouseHoverCompressor(100, KisSignalCompressor::FIRST_ACTIVE)
{ }

void KisToolSelectMagnetic::keyPressEvent(QKeyEvent *event)
{
    if (isSelecting()) {
        if (event->key() == Qt::Key_Control) {
            m_continuedMode = true;
        }
    }
    KisToolSelect::keyPressEvent(event);
}

/*
 * Calculates the checkpoints responsible to determining the last point from where
 * the edge is calculated.
 * Takes 3 point, min, median and max, searches for an edge point from median to max, if fails,
 * searches for the same from median to min, if fails, median becomes that edge point.
 */
void KisToolSelectMagnetic::calculateCheckPoints(vQPointF points)
{
    qreal totalDistance = 0.0;
    int checkPoint      = 0;
    int finalPoint      = 2;
    int midPoint        = 1;
    int minPoint        = 0;
    qreal maxFactor     = 2;

    for (; finalPoint < points.count(); finalPoint++) {
        totalDistance += kisDistance(points[finalPoint], points[finalPoint - 1]);

        if (totalDistance <= m_anchorGap / 3.0) {
            minPoint = finalPoint;
        }

        if (totalDistance <= m_anchorGap) {
            midPoint = finalPoint;
        }

        if (totalDistance > maxFactor * m_anchorGap) {
            break;
        }
    }

    if (totalDistance > maxFactor * m_anchorGap) {
        bool foundSomething = false;

        for (int i = midPoint; i < finalPoint; i++) {
            if (m_worker->intensity(points.at(i).toPoint()) >= m_threshold) {
                m_lastAnchor = points.at(i).toPoint();
                m_anchorPoints.push_back(m_lastAnchor);

                vQPointF temp;
                for (int j = 0; j <= i; j++) {
                    temp.push_back(points[j]);
                }

                m_pointCollection.push_back(temp);
                foundSomething = true;
                checkPoint     = i;
                break;
            }
        }

        if (!foundSomething) {
            for (int i = midPoint - 1; i >= minPoint; i--) {
                if (m_worker->intensity(points.at(i).toPoint()) >= m_threshold) {
                    m_lastAnchor = points.at(i).toPoint();
                    m_anchorPoints.push_back(m_lastAnchor);
                    vQPointF temp;
                    for (int j = midPoint - 1; j >= i; j--) {
                        temp.push_front(points[j]);
                    }

                    m_pointCollection.push_back(temp);
                    foundSomething = true;
                    checkPoint     = i;
                    break;
                }
            }
        }

        if (!foundSomething) {
            m_lastAnchor = points[midPoint].toPoint();
            m_anchorPoints.push_back(m_lastAnchor);
            vQPointF temp;

            for (int j = 0; j <= midPoint; j++) {
                temp.push_back(points[j]);
            }

            m_pointCollection.push_back(temp);
            checkPoint     = midPoint;
            foundSomething = true;
        }
    }

    totalDistance = 0.0;
    reEvaluatePoints();

    for (; finalPoint < points.count(); finalPoint++) {
        totalDistance += kisDistance(points[finalPoint], points[checkPoint]);
        if (totalDistance > maxFactor * m_anchorGap) {
            points.remove(0, checkPoint + 1);
            calculateCheckPoints(points);
            break;
        }
    }
} // KisToolSelectMagnetic::calculateCheckPoints

void KisToolSelectMagnetic::keyReleaseEvent(QKeyEvent *event)
{
    if (isSelecting()) {
        if (event->key() == Qt::Key_Control ||
            !(event->modifiers() & Qt::ControlModifier)) {

            m_continuedMode = false;
            if (mode() != PAINT_MODE) {
                // Prevent finishing the selection if there is only one point, since
                // finishSelectionAction will deselect the current selection. That
                // is fine if the user just clicks, but not if we are in continued
                // mode
                if (m_points.count() > 1) {
                    finishSelectionAction();
                }
                m_points.clear(); // ensure points are always cleared
            }
        }
    }

    KisToolSelect::keyReleaseEvent(event);
}

vQPointF KisToolSelectMagnetic::computeEdgeWrapper(QPoint a, QPoint b)
{
    return m_worker->computeEdge(m_searchRadius, a, b, m_filterRadius);
}

// the cursor is still tracked even when no mousebutton is pressed
void KisToolSelectMagnetic::mouseMoveEvent(KoPointerEvent *event)
{
    if (isMovingSelection()) {
        KisToolSelect::mouseMoveEvent(event);
        return;
    }

    m_lastCursorPos = convertToPixelCoord(event);
    if (isSelecting()) {
        updatePaintPath();
    }
    KisToolSelect::mouseMoveEvent(event);
} // KisToolSelectMagnetic::mouseMoveEvent

// press primary mouse button
void KisToolSelectMagnetic::beginPrimaryAction(KoPointerEvent *event)
{
    KisToolSelectBase::beginPrimaryAction(event);
    if (isMovingSelection()) {
        return;
    }

    setMode(KisTool::PAINT_MODE);
    QPointF temp(convertToPixelCoord(event));

    if (!image()->bounds().contains(temp.toPoint())) {
        return;
    }

    m_cursorOnPress = temp;

    checkIfAnchorIsSelected(temp);

    if (m_complete || m_selected) {
        return;
    }

    if (m_anchorPoints.count() != 0) {
        vQPointF edge = computeEdgeWrapper(m_anchorPoints.last(), temp.toPoint());
        m_points.append(edge);
        m_pointCollection.push_back(edge);
    } else {
        beginSelectInteraction();
        updateInitialAnchorBounds(temp.toPoint());
        emit setButtonsEnabled(true);
    }

    m_lastAnchor = temp.toPoint();
    m_anchorPoints.push_back(m_lastAnchor);
    m_lastCursorPos = temp;
    reEvaluatePoints();
    updateCanvasPixelRect(image()->bounds());
} // KisToolSelectMagnetic::beginPrimaryAction

void KisToolSelectMagnetic::checkIfAnchorIsSelected(QPointF temp)
{
    Q_FOREACH (const QPoint pt, m_anchorPoints) {
        qreal zoomLevel = canvas()->viewConverter()->zoom();
        int sides       = (int) std::ceil(10.0 / zoomLevel);
        QRect r         = QRect(QPoint(0, 0), QSize(sides, sides));
        r.moveCenter(pt);
        if (r.contains(temp.toPoint())) {
            m_selected       = true;
            m_selectedAnchor = m_anchorPoints.lastIndexOf(pt);
            return;
        }
    }
}

/*

~~TODO ALERT~~
The double click adds a bit more functionality to the tool but also the reason
of multiple problems, so disabling it for now, if someone can find some alternate
ways for mimicking what the double clicks intended to do, please drop a patch

void KisToolSelectMagnetic::beginPrimaryDoubleClickAction(KoPointerEvent *event)
{
    QPointF temp = convertToPixelCoord(event);

    if (!image()->bounds().contains(temp.toPoint())) {
        return;
    }

    checkIfAnchorIsSelected(temp);

    if (m_selected) {
        deleteSelectedAnchor();
        return;
    }

    if (m_complete) {
        int pointA = 0, pointB = 1;
        double dist = std::numeric_limits<double>::max();
        int total   = m_anchorPoints.count();
        for (int i = 0; i < total; i++) {
            double distToCompare = kisDistance(m_anchorPoints[i], temp) +
                                   kisDistance(temp, m_anchorPoints[(i + 1) % total]);
            if (distToCompare < dist) {
                pointA = i;
                pointB = (i + 1) % total;
                dist   = distToCompare;
            }
        }

        vQPointF path1 = computeEdgeWrapper(m_anchorPoints[pointA], temp.toPoint());
        vQPointF path2 = computeEdgeWrapper(temp.toPoint(), m_anchorPoints[pointB]);

        m_pointCollection[pointA] = path1;
        m_pointCollection.insert(pointB, path2);
        m_anchorPoints.insert(pointB, temp.toPoint());

        reEvaluatePoints();
    }
} // KisToolSelectMagnetic::beginPrimaryDoubleClickAction
*/

// drag while primary mouse button is pressed
void KisToolSelectMagnetic::continuePrimaryAction(KoPointerEvent *event)
{
    if (isMovingSelection()) {
        KisToolSelectBase::continuePrimaryAction(event);
        return;
    }

    if (m_selected) {
        m_anchorPoints[m_selectedAnchor] = convertToPixelCoord(event).toPoint();
    } else if (!m_complete) {
        m_lastCursorPos = convertToPixelCoord(event);
        if(kisDistance(m_lastCursorPos, m_cursorOnPress) >= m_anchorGap)
            m_mouseHoverCompressor.start();
    }
}

void KisToolSelectMagnetic::slotCalculateEdge()
{
    QPoint current    = m_lastCursorPos.toPoint();
    if (!image()->bounds().contains(current))
        return;

    if(kisDistance(m_lastAnchor, current) < m_anchorGap)
        return;

    vQPointF pointSet = computeEdgeWrapper(m_lastAnchor, current);
    calculateCheckPoints(pointSet);
}

// release primary mouse button
void KisToolSelectMagnetic::endPrimaryAction(KoPointerEvent *event)
{
    if (isMovingSelection()) {
        KisToolSelectBase::endPrimaryAction(event);
        return;
    }

    if (m_selected && convertToPixelCoord(event) != m_cursorOnPress) {
        if (!image()->bounds().contains(m_anchorPoints[m_selectedAnchor])) {
            deleteSelectedAnchor();
        } else {
            updateSelectedAnchor();
        }
    } else if (m_selected) {
        QPointF temp(convertToPixelCoord(event));

        if (!image()->bounds().contains(temp.toPoint())) {
            return;
        }

        if (m_snapBound.contains(temp) && m_anchorPoints.count() > 1) {
            if(m_complete){
                finishSelectionAction();
                return;
            }
            vQPointF edge = computeEdgeWrapper(m_anchorPoints.last(), temp.toPoint());
            m_points.append(edge);
            m_pointCollection.push_back(edge);
            m_complete = true;
        }
    }
    if (m_mouseHoverCompressor.isActive()) {
        m_mouseHoverCompressor.stop();
        slotCalculateEdge();
    }
    m_selected = false;
} // KisToolSelectMagnetic::endPrimaryAction

void KisToolSelectMagnetic::deleteSelectedAnchor()
{
    if (m_anchorPoints.isEmpty())
        return;

    if (m_anchorPoints.size() <= 1) {
        resetVariables();

    } else if (m_selectedAnchor == 0) { // if it is the initial anchor
        m_anchorPoints.pop_front();
        m_pointCollection.pop_front();

        if (m_complete) {
            m_pointCollection[m_pointCollection.size() - 1] = computeEdgeWrapper(m_anchorPoints.last(), m_anchorPoints.first());
        }

    } else if (m_selectedAnchor == m_anchorPoints.count() - 1) { // if it is the last anchor
        m_anchorPoints.pop_back();
        m_pointCollection.pop_back();

        if (m_complete) {
            m_pointCollection[m_pointCollection.size() - 1] = computeEdgeWrapper(m_anchorPoints.last(), m_anchorPoints.first());
        }

    } else { // it is in the middle
        m_anchorPoints.remove(m_selectedAnchor);
        m_pointCollection.remove(m_selectedAnchor);
        m_pointCollection[m_selectedAnchor - 1] =
            computeEdgeWrapper(m_anchorPoints[m_selectedAnchor - 1],
                               m_anchorPoints[m_selectedAnchor]);
    }

    if (m_complete && m_anchorPoints.size() < 3) {
        m_complete = false;
        m_pointCollection.pop_back();
    }

    reEvaluatePoints();

} // KisToolSelectMagnetic::deleteSelectedAnchor

void KisToolSelectMagnetic::updateSelectedAnchor()
{
    //the only anchor
    if (m_anchorPoints.count() <= 1) {
        return;
    }

    if (m_selectedAnchor == 0) {
        m_pointCollection[m_selectedAnchor] = computeEdgeWrapper(m_anchorPoints[0], m_anchorPoints[1]);
        if (m_complete) {
            m_pointCollection[m_pointCollection.count() - 1] =
                computeEdgeWrapper(m_anchorPoints.last(),
                                   m_anchorPoints.first());
        }
    } else if (m_selectedAnchor == m_anchorPoints.count() - 1) {
        m_pointCollection[m_selectedAnchor - 1] =
            computeEdgeWrapper(m_anchorPoints[m_anchorPoints.count() - 2],
                               m_anchorPoints[m_anchorPoints.count() - 1]);
        if (m_complete) {
            m_pointCollection[m_selectedAnchor] =
                computeEdgeWrapper(m_anchorPoints.last(), m_anchorPoints.first());
        }
    } else {
        m_pointCollection[m_selectedAnchor - 1] =
            computeEdgeWrapper(m_anchorPoints[m_selectedAnchor - 1],
                               m_anchorPoints[m_selectedAnchor]);

        m_pointCollection[m_selectedAnchor] =
            computeEdgeWrapper(m_anchorPoints[m_selectedAnchor],
                               m_anchorPoints[m_selectedAnchor + 1]);
    }

    reEvaluatePoints();
}

int KisToolSelectMagnetic::updateInitialAnchorBounds(QPoint pt)
{
    qreal zoomLevel = canvas()->viewConverter()->zoom();
    int sides       = (int) std::ceil(10.0 / zoomLevel);
    m_snapBound = QRectF(QPoint(0, 0), QSize(sides, sides));
    m_snapBound.moveCenter(pt);
    return sides;
}

void KisToolSelectMagnetic::reEvaluatePoints()
{
    m_points.clear();
    Q_FOREACH (const vQPointF vec, m_pointCollection) {
        m_points.append(vec);
    }

    updatePaintPath();
}

void KisToolSelectMagnetic::finishSelectionAction()
{
    KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2 *>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->updateCanvas();
    setMode(KisTool::HOVER_MODE);
    m_complete = false;
    m_finished = true;

    // just for testing out
    // m_worker.saveTheImage(m_points);

    QRectF boundingViewRect =
        pixelToView(KisAlgebra2D::accumulateBounds(m_points));

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Magnetic Selection"));

    if (m_points.count() > 2 &&
        !helper.tryDeselectCurrentSelection(boundingViewRect, selectionAction()))
    {
        KisCursorOverrideLock cursorLock(KisCursor::waitCursor());

        const SelectionMode mode =
            helper.tryOverrideSelectionMode(kisCanvas->viewManager()->selection(),
                                            selectionMode(),
                                            selectionAction());
        if (mode == PIXEL_SELECTION) {
            KisProcessingApplicator applicator(
                currentImage(),
                currentNode(),
                KisProcessingApplicator::NONE,
                KisImageSignalVector(),
                kundo2_i18n("Magnetic Selection"));

            KisPixelSelectionSP tmpSel =
                new KisPixelSelection(new KisDefaultBounds(currentImage()));

            const bool antiAlias = antiAliasSelection();
            const int grow = growSelection();
            const int feather = featherSelection();

            QPainterPath path;
            path.addPolygon(m_points);
            path.closeSubpath();

            KUndo2Command *cmd = new KisCommandUtils::LambdaCommand(
                [tmpSel, antiAlias, grow, feather, path]() mutable
                -> KUndo2Command * {
                    KisPainter painter(tmpSel);
                    painter.setPaintColor(
                        KoColor(Qt::black, tmpSel->colorSpace()));
                    // Since the feathering already smooths the selection, the
                    // antiAlias is not applied if we must feather
                    painter.setAntiAliasPolygonFill(antiAlias && feather == 0);
                    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
                    painter.setStrokeStyle(KisPainter::StrokeStyleNone);

                    painter.paintPainterPath(path);

                    if (grow > 0) {
                        KisGrowSelectionFilter biggy(grow, grow);
                        biggy.process(tmpSel,
                                      tmpSel->selectedRect().adjusted(-grow,
                                                                      -grow,
                                                                      grow,
                                                                      grow));
                    } else if (grow < 0) {
                        KisShrinkSelectionFilter tiny(-grow, -grow, false);
                        tiny.process(tmpSel, tmpSel->selectedRect());
                    }
                    if (feather > 0) {
                        KisFeatherSelectionFilter feathery(feather);
                        feathery.process(
                            tmpSel,
                            tmpSel->selectedRect().adjusted(-feather,
                                                            -feather,
                                                            feather,
                                                            feather));
                    }

                    if (grow == 0 && feather == 0) {
                        tmpSel->setOutlineCache(path);
                    } else {
                        tmpSel->invalidateOutlineCache();
                    }

                    return 0;
                });

            applicator.applyCommand(cmd, KisStrokeJobData::SEQUENTIAL);
            helper.selectPixelSelection(applicator, tmpSel, selectionAction());
            applicator.end();

        } else {
            KoPathShape *path = new KoPathShape();
            path->setShapeId(KoPathShapeId);

            QTransform resolutionMatrix;
            resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
            path->moveTo(resolutionMatrix.map(m_points[0]));
            for (int i = 1; i < m_points.count(); i++)
                path->lineTo(resolutionMatrix.map(m_points[i]));
            path->close();
            path->normalize();
            helper.addSelectionShape(path, selectionAction());
        }
    }

    resetVariables();

    endSelectInteraction();
} // KisToolSelectMagnetic::finishSelectionAction

void KisToolSelectMagnetic::resetVariables()
{
    m_points.clear();
    m_anchorPoints.clear();
    m_pointCollection.clear();
    m_paintPath = QPainterPath();
    m_complete = false;
}

void KisToolSelectMagnetic::updatePaintPath()
{
    m_paintPath = QPainterPath();
    if (m_points.size() > 0) {
        m_paintPath.moveTo(pixelToView(m_points[0]));
    }
    for (int i = 1; i < m_points.count(); i++) {
        m_paintPath.lineTo(pixelToView(m_points[i]));
    }

    updateFeedback();

    if (m_continuedMode && mode() != PAINT_MODE) {
        updateContinuedMode();
    }

    updateCanvasPixelRect(image()->bounds());
}

void KisToolSelectMagnetic::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    updatePaintPath();
    if ((mode() == KisTool::PAINT_MODE || m_continuedMode) &&
        !m_anchorPoints.isEmpty())
    {
        QPainterPath outline = m_paintPath;
        if (m_continuedMode && mode() != KisTool::PAINT_MODE) {
            outline.lineTo(pixelToView(m_lastCursorPos));
        }
        paintToolOutline(&gc, outline);
        drawAnchors(gc);
    }
}

void KisToolSelectMagnetic::drawAnchors(QPainter &gc)
{
    int sides = updateInitialAnchorBounds(m_anchorPoints.first());
    Q_FOREACH (const QPoint pt, m_anchorPoints) {
        KisHandlePainterHelper helper(&gc, handleRadius(), decorationThickness());
        QRect r(QPoint(0, 0), QSize(sides, sides));
        r.moveCenter(pt);
        if (r.contains(m_lastCursorPos.toPoint())) {
            helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
        } else {
            helper.setHandleStyle(KisHandleStyle::primarySelection());
        }
        helper.drawHandleRect(pixelToView(pt), 4, QPoint(0, 0));
    }
}

void KisToolSelectMagnetic::updateFeedback()
{
    if (m_points.count() > 1) {
        qint32 lastPointIndex = m_points.count() - 1;

        QRectF updateRect = QRectF(m_points[lastPointIndex - 1], m_points[lastPointIndex]).normalized();
        updateRect = kisGrowRect(updateRect, FEEDBACK_LINE_WIDTH);

        updateCanvasPixelRect(updateRect);
    }
}

void KisToolSelectMagnetic::updateContinuedMode()
{
    if (!m_points.isEmpty()) {
        qint32 lastPointIndex = m_points.count() - 1;

        QRectF updateRect = QRectF(m_points[lastPointIndex - 1], m_lastCursorPos).normalized();
        updateRect = kisGrowRect(updateRect, FEEDBACK_LINE_WIDTH);

        updateCanvasPixelRect(updateRect);
    }
}

void KisToolSelectMagnetic::activate(const QSet<KoShape *> &shapes)
{
    m_worker.reset(new KisMagneticWorker(image()->projection()));
    m_configGroup = KSharedConfig::openConfig()->group(toolId());
    connect(action("undo_polygon_selection"), SIGNAL(triggered()), SLOT(undoPoints()), Qt::UniqueConnection);
    connect(&m_mouseHoverCompressor, SIGNAL(timeout()), this, SLOT(slotCalculateEdge()));
    KisToolSelect::activate(shapes);
}

void KisToolSelectMagnetic::deactivate()
{
    KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2 *>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->updateCanvas();
    resetVariables();
    m_continuedMode = false;
    disconnect(action("undo_polygon_selection"), nullptr, this, nullptr);

    KisTool::deactivate();
}

void KisToolSelectMagnetic::undoPoints()
{
    if (m_complete) return;

    if(m_anchorPoints.count() <= 1){
        resetVariables();
        return;
    }

    m_anchorPoints.pop_back();
    m_pointCollection.pop_back();
    reEvaluatePoints();
}

void KisToolSelectMagnetic::requestStrokeEnd()
{
    if (m_finished || m_anchorPoints.count() < 2) return;

    setButtonsEnabled(false);
    finishSelectionAction();
    m_finished = false;
}

void KisToolSelectMagnetic::requestStrokeCancellation()
{
    m_complete = false;
    m_finished = false;
    setButtonsEnabled(false);
    resetVariables();
}

QWidget * KisToolSelectMagnetic::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    KisSelectionOptions *selectionWidget = selectionOptionWidget();

    // Create widgets
    KisDoubleSliderSpinBox *sliderRadius = new KisDoubleSliderSpinBox;
    sliderRadius->setObjectName("radius");
    sliderRadius->setRange(2.5, 100.0, 2);
    sliderRadius->setSingleStep(0.5);
    sliderRadius->setPrefix(
        i18nc("Filter radius in Magnetic Select Tool settings",
              "Filter Radius: "));

    KisSliderSpinBox *sliderThreshold = new KisSliderSpinBox;
    sliderThreshold->setObjectName("threshold");
    sliderThreshold->setRange(1, 255);
    sliderThreshold->setSingleStep(10);
    sliderThreshold->setPrefix(
        i18nc("Threshold in Magnetic Selection's Tool options", "Threshold: "));

    KisSliderSpinBox *sliderSearchRadius = new KisSliderSpinBox;
    sliderSearchRadius->setObjectName("frequency");
    sliderSearchRadius->setRange(20, 200);
    sliderSearchRadius->setSingleStep(10);
    sliderSearchRadius->setPrefix(
        i18nc("Search Radius in Magnetic Selection's Tool options",
              "Search Radius: "));
    sliderSearchRadius->setSuffix(" px");

    KisSliderSpinBox *sliderAnchorGap = new KisSliderSpinBox;
    sliderAnchorGap->setObjectName("anchorgap");
    sliderAnchorGap->setRange(20, 200);
    sliderAnchorGap->setSingleStep(10);
    sliderAnchorGap->setPrefix(
        i18nc("Anchor Gap in Magnetic Selection's Tool options",
              "Anchor Gap: "));
    sliderAnchorGap->setSuffix(" px");

    QPushButton *buttonCompleteSelection =
        new QPushButton(i18nc("Complete the selection", "Complete"),
                        selectionWidget);
    buttonCompleteSelection->setEnabled(false);

    QPushButton *buttonDiscardSelection =
        new QPushButton(i18nc("Discard the selection", "Discard"),
                        selectionWidget);
    buttonDiscardSelection->setEnabled(false);

    // Set the tooltips
    sliderRadius->setToolTip(i18nc("@info:tooltip",
                                   "Radius of the filter for the detecting "
                                   "edges, might take some time to calculate"));
    sliderThreshold->setToolTip(
        i18nc("@info:tooltip",
              "Threshold for determining the minimum intensity of the edges"));
    sliderSearchRadius->setToolTip(
        i18nc("@info:tooltip", "Extra area to be searched"));
    sliderAnchorGap->setToolTip(
        i18nc("@info:tooltip", "Gap between 2 anchors in interactive mode"));
    buttonCompleteSelection->setToolTip(
        i18nc("@info:tooltip", "Complete Selection"));
    buttonDiscardSelection->setToolTip(
        i18nc("@info:tooltip", "Discard Selection"));

    // Construct the option widget
    KisOptionCollectionWidgetWithHeader *sectionPathOptions =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'path options' section label in magnetic selection's "
                  "tool options",
                  "Path options"));
    sectionPathOptions->appendWidget("sliderRadius", sliderRadius);
    sectionPathOptions->appendWidget("sliderThreshold", sliderThreshold);
    sectionPathOptions->appendWidget("sliderSearchRadius", sliderSearchRadius);
    sectionPathOptions->appendWidget("sliderAnchorGap", sliderAnchorGap);
    sectionPathOptions->appendWidget("buttonCompleteSelection",
                                     buttonCompleteSelection);
    sectionPathOptions->appendWidget("buttonDiscardSelection",
                                     buttonDiscardSelection);
    selectionWidget->appendWidget("sectionPathOptions", sectionPathOptions);

    // Load configuration settings into tool options
    m_filterRadius = m_configGroup.readEntry("filterradius", 3.0);
    m_threshold = m_configGroup.readEntry("threshold", 100);
    m_searchRadius = m_configGroup.readEntry("searchradius", 30);
    m_anchorGap = m_configGroup.readEntry("anchorgap", 20);

    sliderRadius->setValue(m_filterRadius);
    sliderThreshold->setValue(m_threshold);
    sliderSearchRadius->setValue(m_searchRadius);
    sliderAnchorGap->setValue(m_anchorGap);

    // Make connections
    connect(sliderRadius,
            SIGNAL(valueChanged(qreal)),
            this,
            SLOT(slotSetFilterRadius(qreal)));
    connect(sliderThreshold,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotSetThreshold(int)));
    connect(sliderSearchRadius,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotSetSearchRadius(int)));
    connect(sliderAnchorGap,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotSetAnchorGap(int)));
    connect(buttonCompleteSelection,
            SIGNAL(clicked()),
            this,
            SLOT(requestStrokeEnd()));
    connect(this,
            SIGNAL(setButtonsEnabled(bool)),
            buttonCompleteSelection,
            SLOT(setEnabled(bool)));
    connect(buttonDiscardSelection,
            SIGNAL(clicked()),
            this,
            SLOT(requestStrokeCancellation()));
    connect(this,
            SIGNAL(setButtonsEnabled(bool)),
            buttonDiscardSelection,
            SLOT(setEnabled(bool)));

    return selectionWidget;

} // KisToolSelectMagnetic::createOptionWidget

void KisToolSelectMagnetic::slotSetFilterRadius(qreal r)
{
    m_filterRadius = r;
    m_configGroup.writeEntry("filterradius", r);
}

void KisToolSelectMagnetic::slotSetThreshold(int t)
{
    m_threshold = t;
    m_configGroup.writeEntry("threshold", t);
}

void KisToolSelectMagnetic::slotSetSearchRadius(int r)
{
    m_searchRadius = r;
    m_configGroup.writeEntry("searchradius", r);
}

void KisToolSelectMagnetic::slotSetAnchorGap(int g)
{
    m_anchorGap = g;
    m_configGroup.writeEntry("anchorgap", g);
}

void KisToolSelectMagnetic::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_magnetic_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_magnetic_selection_cursor_sub.png", 6, 6));
    } else if (selectionAction() == SELECTION_INTERSECT) {
        useCursor(KisCursor::load("tool_magnetic_selection_cursor_inter.png", 6, 6));
    } else if (selectionAction() == SELECTION_SYMMETRICDIFFERENCE) {
        useCursor(KisCursor::load("tool_magnetic_selection_cursor_symdiff.png", 6, 6));
    } else {
        KisToolSelect::resetCursorStyle();
    }
}
