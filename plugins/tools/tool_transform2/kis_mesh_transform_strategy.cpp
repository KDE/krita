/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_mesh_transform_strategy.h"
#include "tool_transform_args.h"

#include <QPointF>
#include <QPainter>
#include <QPainterPath>

#include "kis_painting_tweaks.h"
#include "kis_cursor.h"

#include "transform_transaction_properties.h"
#include "KisHandlePainterHelper.h"
#include "kis_transform_utils.h"
#include "kis_signal_compressor.h"


uint qHash(const QPoint &value) {
    return uint((0xffffffffffffffff - quint64(value.y())) ^ quint64(value.x()));
}

struct KisMeshTransformStrategy::Private
{
    Private(KisMeshTransformStrategy *_q,
            const KisCoordinatesConverter *_converter,
            ToolTransformArgs &_currentArgs,
            TransformTransactionProperties &_transaction)
        : q(_q),
          converter(_converter),
          currentArgs(_currentArgs),
          transaction(_transaction),
          recalculateSignalCompressor(40, KisSignalCompressor::FIRST_ACTIVE)
    {
    }

    KisMeshTransformStrategy * const q;

    enum Mode {
        OVER_POINT = 0,
        OVER_POINT_SYMMETRIC,
        OVER_NODE,
        OVER_NODE_WHOLE_LINE,
        OVER_SEGMENT,
        OVER_SEGMENT_SYMMETRIC,
        OVER_PATCH,
        OVER_PATCH_LOCKED,
        SPLIT_SEGMENT,
        MULTIPLE_POINT_SELECTION,
        MOVE_MODE,
        ROTATE_MODE,
        SCALE_MODE,
        NOTHING
    };
    Mode mode = NOTHING;

    const KisCoordinatesConverter *converter;
    ToolTransformArgs &currentArgs;
    TransformTransactionProperties &transaction;

    QSet<KisBezierTransformMesh::NodeIndex> selectedNodes;
    boost::optional<KisBezierTransformMesh::SegmentIndex> hoveredSegment;
    boost::optional<KisBezierTransformMesh::ControlPointIndex> hoveredControl;
    boost::optional<KisBezierTransformMesh::PatchIndex> hoveredPatch;
    qreal localSegmentPosition = 0.0;
    QPointF localPatchPosition;

    QPointF mouseClickPos;

    QPointF initialRotationCenter;
    qreal initialSelectionMaxDimension = 0.0;
    KisBezierTransformMesh initialMeshState;

    bool pointWasDragged = false;
    QPointF lastMousePos;
    QSize lastMeshSize;

    KisSignalCompressor recalculateSignalCompressor;

    QTransform paintingTransform;
    QPointF paintingOffset;
    QImage transformedImage;

    void recalculateTransformations();
    QTransform imageToThumb(bool useFlakeOptimization);
};


KisMeshTransformStrategy::KisMeshTransformStrategy(const KisCoordinatesConverter *converter,
                                                   ToolTransformArgs &currentArgs,
                                                   TransformTransactionProperties &transaction)
    : KisSimplifiedActionPolicyStrategy(converter),
      m_d(new Private(this, converter, currentArgs, transaction))
{

    connect(&m_d->recalculateSignalCompressor, SIGNAL(timeout()),
            SLOT(recalculateTransformations()));

    m_d->selectedNodes << KisBezierTransformMesh::NodeIndex(1, 1);
    m_d->hoveredSegment = KisBezierTransformMesh::SegmentIndex(KisBezierTransformMesh::NodeIndex(0,0), 1);
    m_d->hoveredControl = KisBezierTransformMesh::ControlPointIndex(KisBezierTransformMesh::NodeIndex(1, 0), KisBezierTransformMesh::ControlType::Node);
}

KisMeshTransformStrategy::~KisMeshTransformStrategy()
{
}

