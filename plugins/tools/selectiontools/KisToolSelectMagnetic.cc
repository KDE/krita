/*
 *  SPDX-FileCopyrightText: 2019 Kuntal Majumder <hellozee@disroot.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisToolSelectMagnetic.h"

#include <QApplication>
#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

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

#include "kis_painter.h"
#include <brushengine/kis_paintop_registry.h>
#include "canvas/kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"

#include "kis_algebra_2d.h"

#include "KisHandlePainterHelper.h"

#include <kis_slider_spin_box.h>

#define FEEDBACK_LINE_WIDTH 2

KisToolSelectMagnetic::KisToolSelectMagnetic(KoCanvasBase *canvas)
    : KisToolSelect(canvas,
                    KisCursor::load("tool_magnetic_selection_cursor.png", 5, 5),
                    i18n("Magnetic Selection")),
    m_continuedMode(false), m_complete(false), m_selected(false), m_finished(false),
    m_worker(image()->projection()), m_threshold(70), m_searchRadius(30), m_anchorGap(30),
    m_filterRadius(3.0), m_mouseHoverCompressor(100, KisSignalCompressor::FIRST_ACTIVE)

{ }

void KisToolSelectMagnetic::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        m_continuedMode = true;
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
            if (m_worker.intensity(points.at(i).toPoint()) >= m_threshold) {
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
                if (m_worker.intensity(points.at(i).toPoint()) >= m_threshold) {
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
    if (event->key() == Qt::Key_Control ||
        !(event->modifiers() & Qt::ControlModifier))
    {
        m_continuedMode = false;
        if (mode() != PAINT_MODE && !m_points.isEmpty()) {
            finishSelectionAction();
        }
    }

    KisToolSelect::keyReleaseEvent(event);
}

vQPointF KisToolSelectMagnetic::computeEdgeWrapper(QPoint a, QPoint b)
{
    return m_worker.computeEdge(m_searchRadius, a, b, m_filterRadius);
}

// the cursor is still tracked even when no mousebutton is pressed
void KisToolSelectMagnetic::mouseMoveEvent(KoPointerEvent *event)
{
    m_lastCursorPos = convertToPixelCoord(event);
    KisToolSelect::mouseMoveEvent(event);
    updatePaintPath();
} // KisToolSelectMagnetic::mouseMoveEvent

// press primary mouse button
void KisToolSelectMagnetic::beginPrimaryAction(KoPointerEvent *event)
{
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
    if (m_selected) {
        m_anchorPoints[m_selectedAnchor] = convertToPixelCoord(event).toPoint();
    } else if (!m_complete) {
        m_lastCursorPos = convertToPixelCoord(event);
        if(kisDistance(m_lastCursorPos, m_cursorOnPress) >= m_anchorGap)
            m_mouseHoverCompressor.start();
    }
    KisToolSelectBase::continuePrimaryAction(event);
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
    KisToolSelectBase::endPrimaryAction(event);
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
        QApplication::setOverrideCursor(KisCursor::waitCursor());

        const SelectionMode mode =
            helper.tryOverrideSelectionMode(kisCanvas->viewManager()->selection(),
                                            selectionMode(),
                                            selectionAction());
        if (mode == PIXEL_SELECTION) {
            KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

            KisPainter painter(tmpSel);
            painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
            painter.setAntiAliasPolygonFill(antiAliasSelection());
            painter.setFillStyle(KisPainter::FillStyleForegroundColor);
            painter.setStrokeStyle(KisPainter::StrokeStyleNone);

            painter.paintPolygon(m_points);

            QPainterPath cache;
            cache.addPolygon(m_points);
            cache.closeSubpath();
            tmpSel->setOutlineCache(cache);

            helper.selectPixelSelection(tmpSel, selectionAction());
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
        QApplication::restoreOverrideCursor();
    }

    resetVariables();
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
        KisHandlePainterHelper helper(&gc, handleRadius());
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
    m_worker      = KisMagneticWorker(image()->projection());
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
    QHBoxLayout *f1 = new QHBoxLayout();
    QLabel *filterRadiusLabel = new QLabel(i18n("Filter Radius: "), selectionWidget);
    f1->addWidget(filterRadiusLabel);

    KisDoubleSliderSpinBox *filterRadiusInput = new KisDoubleSliderSpinBox(selectionWidget);
    filterRadiusInput->setObjectName("radius");
    filterRadiusInput->setRange(2.5, 100.0, 2);
    filterRadiusInput->setSingleStep(0.5);
    filterRadiusInput->setToolTip(
        i18nc("@info:tooltip", "Radius of the filter for the detecting edges, might take some time to calculate"));
    f1->addWidget(filterRadiusInput);
    connect(filterRadiusInput, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetFilterRadius(qreal)));

    QHBoxLayout *f2        = new QHBoxLayout();
    QLabel *thresholdLabel = new QLabel(i18nc("Threshold label in Magnetic Selection's Tool options", "Threshold: "), selectionWidget);
    f2->addWidget(thresholdLabel);

    KisSliderSpinBox *thresholdInput = new KisSliderSpinBox(selectionWidget);
    thresholdInput->setObjectName("threshold");
    thresholdInput->setRange(1, 255);
    thresholdInput->setSingleStep(10);
    thresholdInput->setToolTip(i18nc("@info:tooltip", "Threshold for determining the minimum intensity of the edges"));
    f2->addWidget(thresholdInput);
    connect(thresholdInput, SIGNAL(valueChanged(int)), this, SLOT(slotSetThreshold(int)));

    QHBoxLayout *f3 = new QHBoxLayout();
    QLabel *searchRadiusLabel = new QLabel(i18n("Search Radius: "), selectionWidget);
    f3->addWidget(searchRadiusLabel);

    KisSliderSpinBox *searchRadiusInput = new KisSliderSpinBox(selectionWidget);
    searchRadiusInput->setObjectName("frequency");
    searchRadiusInput->setRange(20, 200);
    searchRadiusInput->setSingleStep(10);
    searchRadiusInput->setToolTip(i18nc("@info:tooltip", "Extra area to be searched"));
    searchRadiusInput->setSuffix(" px");
    f3->addWidget(searchRadiusInput);
    connect(searchRadiusInput, SIGNAL(valueChanged(int)), this, SLOT(slotSetSearchRadius(int)));

    QHBoxLayout *f4        = new QHBoxLayout();
    QLabel *anchorGapLabel = new QLabel(i18n("Anchor Gap: "), selectionWidget);
    f4->addWidget(anchorGapLabel);

    KisSliderSpinBox *anchorGapInput = new KisSliderSpinBox(selectionWidget);
    anchorGapInput->setObjectName("anchorgap");
    anchorGapInput->setRange(20, 200);
    anchorGapInput->setSingleStep(10);
    anchorGapInput->setToolTip(i18nc("@info:tooltip", "Gap between 2 anchors in interactive mode"));
    anchorGapInput->setSuffix(" px");
    f4->addWidget(anchorGapInput);

    connect(anchorGapInput, SIGNAL(valueChanged(int)), this, SLOT(slotSetAnchorGap(int)));

    QHBoxLayout *f5 = new QHBoxLayout();
    QPushButton* completeSelection = new QPushButton(i18nc("Complete the selection", "Complete"), selectionWidget);
    QPushButton* discardSelection = new QPushButton(i18nc("Discard the selection", "Discard"), selectionWidget);

    f5->addWidget(completeSelection);
    f5->addWidget(discardSelection);

    completeSelection->setEnabled(false);
    completeSelection->setToolTip(i18nc("@info:tooltip", "Complete Selection"));
    connect(completeSelection, SIGNAL(clicked()), this, SLOT(requestStrokeEnd()));
    connect(this, SIGNAL(setButtonsEnabled(bool)), completeSelection, SLOT(setEnabled(bool)));

    discardSelection->setEnabled(false);
    discardSelection->setToolTip(i18nc("@info:tooltip", "Discard Selection"));
    connect(discardSelection, SIGNAL(clicked()), this, SLOT(requestStrokeCancellation()));
    connect(this, SIGNAL(setButtonsEnabled(bool)), discardSelection, SLOT(setEnabled(bool)));

    QVBoxLayout *l = dynamic_cast<QVBoxLayout *>(selectionWidget->layout());

    l->insertLayout(1, f1);
    l->insertLayout(2, f2);
    l->insertLayout(3, f3);
    l->insertLayout(4, f4);
    l->insertLayout(5, f5);

    filterRadiusInput->setValue(m_configGroup.readEntry("filterradius", 3.0));
    thresholdInput->setValue(m_configGroup.readEntry("threshold", 100));
    searchRadiusInput->setValue(m_configGroup.readEntry("searchradius", 30));
    anchorGapInput->setValue(m_configGroup.readEntry("anchorgap", 20));

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
    } else {
        KisToolSelect::resetCursorStyle();
    }
}