void KisMeshTransformStrategy::setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive, bool shiftModifierActive, bool altModifierActive)
{
    const qreal grabRadius = KisTransformUtils::effectiveHandleGrabRadius(m_d->converter);

    boost::optional<KisBezierTransformMesh::SegmentIndex> hoveredSegment;
    boost::optional<KisBezierTransformMesh::ControlPointIndex> hoveredControl;
    boost::optional<KisBezierTransformMesh::PatchIndex> hoveredPatch;
    Private::Mode mode = Private::NOTHING;
    QPointF localPatchPos;
    qreal localSegmentPos = 0.0;

    const bool symmetricalMode = shiftModifierActive ^ m_d->currentArgs.meshSymmetricalHandles();

    if (m_d->currentArgs.meshShowHandles()) {
        auto index = m_d->currentArgs.meshTransform()->hitTestControlPoint(mousePos, grabRadius);
        if (m_d->currentArgs.meshTransform()->isIndexValid(index)) {
            hoveredControl = index;
            mode = symmetricalMode ? Private::OVER_POINT_SYMMETRIC : Private::OVER_POINT;
        }
    }

    if (mode == Private::NOTHING) {
        auto index = m_d->currentArgs.meshTransform()->hitTestNode(mousePos, grabRadius);
        auto nodeIt = m_d->currentArgs.meshTransform()->find(index);

        if (nodeIt != m_d->currentArgs.meshTransform()->endControlPoints()) {
            hoveredControl = index;
            mode = shiftModifierActive && nodeIt.isBorderNode() && !nodeIt.isCornerNode() ?
                Private::OVER_NODE_WHOLE_LINE :
                Private::OVER_NODE;
        }
    }

    if (mode == Private::NOTHING) {
        auto index = m_d->currentArgs.meshTransform()->hitTestSegment(mousePos, grabRadius, &localSegmentPos);
        if (m_d->currentArgs.meshTransform()->isIndexValid(index)) {
            hoveredSegment = index;
            mode = symmetricalMode ? Private::OVER_SEGMENT_SYMMETRIC : Private::OVER_SEGMENT;
        }
    }

    if (mode == Private::NOTHING) {
        auto index = m_d->currentArgs.meshTransform()->hitTestPatch(mousePos, &localPatchPos);
        if (m_d->currentArgs.meshTransform()->isIndexValid(index)) {
            hoveredPatch = index;
            mode = !shiftModifierActive ? Private::OVER_PATCH : Private::OVER_PATCH_LOCKED;
        }
    }


    // verify that we have only one active selection at a time
    KIS_SAFE_ASSERT_RECOVER_RETURN(bool(hoveredControl) +
                                   bool(hoveredSegment) +
                                   bool(hoveredPatch)<= 1);


    KisBezierTransformMesh::control_point_iterator controlIt =
        m_d->currentArgs.meshTransform()->endControlPoints();

    if (hoveredControl) {
        controlIt = m_d->currentArgs.meshTransform()->find(*hoveredControl);
    }

    if (altModifierActive &&
        ((hoveredControl &&
          hoveredControl->isNode() &&
          controlIt.isBorderNode() &&
          !controlIt.isCornerNode()) ||
         hoveredSegment)) {

        mode = Private::SPLIT_SEGMENT;

    } else {
        if (hoveredControl || hoveredSegment) {
            if (perspectiveModifierActive) {
                mode = Private::MULTIPLE_POINT_SELECTION;
            } else if (hoveredControl &&
                       hoveredControl->isNode() &&
                       m_d->selectedNodes.size() > 1 &&
                       m_d->selectedNodes.contains(hoveredControl->nodeIndex)) {

                mode = Private::MOVE_MODE;
            }
        } else if (!hoveredPatch) {
            if (perspectiveModifierActive) {
                mode = Private::SCALE_MODE;
            } else if (shiftModifierActive) {
                mode = Private::MOVE_MODE;
            } else {
                mode = Private::ROTATE_MODE;
            }
        }
    }

    if (mode != m_d->mode ||
        hoveredControl != m_d->hoveredControl ||
        hoveredSegment != m_d->hoveredSegment ||
        hoveredPatch != m_d->hoveredPatch) {

        m_d->hoveredControl = hoveredControl;
        m_d->hoveredSegment = hoveredSegment;
        m_d->hoveredPatch = hoveredPatch;

        m_d->mode = mode;
        Q_EMIT requestCanvasUpdate();
    }

    m_d->localPatchPosition = localPatchPos;
    m_d->localSegmentPosition = localSegmentPos;

    verifyExpectedMeshSize();
}

void KisMeshTransformStrategy::verifyExpectedMeshSize()
{
    bool shouldUpdate = false;

    const QSize currentMeshSize = m_d->currentArgs.meshTransform()->size();
    if (currentMeshSize != m_d->lastMeshSize) {
        m_d->selectedNodes.clear();
        shouldUpdate = true;
    }
    m_d->lastMeshSize = currentMeshSize;

    if (shouldUpdate) {
        emit requestCanvasUpdate();
    }
}

void KisMeshTransformStrategy::paint(QPainter &gc)
{
    gc.save();

    gc.setOpacity(m_d->transaction.basePreviewOpacity());
    gc.setTransform(m_d->paintingTransform, true);
    gc.drawImage(m_d->paintingOffset, m_d->transformedImage);

    gc.restore();

    gc.save();
    gc.setTransform(KisTransformUtils::imageToFlakeTransform(m_d->converter), true);

    KisHandlePainterHelper handlePainter(&gc, 0.5 * KisTransformUtils::handleRadius);

    for (auto it = m_d->currentArgs.meshTransform()->beginSegments();
         it != m_d->currentArgs.meshTransform()->endSegments();
         ++it) {

        if (m_d->hoveredSegment && it.segmentIndex() == *m_d->hoveredSegment) {
            handlePainter.setHandleStyle(KisHandleStyle::highlightedPrimaryHandlesWithSolidOutline());
        } else {
            handlePainter.setHandleStyle(KisHandleStyle::primarySelection());
        }

        QPainterPath path;
        path.moveTo(it.p0());
        path.cubicTo(it.p1(), it.p2(), it.p3());

        handlePainter.drawPath(path);
    }

    for (auto it = m_d->currentArgs.meshTransform()->beginControlPoints();
         it != m_d->currentArgs.meshTransform()->endControlPoints();
         ++it) {

        if (!m_d->currentArgs.meshShowHandles() && !it.isNode()) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->hoveredControl || *m_d->hoveredControl != it.controlIndex());

            continue;
        }


        if (m_d->hoveredControl && *m_d->hoveredControl == it.controlIndex()) {

            handlePainter.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());

        } else if (it.type() == KisBezierTransformMesh::ControlType::Node &&
                   m_d->selectedNodes.contains(it.nodeIndex())) {

            handlePainter.setHandleStyle(KisHandleStyle::selectedPrimaryHandles());

        } else {
            handlePainter.setHandleStyle(KisHandleStyle::primarySelection());
        }

        if (it.type() == KisBezierTransformMesh::ControlType::Node) {
            handlePainter.drawHandleCircle(*it);
        } else {
            handlePainter.drawConnectionLine(it.node().node, *it);
            handlePainter.drawHandleSmallCircle(*it);
        }
    }

    gc.restore();
}

QCursor KisMeshTransformStrategy::getCurrentCursor() const
{
    QCursor cursor;

    switch (m_d->mode) {
    case Private::OVER_NODE:
    case Private::OVER_POINT:
    case Private::OVER_SEGMENT:
        cursor = KisCursor::meshCursorFree();
        break;
    case Private::OVER_NODE_WHOLE_LINE:
    case Private::OVER_POINT_SYMMETRIC:
    case Private::OVER_SEGMENT_SYMMETRIC:
    case Private::OVER_PATCH:
    case Private::OVER_PATCH_LOCKED:
        cursor = KisCursor::meshCursorLocked();
        break;
    case Private::SPLIT_SEGMENT: {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->hoveredSegment || m_d->hoveredControl,
                                             KisCursor::arrowCursor());

        if (m_d->hoveredControl) {
            auto it = m_d->currentArgs.meshTransform()->find(*m_d->hoveredControl);
            cursor = it.isTopBorder() || it.isBottomBorder() ?
                KisCursor::splitHCursor() : KisCursor::splitVCursor();

        } else if (m_d->hoveredSegment) {
            auto it = m_d->currentArgs.meshTransform()->find(*m_d->hoveredSegment);

            const QRectF segmentRect(it.p0(), it.p3());
            cursor = segmentRect.width() > segmentRect.height() ?
                KisCursor::splitHCursor() : KisCursor::splitVCursor();
        }

        break;
    }
    case Private::MULTIPLE_POINT_SELECTION:
        cursor = KisCursor::crossCursor();
        break;
    case Private::MOVE_MODE:
        cursor = KisCursor::moveCursor();
        break;
    case Private::ROTATE_MODE:
        cursor = KisCursor::rotateCursor();
        break;
    case Private::SCALE_MODE:
        cursor = KisCursor::sizeVerCursor();
        break;
    case Private::NOTHING:
        cursor = KisCursor::arrowCursor();
        break;
    }

    return cursor;
}

void KisMeshTransformStrategy::externalConfigChanged()
{
    verifyExpectedMeshSize();
    m_d->recalculateTransformations();
}

bool KisMeshTransformStrategy::splitHoveredSegment(const QPointF &pt)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->hoveredSegment || m_d->hoveredControl, false);

    if (m_d->hoveredControl) {
        auto it = m_d->currentArgs.meshTransform()->find(*m_d->hoveredControl);

        KisBezierTransformMesh::segment_iterator resultSegment = m_d->currentArgs.meshTransform()->endSegments();
        qreal resultParam = 0;
        qreal resultDistance = std::numeric_limits<qreal>::max();
        KisBezierTransformMesh::NodeIndex resultRemovedNodeIndex;

        auto estimateSegment =
            [&resultParam,
             &resultSegment,
             &resultDistance,
             &resultRemovedNodeIndex] (const KisBezierTransformMesh::segment_iterator &segment,
                               const QPoint &removedNodeOffset,
                               const QPointF &pt,
                               KisBezierTransformMesh &mesh)
        {
            if (segment != mesh.endSegments()) {

                qreal distance = 0.0;
                qreal param = KisBezierUtils::nearestPoint({segment.p0(), segment.p1(), segment.p2(), segment.p3()}, pt, &distance);

                if (distance < resultDistance) {
                    resultDistance = distance;
                    resultParam = param;
                    resultSegment = segment;
                    resultRemovedNodeIndex = segment.firstNodeIndex() + removedNodeOffset;
                }
            }
        };


        if (it.isTopBorder() || it.isBottomBorder()) {
            estimateSegment(it.leftSegment(), QPoint(2, 0), pt, *m_d->currentArgs.meshTransform());
            estimateSegment(it.rightSegment(), QPoint(0, 0), pt, *m_d->currentArgs.meshTransform());
        } else {
            estimateSegment(it.topSegment(), QPoint(0, 2), pt, *m_d->currentArgs.meshTransform());
            estimateSegment(it.bottomSegment(), QPoint(0, 0), pt, *m_d->currentArgs.meshTransform());
        }

        if (resultSegment != m_d->currentArgs.meshTransform()->endSegments()) {
            if (!shouldDeleteNode(resultDistance, resultParam)) {
                const qreal eps = 0.01;
                const qreal proportion = KisBezierUtils::curveProportionByParam(resultSegment.p0(), resultSegment.p1(), resultSegment.p2(), resultSegment.p3(), resultParam, eps);

                m_d->currentArgs.meshTransform()->subdivideSegment(resultSegment.segmentIndex(), proportion);
                m_d->currentArgs.meshTransform()->removeColumnOrRow(resultRemovedNodeIndex, !resultSegment.isHorizontal());

            } else {
                m_d->currentArgs.meshTransform()->removeColumnOrRow(m_d->hoveredControl->nodeIndex, !resultSegment.isHorizontal());
            }
        }

    } else if (m_d->hoveredSegment) {
        auto it = m_d->currentArgs.meshTransform()->find(*m_d->hoveredSegment);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(it != m_d->currentArgs.meshTransform()->endSegments(), false);

        qreal distance = 0;
        const qreal t = KisBezierUtils::nearestPoint({it.p0(), it.p1(), it.p2(), it.p3()}, pt, &distance);

        if (!shouldDeleteNode(distance, t)) {
            const qreal eps = 0.01;
            const qreal proportion = KisBezierUtils::curveProportionByParam(it.p0(), it.p1(), it.p2(), it.p3(), t, eps);
            m_d->currentArgs.meshTransform()->subdivideSegment(it.segmentIndex(), proportion);
        }
    }

    m_d->recalculateSignalCompressor.start();

    return true;
}

bool KisMeshTransformStrategy::shouldDeleteNode(qreal distance, qreal param)
{
    const qreal grabRadius = KisTransformUtils::effectiveHandleGrabRadius(m_d->converter);
    return
        distance > 10 * grabRadius ||
        qFuzzyCompare(param, 0.0) ||
        qFuzzyCompare(param, 1.0);

}

bool KisMeshTransformStrategy::beginPrimaryAction(const QPointF &pt)
{
    // retval shows if the stroke may have a continuation
    bool retval = false;

    m_d->mouseClickPos = pt;

    QRectF selectionBounds;

    if (m_d->selectedNodes.size() > 1) {
        for (auto it = m_d->selectedNodes.begin(); it != m_d->selectedNodes.end(); ++it) {
            KisAlgebra2D::accumulateBounds(
                m_d->currentArgs.meshTransform()->node(*it).node, &selectionBounds);
        }
    } else {
        selectionBounds = m_d->currentArgs.meshTransform()->dstBoundingRect();
    }

    m_d->initialRotationCenter = selectionBounds.center();
    m_d->initialSelectionMaxDimension = KisAlgebra2D::maxDimension(selectionBounds);
    m_d->initialMeshState = *m_d->currentArgs.meshTransform();

    m_d->pointWasDragged = false;

    if (m_d->mode == Private::OVER_NODE ||
        m_d->mode == Private::OVER_POINT ||
        m_d->mode == Private::OVER_POINT_SYMMETRIC) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->hoveredControl, false);

        if (m_d->selectedNodes.size() <= 1 ||
            !m_d->selectedNodes.contains(m_d->hoveredControl->nodeIndex)) {

            m_d->selectedNodes.clear();
            m_d->selectedNodes << m_d->hoveredControl->nodeIndex;
        }

        retval = true;
    } else if (m_d->mode == Private::OVER_NODE_WHOLE_LINE) {
        m_d->selectedNodes.clear();
        auto it = m_d->currentArgs.meshTransform()->find(*m_d->hoveredControl);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(it != m_d->currentArgs.meshTransform()->endControlPoints(), false);

        if (it.isTopBorder() || it.isBottomBorder()) {
            for (int i = 0; i < m_d->currentArgs.meshTransform()->size().height(); i++) {
                m_d->selectedNodes << KisBezierTransformMesh::NodeIndex(m_d->hoveredControl->nodeIndex.x(), i);
            }
        } else {
            for (int i = 0; i < m_d->currentArgs.meshTransform()->size().width(); i++) {
                m_d->selectedNodes << KisBezierTransformMesh::NodeIndex(i, m_d->hoveredControl->nodeIndex.y());
            }
        }
        retval = true;
    } else if (m_d->mode == Private::OVER_SEGMENT || m_d->mode == Private::OVER_SEGMENT_SYMMETRIC) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->hoveredSegment, false);

        auto it = m_d->currentArgs.meshTransform()->find(*m_d->hoveredSegment);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(it != m_d->currentArgs.meshTransform()->endSegments(), false);

        retval = true;

    } else if (m_d->mode == Private::OVER_PATCH || m_d->mode == Private::OVER_PATCH_LOCKED) {
        retval = true;

    } else if (m_d->mode == Private::SPLIT_SEGMENT) {
        retval = splitHoveredSegment(pt);

    } else if (m_d->mode == Private::MULTIPLE_POINT_SELECTION) {
        if (m_d->hoveredControl) {
            if (!m_d->selectedNodes.contains(m_d->hoveredControl->nodeIndex)) {
                m_d->selectedNodes.insert(m_d->hoveredControl->nodeIndex);
            } else {
                m_d->selectedNodes.remove(m_d->hoveredControl->nodeIndex);
            }
        } else if (m_d->hoveredSegment) {
            auto it = m_d->currentArgs.meshTransform()->find(*m_d->hoveredSegment);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(it != m_d->currentArgs.meshTransform()->endSegments(), false);

            if (!m_d->selectedNodes.contains(it.firstNodeIndex()) ||
                !m_d->selectedNodes.contains(it.secondNodeIndex())) {

                m_d->selectedNodes.insert(it.firstNodeIndex());
                m_d->selectedNodes.insert(it.secondNodeIndex());
            } else {
                m_d->selectedNodes.remove(it.firstNodeIndex());
                m_d->selectedNodes.remove(it.secondNodeIndex());
            }
        }
        retval = false;
    } else if (m_d->mode == Private::MOVE_MODE ||
               m_d->mode == Private::SCALE_MODE ||
               m_d->mode == Private::ROTATE_MODE) {

        retval = true;
    }

    m_d->lastMousePos = pt;
    return retval;
}

void KisMeshTransformStrategy::continuePrimaryAction(const QPointF &pt, bool shiftModifierActve, bool altModifierActive)
{
    Q_UNUSED(shiftModifierActve);
    Q_UNUSED(altModifierActive);

    if (m_d->mode == Private::OVER_POINT ||
        m_d->mode == Private::OVER_POINT_SYMMETRIC ||
        m_d->mode == Private::OVER_NODE) {

        KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->hoveredControl);

        KisSmartMoveMeshControlMode mode =
            m_d->mode == Private::OVER_POINT_SYMMETRIC ?
            KisSmartMoveMeshControlMode::MoveSymmetricLock :
            KisSmartMoveMeshControlMode::MoveFree;

        smartMoveControl(*m_d->currentArgs.meshTransform(),
                         *m_d->hoveredControl,
                         pt - m_d->lastMousePos,
                         mode);

    } else if (m_d->mode == Private::OVER_SEGMENT || m_d->mode == Private::OVER_SEGMENT_SYMMETRIC) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->hoveredSegment);

        auto it = m_d->currentArgs.meshTransform()->find(*m_d->hoveredSegment);

        // TODO: recover special case for degree-2 curves. There is a special
        //       function for that in KisBezierUtils::interpolateQuadric(), but
        //       it seems like not working properly.

        const QPointF offset = pt - m_d->lastMousePos;

        QPointF offsetP1;
        QPointF offsetP2;

        std::tie(offsetP1, offsetP2) =
            KisBezierUtils::offsetSegment(m_d->localSegmentPosition, offset);


        KisSmartMoveMeshControlMode mode =
            m_d->mode == Private::OVER_SEGMENT_SYMMETRIC ?
            KisSmartMoveMeshControlMode::MoveSymmetricLock :
            KisSmartMoveMeshControlMode::MoveFree;

        smartMoveControl(*m_d->currentArgs.meshTransform(), it.itP1().controlIndex(), offsetP1, mode);
        smartMoveControl(*m_d->currentArgs.meshTransform(), it.itP2().controlIndex(), offsetP2, mode);

    } else if (m_d->mode == Private::OVER_PATCH || m_d->mode == Private::OVER_PATCH_LOCKED) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->hoveredPatch);

        using KisAlgebra2D::linearReshapeFunc;
        using Mesh = KisBezierTransformMesh;

        KisBezierTransformMesh &mesh = *m_d->currentArgs.meshTransform();
        mesh = m_d->initialMeshState;

        auto patchIt = m_d->currentArgs.meshTransform()->find(*m_d->hoveredPatch);

        QPointF offset = pt - m_d->mouseClickPos;

        auto offsetSegment =
            [this] (KisBezierTransformMesh::segment_iterator it,
                    qreal t,
                    const QPointF &offset) {

            QPointF offsetP1;
            QPointF offsetP2;

            std::tie(offsetP1, offsetP2) =
                KisBezierUtils::offsetSegment(t, offset);


            smartMoveControl(*m_d->currentArgs.meshTransform(), it.itP1().controlIndex(), offsetP1, KisSmartMoveMeshControlMode::MoveSymmetricLock);
            smartMoveControl(*m_d->currentArgs.meshTransform(), it.itP2().controlIndex(), offsetP2, KisSmartMoveMeshControlMode::MoveSymmetricLock);
        };


        const QPointF center = patchIt->localToGlobal(QPointF(0.5, 0.5));
        const qreal centerDistance = kisDistance(m_d->mouseClickPos, center);

        KisBezierTransformMesh::segment_iterator nearestSegment = mesh.endSegments();
        qreal nearestSegmentSignificance = 0;
        qreal nearestSegmentDistance = std::numeric_limits<qreal>::max();
        qreal nearestSegmentDistanceSignificance = 0.0;
        qreal nearestSegmentParam = 0.5;

        auto testSegment =
                [&nearestSegment,
                 &nearestSegmentSignificance,
                 &nearestSegmentDistance,
                 &nearestSegmentDistanceSignificance,
                 &nearestSegmentParam,
                 centerDistance,
                 this] (KisBezierTransformMesh::segment_iterator it, qreal param) {

            const QPointF movedPoint = KisBezierUtils::bezierCurve(it.p0(), it.p1(), it.p2(), it.p3(), param);
            const qreal distance = kisDistance(m_d->mouseClickPos, movedPoint);

            if (distance < nearestSegmentDistance) {
                const qreal proportion = KisBezierUtils::curveProportionByParam(it.p0(), it.p1(), it.p2(), it.p3(), param, 0.1);

                qreal distanceSignificance =
                    centerDistance / (centerDistance + distance);

                if (distanceSignificance > 0.6) {
                    distanceSignificance = std::min(1.0, linearReshapeFunc(distanceSignificance, 0.6, 0.75, 0.6, 1.0));
                }

                const qreal directionSignificance =
                    1.0 - std::min(1.0, std::abs(proportion - 0.5) / 0.4);

                nearestSegmentDistance = distance;
                nearestSegment = it;
                nearestSegmentParam = param;
                nearestSegmentSignificance = m_d->mode != Private::OVER_PATCH_LOCKED ? distanceSignificance * directionSignificance : 0;
                nearestSegmentDistanceSignificance = distanceSignificance;
            }
        };

        testSegment(patchIt.segmentP(), m_d->localPatchPosition.x());
        testSegment(patchIt.segmentQ(), m_d->localPatchPosition.x());
        testSegment(patchIt.segmentR(), m_d->localPatchPosition.y());
        testSegment(patchIt.segmentS(), m_d->localPatchPosition.y());

        KIS_SAFE_ASSERT_RECOVER_RETURN(nearestSegment != mesh.endSegments());

        const qreal translationOffsetCoeff =
            qBound(0.0,
                   linearReshapeFunc(1.0 - nearestSegmentDistanceSignificance,
                                     0.95, 0.75, 1.0, 0.0),
                   1.0);
        const QPointF translationOffset = translationOffsetCoeff * offset;
        offset -= translationOffset;

        QPointF segmentOffset;

        if (nearestSegmentSignificance > 0) {
            segmentOffset = nearestSegmentSignificance * offset;
            offset -= segmentOffset;
        }

        const qreal alpha =
            1.0 - KisBezierUtils::curveProportionByParam(nearestSegment.p0(),
                                                         nearestSegment.p1(),
                                                         nearestSegment.p2(),
                                                         nearestSegment.p3(),
                                                         nearestSegmentParam, 0.1);

        const qreal coeffN1 =
            alpha > 0.5 ? std::max(0.0, linearReshapeFunc(alpha, 0.6, 0.75, 1.0, 0.0)) : 1.0;
        const qreal coeffN0 =
            alpha < 0.5 ? std::max(0.0, linearReshapeFunc(alpha, 0.25, 0.4, 0.0, 1.0)) : 1.0;

        nearestSegment.itP0().node().translate(offset * coeffN0);
        nearestSegment.itP3().node().translate(offset * coeffN1);

        patchIt.nodeTopLeft().node().translate(translationOffset);
        patchIt.nodeTopRight().node().translate(translationOffset);
        patchIt.nodeBottomLeft().node().translate(translationOffset);
        patchIt.nodeBottomRight().node().translate(translationOffset);

        offsetSegment(nearestSegment, nearestSegmentParam, segmentOffset);

    } else if (m_d->mode == Private::SPLIT_SEGMENT) {
        *m_d->currentArgs.meshTransform() = m_d->initialMeshState;
        const bool sanitySplitResult = splitHoveredSegment(pt);
        KIS_SAFE_ASSERT_RECOVER_NOOP(sanitySplitResult);

    } else if (m_d->mode == Private::MOVE_MODE || m_d->mode == Private::OVER_NODE_WHOLE_LINE) {
        const QPointF offset = pt - m_d->lastMousePos;
        if (m_d->selectedNodes.size() > 1) {
            for (auto it = m_d->selectedNodes.begin(); it != m_d->selectedNodes.end(); ++it) {
                m_d->currentArgs.meshTransform()->node(*it).translate(offset);
            }
        } else {
            m_d->currentArgs.meshTransform()->translate(offset);
        }
    } else if (m_d->mode == Private::SCALE_MODE) {
        const qreal scale = 1.0 - (pt - m_d->lastMousePos).y() / m_d->initialSelectionMaxDimension;


        const QTransform t =
            QTransform::fromTranslate(-m_d->initialRotationCenter.x(), -m_d->initialRotationCenter.y()) *
            QTransform::fromScale(scale, scale) *
            QTransform::fromTranslate(m_d->initialRotationCenter.x(), m_d->initialRotationCenter.y());

        if (m_d->selectedNodes.size() > 1) {
            for (auto it = m_d->selectedNodes.begin(); it != m_d->selectedNodes.end(); ++it) {
                m_d->currentArgs.meshTransform()->node(*it).transform(t);
            }
        } else {
            m_d->currentArgs.meshTransform()->transform(t);
        }

    } else if (m_d->mode == Private::ROTATE_MODE) {
        const QPointF oldDirection = m_d->lastMousePos - m_d->initialRotationCenter;
        const QPointF newDirection = pt - m_d->initialRotationCenter;
        const qreal rotateAngle = KisAlgebra2D::angleBetweenVectors(oldDirection, newDirection);

        QTransform R;
        R.rotateRadians(rotateAngle);

        const QTransform t =
            QTransform::fromTranslate(-m_d->initialRotationCenter.x(), -m_d->initialRotationCenter.y()) *
            R *
            QTransform::fromTranslate(m_d->initialRotationCenter.x(), m_d->initialRotationCenter.y());

        if (m_d->selectedNodes.size() > 1) {
            for (auto it = m_d->selectedNodes.begin(); it != m_d->selectedNodes.end(); ++it) {
                m_d->currentArgs.meshTransform()->node(*it).transform(t);
            }
        } else {
            m_d->currentArgs.meshTransform()->transform(t);
        }
    }

    m_d->lastMousePos = pt;
    m_d->recalculateSignalCompressor.start();
}

bool KisMeshTransformStrategy::endPrimaryAction()
{
    return m_d->mode != Private::NOTHING;
}

bool KisMeshTransformStrategy::acceptsClicks() const
{
    return m_d->mode == Private::SPLIT_SEGMENT;
}

QTransform KisMeshTransformStrategy::Private::imageToThumb(bool useFlakeOptimization)
{
    return useFlakeOptimization ?
        converter->documentToFlakeTransform() * converter->imageToDocumentTransform() :
        q->thumbToImageTransform().inverted();
}

void KisMeshTransformStrategy::Private::recalculateTransformations()
{
    const QTransform scaleTransform = KisTransformUtils::imageToFlakeTransform(converter);

    const QTransform resultThumbTransform = q->thumbToImageTransform() * scaleTransform;
    const qreal scale = KisTransformUtils::scaleFromAffineMatrix(resultThumbTransform);
    const bool useFlakeOptimization = scale < 1.0 &&
        !KisTransformUtils::thumbnailTooSmall(resultThumbTransform, q->originalImage().rect());

    const QTransform imageToThumb = this->imageToThumb(useFlakeOptimization);
    KIS_SAFE_ASSERT_RECOVER_RETURN(imageToThumb.type() <= QTransform::TxScale);

    KisBezierTransformMesh mesh(*currentArgs.meshTransform());
    mesh.transformSrcAndDst(imageToThumb);

    paintingOffset = transaction.originalTopLeft();

    if (!q->originalImage().isNull()) {
        const QPointF origTLInFlake = imageToThumb.map(transaction.originalTopLeft());
        if (useFlakeOptimization) {
            transformedImage = q->originalImage().transformed(resultThumbTransform);
            paintingTransform = QTransform();
        } else {
            transformedImage = q->originalImage();
            paintingTransform = resultThumbTransform;

        }

        const QRect dstImageRect = mesh.dstBoundingRect().toAlignedRect();
        QImage dstImage(dstImageRect.size(), transformedImage.format());
        dstImage.fill(0);

        mesh.transformMesh(origTLInFlake.toPoint(), transformedImage,
                           dstImageRect.topLeft(), &dstImage);

        transformedImage = dstImage;
        paintingOffset = dstImageRect.topLeft();

    } else {
        transformedImage = q->originalImage();
        paintingOffset = imageToThumb.map(transaction.originalTopLeft());
        paintingTransform = resultThumbTransform;
    }

    Q_EMIT q->requestCanvasUpdate();
}

#include "moc_kis_mesh_transform_strategy.cpp"
