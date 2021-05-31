/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2004 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPainter>
#include <QContextMenuEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QPolygon>
#include <QPaintEvent>
#include <QMenu>
#include <QStyleOptionToolButton>
#include <QWindow>
#include <QAction>

#include <kis_debug.h>
#include <klocalizedstring.h>
#include <resources/KoSegmentGradient.h>
#include <KisGradientWidgetsUtils.h>
#include <KisDlgInternalColorSelector.h>
#include <krita_utils.h>

#include "KisSegmentGradientSlider.h"

#define MARGIN 5
#define HANDLE_SIZE 10

KisSegmentGradientSlider::KisSegmentGradientSlider(QWidget *parent, const char* name, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_drag(false)
    , m_updateCompressor(40, KisSignalCompressor::FIRST_ACTIVE)
{
    setObjectName(name);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setFocusPolicy(Qt::WheelFocus);

    connect(this, SIGNAL(updateRequested()), &m_updateCompressor, SLOT(start()));
    connect(&m_updateCompressor, SIGNAL(timeout()), this, SLOT(update()));

    QWindow *window = this->window()->windowHandle();
    if (window) {
        connect(window, SIGNAL(screenChanged(QScreen*)), SLOT(updateHandleSize()));
    }
    updateHandleSize();
}

void KisSegmentGradientSlider::setGradientResource(KoSegmentGradientSP agr)
{
    m_gradient = agr;
    m_selectedHandle = { HandleType_Stop, 0 };
    emit selectedHandleChanged();
    emit updateRequested();
}

void KisSegmentGradientSlider::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    const QRect previewRect = gradientStripeRect();
    
    if (m_gradient) {
        // Gradient
        KisGradientWidgetsUtils::paintGradientBox(painter, m_gradient, previewRect);
        // Haldles
        QList<KoGradientSegment*> segments = m_gradient->segments();
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QRect handlesRect = this->handlesStripeRect();
        const bool hasFocus = this->hasFocus();
        // Segment handles
        if (m_selectedHandle.type == HandleType_Segment) {
            const KoGradientSegment *selectedSegment = segments[m_selectedHandle.index];
            QRectF segmentHandleRect =
                handlesRect.adjusted(
                    selectedSegment->startOffset() * handlesRect.width(),
                    -1,
                    -(handlesRect.width() - selectedSegment->endOffset() * handlesRect.width()),
                    -4
                );
            painter.fillRect(segmentHandleRect, palette().highlight());
        }
        if (m_hoveredHandle.type == HandleType_Segment &&
            (m_selectedHandle.type != HandleType_Segment || m_hoveredHandle.index != m_selectedHandle.index)) {
            const KoGradientSegment *hoveredSegment = segments[m_hoveredHandle.index];
            QRectF segmentHandleRect =
                handlesRect.adjusted(
                    hoveredSegment->startOffset() * handlesRect.width(),
                    -1,
                    -(handlesRect.width() - hoveredSegment->endOffset() * handlesRect.width()),
                    -4
                );
            QColor c = palette().highlight().color();
            c.setAlpha(96);
            painter.fillRect(segmentHandleRect, c);
        }
        // Mid-Point handles
        const qreal midPointHandleSize = m_handleSize.height() * 0.5;
        const qreal midPointHandleOffsetY = (handlesRect.height() - 5.0 - midPointHandleSize) * 0.5;
        for (int i = 0; i < segments.count(); i++) {
            if (m_selectedHandle.type == HandleType_MidPoint && m_selectedHandle.index == i) {
                // If this handle is selected then we will paint it later
                // on top of everything else
                continue;
            }
            QPointF handlePos =
                handlesRect.topLeft() +
                QPointF(segments[i]->middleOffset() * handlesRect.width(), midPointHandleOffsetY);
            KisGradientWidgetsUtils::paintMidPointHandle(
                painter, handlePos, midPointHandleSize,
                false, m_hoveredHandle.type == HandleType_MidPoint && m_hoveredHandle.index == i, hasFocus,
                palette().windowText().color(), palette().window().color(), palette().highlight().color()
            );
        }
        // Stop handles
        const QColor highlightColor = palette().color(QPalette::Highlight);
        // First stop if it is not selected, in which case it will be painted 
        // later on top of everything else
        if (m_selectedHandle.type != HandleType_Stop || m_selectedHandle.index != 0) {
            KoGradientSegment* segment = segments.front();
            KisGradientWidgetsUtils::ColorType colorType = KisGradientWidgetsUtils::segmentEndPointTypeToColorType(segment->startType());
            // Pass the color info as color 2 so that the type indicator is
            // shown on the right side of the handle for this stop
            KisGradientWidgetsUtils::paintStopHandle(
                painter,
                QPointF(handlesRect.left() + segment->startOffset() * handlesRect.width(), handlesRect.top()),
                QSizeF(m_handleSize),
                false, m_hoveredHandle.type == HandleType_Stop && m_hoveredHandle.index == 0, hasFocus,
                highlightColor,
                {},
                { colorType, segment->startColor().toQColor() }
            );
        }
        // Middle stops
        if (segments.size() > 1) {
            for (int i = 0; i < segments.count() - 1; ++i) {
                if (m_selectedHandle.type == HandleType_Stop && m_selectedHandle.index == i + 1) {
                    // If this handle is selected then we will paint it later
                    // on top of everything else
                    continue;
                }
                KoGradientSegment* currentSegment = segments[i];
                KoGradientSegment* nextSegment = segments[i + 1];
                // If the end point of the current segment and the start point
                // of the next segmenthave the same offset, that means the
                // segments touch each other, are connected (normal behavior),
                // so we paint a single special handle.
                // If the end points have different offsets then krita stills
                // considers the gradient valid (although Gimp doesn't) so we
                // paint two different handles. 
                if (currentSegment->endOffset() == nextSegment->startOffset()) {
                    KisGradientWidgetsUtils::paintStopHandle(
                        painter,
                        QPointF(handlesRect.left() + currentSegment->endOffset() * handlesRect.width(), handlesRect.top()),
                        QSizeF(m_handleSize),
                        false, m_hoveredHandle.type == HandleType_Stop && m_hoveredHandle.index == i + 1, hasFocus,
                        highlightColor,
                        { KisGradientWidgetsUtils::segmentEndPointTypeToColorType(currentSegment->endType()), currentSegment->endColor().toQColor() },
                        { KisGradientWidgetsUtils::segmentEndPointTypeToColorType(nextSegment->startType()), nextSegment->startColor().toQColor() }
                    );
                } else {
                    KisGradientWidgetsUtils::paintStopHandle(
                        painter,
                        QPointF(handlesRect.left() + currentSegment->endOffset() * handlesRect.width(), handlesRect.top()),
                        QSizeF(m_handleSize),
                        false, m_hoveredHandle.type == HandleType_Stop && m_hoveredHandle.index == i + 1, hasFocus,
                        highlightColor,
                        { KisGradientWidgetsUtils::segmentEndPointTypeToColorType(currentSegment->endType()), currentSegment->endColor().toQColor() }
                    );
                    KisGradientWidgetsUtils::paintStopHandle(
                        painter,
                        QPointF(handlesRect.left() + nextSegment->startOffset() * handlesRect.width(), handlesRect.top()),
                        QSizeF(m_handleSize),
                        false, m_hoveredHandle.type == HandleType_Stop && m_hoveredHandle.index == i + 1, hasFocus,
                        highlightColor,
                        { KisGradientWidgetsUtils::segmentEndPointTypeToColorType(nextSegment->startType()), nextSegment->startColor().toQColor() }
                    );
                }
            }
        }
        // Last stop. This is the last thing to be painted before the selected
        // handle, so we don't need to make a special case here
        {
            KoGradientSegment* segment = segments.back();
            KisGradientWidgetsUtils::ColorType colorType = KisGradientWidgetsUtils::segmentEndPointTypeToColorType(segment->endType());
            KisGradientWidgetsUtils::paintStopHandle(
                painter,
                QPointF(handlesRect.left() + segment->endOffset() * handlesRect.width(), handlesRect.top()),
                QSizeF(m_handleSize),
                m_selectedHandle.type == HandleType_Stop && m_selectedHandle.index == segments.size(),
                m_hoveredHandle.type == HandleType_Stop && m_hoveredHandle.index == segments.size() &&
                (m_selectedHandle.type != HandleType_Stop || m_selectedHandle.index != segments.size()),
                hasFocus,
                highlightColor,
                { colorType, segment->endColor().toQColor() }
            );
        }
        // Selected stop
        if (m_selectedHandle.type == HandleType_MidPoint) {
            QPointF handlePos =
                handlesRect.topLeft() +
                QPointF(segments[m_selectedHandle.index]->middleOffset() * handlesRect.width(), midPointHandleOffsetY);
            KisGradientWidgetsUtils::paintMidPointHandle(
                painter, handlePos, midPointHandleSize,
                true, false, hasFocus,
                palette().windowText().color(), palette().window().color(), palette().highlight().color()
            );
        } else if (m_selectedHandle.type == HandleType_Stop) {
            if (m_selectedHandle.index == 0) {
                KoGradientSegment* segment = segments.front();
                KisGradientWidgetsUtils::ColorType colorType = KisGradientWidgetsUtils::segmentEndPointTypeToColorType(segment->startType());
                // Pass the color info as color 2 so that the type indicator is
                // shown on the right side of the handle for this stop
                KisGradientWidgetsUtils::paintStopHandle(
                    painter,
                    QPointF(handlesRect.left() + segment->startOffset() * handlesRect.width(), handlesRect.top()),
                    QSizeF(m_handleSize),
                    true, false, hasFocus,
                    highlightColor,
                    {},
                    { colorType, segment->startColor().toQColor() }
                );
            } else if (m_selectedHandle.index < segments.size()) {
                KoGradientSegment* currentSegment = segments[m_selectedHandle.index - 1];
                KoGradientSegment* nextSegment = segments[m_selectedHandle.index];
                if (currentSegment->endOffset() == nextSegment->startOffset()) {
                    KisGradientWidgetsUtils::paintStopHandle(
                        painter,
                        QPointF(handlesRect.left() + currentSegment->endOffset() * handlesRect.width(), handlesRect.top()),
                        QSizeF(m_handleSize),
                        true, false, hasFocus,
                        highlightColor,
                        { KisGradientWidgetsUtils::segmentEndPointTypeToColorType(currentSegment->endType()), currentSegment->endColor().toQColor() },
                        { KisGradientWidgetsUtils::segmentEndPointTypeToColorType(nextSegment->startType()), nextSegment->startColor().toQColor() }
                    );
                } else {
                    KisGradientWidgetsUtils::paintStopHandle(
                        painter,
                        QPointF(handlesRect.left() + currentSegment->endOffset() * handlesRect.width(), handlesRect.top()),
                        QSizeF(m_handleSize),
                        true, false, hasFocus,
                        highlightColor,
                        { KisGradientWidgetsUtils::segmentEndPointTypeToColorType(currentSegment->endType()), currentSegment->endColor().toQColor() }
                    );
                    KisGradientWidgetsUtils::paintStopHandle(
                        painter,
                        QPointF(handlesRect.left() + nextSegment->startOffset() * handlesRect.width(), handlesRect.top()),
                        QSizeF(m_handleSize),
                        true, false, hasFocus,
                        highlightColor,
                        { KisGradientWidgetsUtils::segmentEndPointTypeToColorType(nextSegment->startType()), nextSegment->startColor().toQColor() }
                    );
                }
            }
        }
    } else {
        painter.setPen(palette().color(QPalette::Mid));
        painter.drawRect(previewRect);
    }
}

void KisSegmentGradientSlider::mousePressEvent(QMouseEvent * e)
{
    if (e->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(e);
        return;
    }

    const QRect rect = sliderRect();
    const QRect handlesRect = handlesStripeRect();

    // Find segment under cursor
    Handle selectedHandle;
    const qreal t = (e->x() - rect.left()) / static_cast<qreal>(rect.width());
    const qreal handleClickTolerance = m_handleSize.width() / static_cast<qreal>(rect.width());
    m_dragT = t;

    for (int i = 0; i < m_gradient->segments().size(); ++i) {
        KoGradientSegment *segment = m_gradient->segments()[i];
        // Check if a knob was pressed
        if (qAbs(t - segment->startOffset()) <= handleClickTolerance && e->pos().y() >= handlesRect.y()) {
            // Left knob was pressed
            selectedHandle.type = HandleType_Stop;
            selectedHandle.index = i;
            m_drag = true;
            break;
        } else if (qAbs(t - segment->endOffset()) <= handleClickTolerance && e->pos().y() >= handlesRect.y()) {
            // Right knob was pressed
            selectedHandle.type = HandleType_Stop;
            selectedHandle.index = i + 1;
            m_drag = true;
            break;
        } else if (qAbs(t - segment->middleOffset()) <= handleClickTolerance && e->pos().y() >= handlesRect.y()) {
            // middle knob was pressed
            selectedHandle.type = HandleType_MidPoint;
            selectedHandle.index = i;
            m_drag = true;
            break;
        } else if (t >= segment->startOffset() && t <= segment->endOffset()) {
            // the segment area was pressed
            selectedHandle.type = HandleType_Segment;
            selectedHandle.index = i;
            if (e->modifiers() & Qt::ControlModifier) {
                KoColor color;
                m_gradient->colorAt(color, t);
                m_selectedHandle = selectedHandle;
                segment->setMiddleOffset(t);
                m_gradient->splitSegment(segment);
                m_selectedHandle.type = HandleType_Stop;
                m_selectedHandle.index = i + 1;
                m_gradient->segments()[i]->setEndColor(color);
                m_gradient->segments()[i + 1]->setStartColor(color);
                m_drag = true;
                emit selectedHandleChanged();
                emit updateRequested();
                return;
            } else if (e->modifiers() & Qt::ShiftModifier) {
                m_selectedHandle = selectedHandle;
                duplicateSelectedSegment();
                return;
            }
            m_drag = true;
            m_relativeDragOffset = t - segment->startOffset();
            break;
        }
    }

    if (m_drag) {
        m_hoveredHandle = {};
    }

    if (m_selectedHandle.type != selectedHandle.type || m_selectedHandle.index != selectedHandle.index) {
        m_selectedHandle = selectedHandle;
        emit selectedHandleChanged();
        emit updateRequested();
    }
}

void KisSegmentGradientSlider::mouseReleaseEvent(QMouseEvent * e)
{
    Q_UNUSED(e);
    m_temporallyDeletedHandleInfo.handle.type = HandleType_None;
    m_drag = false;
}

void KisSegmentGradientSlider::mouseMoveEvent(QMouseEvent * e)
{
    const QRect rect = sliderRect();
    const qreal t = (e->x() - rect.left()) / static_cast<qreal>(rect.width());

    if (m_drag) {
        if (!(e->buttons() & Qt::LeftButton)) {
            QWidget::mouseMoveEvent(e);
            return;
        }
        const QRect augmentedRect = kisGrowRect(this->rect(), removeStopDistance);
        if (m_temporallyDeletedHandleInfo.handle.type == HandleType_Segment) {
            if (augmentedRect.contains(e->pos())) {
                m_gradient->duplicateSegment(m_gradient->segments()[m_temporallyDeletedHandleInfo.handle.index - 1]);
                KoGradientSegment *segment = m_gradient->segments()[m_temporallyDeletedHandleInfo.handle.index];
                segment->setStartType(m_temporallyDeletedHandleInfo.leftEndPointType);
                segment->setStartOffset(m_temporallyDeletedHandleInfo.leftEndPointOffset);
                segment->setStartColor(m_temporallyDeletedHandleInfo.leftEndPointColor);
                segment->setEndType(m_temporallyDeletedHandleInfo.rightEndPointType);
                segment->setEndOffset(m_temporallyDeletedHandleInfo.rightEndPointOffset);
                segment->setEndColor(m_temporallyDeletedHandleInfo.rightEndPointColor);
                segment->setMiddleOffset(m_temporallyDeletedHandleInfo.leftMiddleOffset);
                segment->setInterpolation(m_temporallyDeletedHandleInfo.leftInterpolationType);
                segment->setColorInterpolation(m_temporallyDeletedHandleInfo.leftColorInterpolationType);
                m_selectedHandle.type = HandleType_Segment;
                m_selectedHandle.index = m_temporallyDeletedHandleInfo.handle.index;
                m_temporallyDeletedHandleInfo.handle.type = HandleType_None;
            }
        } else if (m_temporallyDeletedHandleInfo.handle.type == HandleType_Stop) {
            if (augmentedRect.contains(e->pos())) {
                m_gradient->duplicateSegment(m_gradient->segments()[m_temporallyDeletedHandleInfo.handle.index - 1]);
                KoGradientSegment *previousSegment = m_gradient->segments()[m_temporallyDeletedHandleInfo.handle.index - 1];
                KoGradientSegment *nextSegment = m_gradient->segments()[m_temporallyDeletedHandleInfo.handle.index];
                previousSegment->setEndType(m_temporallyDeletedHandleInfo.leftEndPointType);
                previousSegment->setEndColor(m_temporallyDeletedHandleInfo.leftEndPointColor);
                previousSegment->setInterpolation(m_temporallyDeletedHandleInfo.leftInterpolationType);
                previousSegment->setColorInterpolation(m_temporallyDeletedHandleInfo.leftColorInterpolationType);
                previousSegment->setMiddleOffset(
                    previousSegment->startOffset() +
                    (m_temporallyDeletedHandleInfo.leftMiddleOffset - previousSegment->startOffset()) /
                    (m_temporallyDeletedHandleInfo.leftEndPointOffset - previousSegment->startOffset()) *
                    previousSegment->length()
                );
                nextSegment->setStartType(m_temporallyDeletedHandleInfo.rightEndPointType);
                nextSegment->setStartColor(m_temporallyDeletedHandleInfo.rightEndPointColor);
                nextSegment->setInterpolation(m_temporallyDeletedHandleInfo.rightInterpolationType);
                nextSegment->setColorInterpolation(m_temporallyDeletedHandleInfo.rightColorInterpolationType);
                nextSegment->setMiddleOffset(
                    nextSegment->startOffset() +
                    (m_temporallyDeletedHandleInfo.rightMiddleOffset - m_temporallyDeletedHandleInfo.rightEndPointOffset) /
                    (nextSegment->endOffset() - m_temporallyDeletedHandleInfo.rightEndPointOffset) *
                    nextSegment->length()
                );
                m_selectedHandle.type = HandleType_Stop;
                m_selectedHandle.index = m_temporallyDeletedHandleInfo.handle.index;
                m_temporallyDeletedHandleInfo.handle.type = HandleType_None;
            }
        }

        if (m_selectedHandle.type == HandleType_Segment) {
            if (m_temporallyDeletedHandleInfo.handle.type == HandleType_None) {
                KoGradientSegment *segment = m_gradient->segments()[m_selectedHandle.index];
                if (m_gradient->segments().size() > 1 && m_selectedHandle.index > 0 && m_selectedHandle.index < m_gradient->segments().size() - 1 &&
                    !augmentedRect.contains(e->pos())) {
                    m_temporallyDeletedHandleInfo.handle.type = HandleType_Segment;
                    m_temporallyDeletedHandleInfo.handle.index = m_selectedHandle.index;
                    m_temporallyDeletedHandleInfo.leftEndPointType = segment->startType();
                    m_temporallyDeletedHandleInfo.leftEndPointOffset = segment->startOffset();
                    m_temporallyDeletedHandleInfo.leftEndPointColor = segment->startColor();
                    m_temporallyDeletedHandleInfo.rightEndPointType = segment->endType();
                    m_temporallyDeletedHandleInfo.rightEndPointOffset = segment->endOffset();
                    m_temporallyDeletedHandleInfo.rightEndPointColor = segment->endColor();
                    m_temporallyDeletedHandleInfo.leftInterpolationType = segment->interpolation();
                    m_temporallyDeletedHandleInfo.leftColorInterpolationType = segment->colorInterpolation();
                    m_temporallyDeletedHandleInfo.leftMiddleOffset = segment->middleOffset();
                    m_gradient->collapseSegment(m_gradient->segments()[m_selectedHandle.index]);
                    m_selectedHandle.type = HandleType_None;
                } else {
                    KoGradientSegment *segment = m_gradient->segments()[m_selectedHandle.index];
                    KoGradientSegment *previousSegment = m_selectedHandle.index == 0 ? nullptr : m_gradient->segments()[m_selectedHandle.index - 1];
                    KoGradientSegment *nextSegment = m_selectedHandle.index == m_gradient->segments().size() - 1 ? nullptr : m_gradient->segments()[m_selectedHandle.index + 1];
                    if (previousSegment && nextSegment) {
                        const qreal midPointRelativePos = segment->middleOffset() - segment->startOffset();
                        const qreal previousMidPointLocalPos =
                            previousSegment->length() > std::numeric_limits<qreal>::epsilon()
                            ? (previousSegment->middleOffset() - previousSegment->startOffset()) / previousSegment->length()
                            : 0.0;
                        const qreal nextMidPointLocalPos =
                            nextSegment->length() > std::numeric_limits<qreal>::epsilon()
                            ? (nextSegment->middleOffset() - nextSegment->startOffset()) / nextSegment->length()
                            : 0.0;
                        qreal newStartOffset, newEndOffset;
                        if (t < m_dragT) {
                            newStartOffset = qMax(t - m_relativeDragOffset, previousSegment->startOffset() + shrinkEpsilon);
                            newEndOffset = newStartOffset + segment->length();
                        } else {
                            newEndOffset = qMin(t + (segment->length() - m_relativeDragOffset), nextSegment->endOffset() - shrinkEpsilon);
                            newStartOffset = newEndOffset - segment->length();
                        }
                        previousSegment->setEndOffset(newStartOffset);
                        segment->setStartOffset(newStartOffset);
                        segment->setEndOffset(newEndOffset);
                        nextSegment->setStartOffset(newEndOffset);
                        previousSegment->setMiddleOffset(
                            previousSegment->startOffset() +
                            previousMidPointLocalPos * previousSegment->length()
                        );
                        nextSegment->setMiddleOffset(
                            nextSegment->startOffset() +
                            nextMidPointLocalPos * nextSegment->length()
                        );
                        segment->setMiddleOffset(segment->startOffset() + midPointRelativePos);
                    } else {
                        if (!previousSegment) {
                            segment->setStartOffset(0.0);
                        }
                        if (!nextSegment) {
                            segment->setEndOffset(1.0);
                        }
                    }
                }
                emit selectedHandleChanged();
                emit updateRequested();
            }

        } else if (m_selectedHandle.type == HandleType_Stop) {
            if (m_temporallyDeletedHandleInfo.handle.type == HandleType_None) {
                KoGradientSegment *previousSegment = m_selectedHandle.index == 0 ? nullptr : m_gradient->segments()[m_selectedHandle.index - 1];
                KoGradientSegment *nextSegment = m_selectedHandle.index == m_gradient->segments().size() ? nullptr : m_gradient->segments()[m_selectedHandle.index];
                if (m_gradient->segments().size() > 1 && m_selectedHandle.index > 0 && m_selectedHandle.index < m_gradient->segments().size() &&
                    !augmentedRect.contains(e->pos())) {
                    m_temporallyDeletedHandleInfo.handle.type = HandleType_Stop;
                    m_temporallyDeletedHandleInfo.handle.index = m_selectedHandle.index;
                    m_temporallyDeletedHandleInfo.leftEndPointType = previousSegment->endType();
                    m_temporallyDeletedHandleInfo.leftEndPointOffset = previousSegment->endOffset();
                    m_temporallyDeletedHandleInfo.leftEndPointColor = previousSegment->endColor();
                    m_temporallyDeletedHandleInfo.leftInterpolationType = previousSegment->interpolation();
                    m_temporallyDeletedHandleInfo.leftColorInterpolationType = previousSegment->colorInterpolation();
                    m_temporallyDeletedHandleInfo.leftMiddleOffset = previousSegment->middleOffset();
                    m_temporallyDeletedHandleInfo.rightEndPointType = nextSegment->startType();
                    m_temporallyDeletedHandleInfo.rightEndPointOffset = nextSegment->startOffset();
                    m_temporallyDeletedHandleInfo.rightEndPointColor = nextSegment->startColor();
                    m_temporallyDeletedHandleInfo.rightInterpolationType = nextSegment->interpolation();
                    m_temporallyDeletedHandleInfo.rightColorInterpolationType = nextSegment->colorInterpolation();
                    m_temporallyDeletedHandleInfo.rightMiddleOffset = nextSegment->middleOffset();
                    previousSegment->setEndType(nextSegment->endType());
                    previousSegment->setEndColor(nextSegment->endColor());
                    deleteHandleImpl(m_selectedHandle);
                    m_selectedHandle.type = HandleType_None;
                } else {
                    KoGradientSegment *previousSegment = m_selectedHandle.index == 0 ? nullptr : m_gradient->segments()[m_selectedHandle.index - 1];
                    KoGradientSegment *nextSegment = m_selectedHandle.index == m_gradient->segments().size() ? nullptr : m_gradient->segments()[m_selectedHandle.index];
                    if (previousSegment && nextSegment) {
                        const qreal previousMidPointLocalPos =
                            previousSegment->length() > std::numeric_limits<qreal>::epsilon()
                            ? (previousSegment->middleOffset() - previousSegment->startOffset()) / previousSegment->length()
                            : 0.0;
                        const qreal nextMidPointLocalPos =
                            nextSegment->length() > std::numeric_limits<qreal>::epsilon()
                            ? (nextSegment->middleOffset() - nextSegment->startOffset()) / nextSegment->length()
                            : 0.0;
                        qreal newOffset;
                        if (t < m_dragT) {
                            newOffset = qMax(t, previousSegment->startOffset() + shrinkEpsilon);
                        } else {
                            newOffset = qMin(t, nextSegment->endOffset() - shrinkEpsilon);
                        }
                        previousSegment->setEndOffset(newOffset);
                        nextSegment->setStartOffset(newOffset);
                        previousSegment->setMiddleOffset(
                            previousSegment->startOffset() +
                            previousMidPointLocalPos * previousSegment->length()
                        );
                        nextSegment->setMiddleOffset(
                            nextSegment->startOffset() +
                            nextMidPointLocalPos * nextSegment->length()
                        );
                    } else {
                        if (!previousSegment) {
                            nextSegment->setStartOffset(0.0);
                        }
                        if (!nextSegment) {
                            previousSegment->setEndOffset(1.0);
                        }
                    }
                }
                emit selectedHandleChanged();
                emit updateRequested();
            }

        } else if (m_selectedHandle.type == HandleType_MidPoint) {
            KoGradientSegment *segment = m_gradient->segments()[m_selectedHandle.index];
            segment->setMiddleOffset(qBound(segment->startOffset(), t, segment->endOffset()));
            emit selectedHandleChanged();
            emit updateRequested();
        }

    } else {
        const QRect handlesRect = handlesStripeRect();
        Handle hoveredHandle;
        const qreal handleClickTolerance = m_handleSize.width() / static_cast<qreal>(rect.width());
        for (int i = 0; i < m_gradient->segments().size(); ++i) {
            KoGradientSegment *segment = m_gradient->segments()[i];
            // Check if a knob was hovered
            if (qAbs(t - segment->startOffset()) <= handleClickTolerance && e->pos().y() >= handlesRect.y()) {
                // Left knob was hovered
                hoveredHandle.type = HandleType_Stop;
                hoveredHandle.index = i;
                break;
            } else if (qAbs(t - segment->endOffset()) <= handleClickTolerance && e->pos().y() >= handlesRect.y()) {
                // Right knob was hovered
                hoveredHandle.type = HandleType_Stop;
                hoveredHandle.index = i + 1;
                break;
            } else if (qAbs(t - segment->middleOffset()) <= handleClickTolerance && e->pos().y() >= handlesRect.y()) {
                // middle knob was hovered
                hoveredHandle.type = HandleType_MidPoint;
                hoveredHandle.index = i;
                break;
            } else if (t >= segment->startOffset() && t <= segment->endOffset()) {
                // the segment area was hovered
                hoveredHandle.type = HandleType_Segment;
                hoveredHandle.index = i;
                break;
            }
        }
        m_hoveredHandle = hoveredHandle;
        emit updateRequested();
    }
}

void KisSegmentGradientSlider::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        QWidget::mouseDoubleClickEvent(e);
        return;
    }

    const QRect rect = sliderRect();
    const QRect handlesRect = handlesStripeRect();
    const qreal t = (e->x() - rect.left()) / static_cast<qreal>(rect.width());
    const qreal handleClickTolerance = m_handleSize.width() / static_cast<qreal>(rect.width());
    const KoGradientSegment *previousSegment = m_selectedHandle.index == 0 ? nullptr : m_gradient->segments()[m_selectedHandle.index - 1];
    const KoGradientSegment *nextSegment = m_selectedHandle.index == m_gradient->segments().size() ? nullptr : m_gradient->segments()[m_selectedHandle.index];

    if ((previousSegment && qAbs(t - previousSegment->endOffset()) <= handleClickTolerance && e->pos().y() >= handlesRect.y()) ||
        (nextSegment && qAbs(t - nextSegment->startOffset()) <= handleClickTolerance && e->pos().y() >= handlesRect.y())) {
        chooseSelectedStopColor();
    }
}

void KisSegmentGradientSlider::selectPreviousHandle()
{
    if (m_selectedHandle.type == HandleType_Segment) {
        m_selectedHandle.type = HandleType_Stop;
        emit selectedHandleChanged();
        emit updateRequested();
    } else if (m_selectedHandle.type == HandleType_Stop) {
        if (m_selectedHandle.index > 0) {
            m_selectedHandle.type = HandleType_MidPoint;
            --m_selectedHandle.index;
            emit selectedHandleChanged();
            emit updateRequested();
        }
    } else if (m_selectedHandle.type == HandleType_MidPoint) {
        m_selectedHandle.type = HandleType_Segment;
        emit selectedHandleChanged();
        emit updateRequested();
    }
}

void KisSegmentGradientSlider::selectNextHandle()
{
    if (m_selectedHandle.type == HandleType_Segment) {
        m_selectedHandle.type = HandleType_MidPoint;
        emit selectedHandleChanged();
        emit updateRequested();
    } else if (m_selectedHandle.type == HandleType_Stop) {
        if (m_selectedHandle.index < m_gradient->segments().size()) {
            m_selectedHandle.type = HandleType_Segment;
            emit selectedHandleChanged();
            emit updateRequested();
        }
    } else if (m_selectedHandle.type == HandleType_MidPoint) {
        m_selectedHandle.type = HandleType_Stop;
        ++m_selectedHandle.index;
        emit selectedHandleChanged();
        emit updateRequested();
    }
}

void KisSegmentGradientSlider::handleIncrementInput(int direction, Qt::KeyboardModifiers modifiers)
{
    if (direction == 0) {
        return;
    }
    if (modifiers & Qt::ControlModifier) {
        if (direction < 0) {
            selectPreviousHandle();
        } else {
            selectNextHandle();
        }
    } else {
        const qreal increment = modifiers & Qt::ShiftModifier ? 0.001 : 0.01;
        moveSelectedHandle(direction < 0 ? -increment : increment);
    }
}

void KisSegmentGradientSlider::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() != 0) {
        handleIncrementInput(e->angleDelta().y(), e->modifiers());
        e->accept();
    } else {
        QWidget::wheelEvent(e);
    }
}

void KisSegmentGradientSlider::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Left:
        handleIncrementInput(-1, e->modifiers());
        break;
    case Qt::Key_Right:
        handleIncrementInput(1, e->modifiers());
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        chooseSelectedStopColor();
        break;
    case Qt::Key_Delete:
        deleteSelectedHandle();
        break;
    default:
        QWidget::keyPressEvent(e);
        break;
    }
}

void KisSegmentGradientSlider::leaveEvent(QEvent *e)
{
    m_hoveredHandle = {};
    emit updateRequested();
    QWidget::leaveEvent(e);
}

void KisSegmentGradientSlider::moveHandle(Handle handle, qreal distance, bool useShrinkEpsilon)
{
    const qreal epsilon = useShrinkEpsilon ? shrinkEpsilon : 0.0; 
    if (handle.type == HandleType_Segment) {
        KoGradientSegment *segment = m_gradient->segments()[handle.index];
        KoGradientSegment *previousSegment = handle.index == 0 ? nullptr : m_gradient->segments()[handle.index - 1];
        KoGradientSegment *nextSegment = handle.index == m_gradient->segments().size() - 1 ? nullptr : m_gradient->segments()[handle.index + 1];
        if (previousSegment && nextSegment) {
            const qreal midPointRelativePos = segment->middleOffset() - segment->startOffset();
            const qreal previousMidPointLocalPos =
                previousSegment->length() > std::numeric_limits<qreal>::epsilon()
                ? (previousSegment->middleOffset() - previousSegment->startOffset()) / previousSegment->length()
                : 0.0;
            const qreal nextMidPointLocalPos =
                nextSegment->length() > std::numeric_limits<qreal>::epsilon()
                ? (nextSegment->middleOffset() - nextSegment->startOffset()) / nextSegment->length()
                : 0.0;
            qreal newStartOffset, newEndOffset;
            if (distance < 0.0) {
                newStartOffset = qMax(segment->startOffset() + distance, previousSegment->startOffset() + epsilon);
                newEndOffset = newStartOffset + segment->length();
            } else {
                newEndOffset = qMin(segment->endOffset() + distance, nextSegment->endOffset() - epsilon);
                newStartOffset = newEndOffset - segment->length();
            }
            previousSegment->setEndOffset(newStartOffset);
            segment->setStartOffset(newStartOffset);
            segment->setEndOffset(newEndOffset);
            nextSegment->setStartOffset(newEndOffset);
            previousSegment->setMiddleOffset(
                previousSegment->startOffset() +
                previousMidPointLocalPos * previousSegment->length()
            );
            nextSegment->setMiddleOffset(
                nextSegment->startOffset() +
                nextMidPointLocalPos * nextSegment->length()
            );
            segment->setMiddleOffset(segment->startOffset() + midPointRelativePos);
        } else {
            if (!previousSegment) {
                segment->setStartOffset(0.0);
            }
            if (!nextSegment) {
                segment->setEndOffset(1.0);
            }
        }
        emit selectedHandleChanged();
        emit updateRequested();
    } else if (handle.type == HandleType_Stop) {
        KoGradientSegment *previousSegment = handle.index == 0 ? nullptr : m_gradient->segments()[handle.index - 1];
        KoGradientSegment *nextSegment = handle.index == m_gradient->segments().size() ? nullptr : m_gradient->segments()[handle.index];
        if (previousSegment && nextSegment) {
            const qreal previousMidPointLocalPos =
                previousSegment->length() > std::numeric_limits<qreal>::epsilon()
                ? (previousSegment->middleOffset() - previousSegment->startOffset()) / previousSegment->length()
                : 0.0;
            const qreal nextMidPointLocalPos =
                nextSegment->length() > std::numeric_limits<qreal>::epsilon()
                ? (nextSegment->middleOffset() - nextSegment->startOffset()) / nextSegment->length()
                : 0.0;
            qreal newOffset;
            if (distance < 0) {
                newOffset = qMax(previousSegment->endOffset() + distance, previousSegment->startOffset() + epsilon);
            } else {
                newOffset = qMin(previousSegment->endOffset() + distance, nextSegment->endOffset() - epsilon);
            }
            previousSegment->setEndOffset(newOffset);
            nextSegment->setStartOffset(newOffset);
            previousSegment->setMiddleOffset(
                previousSegment->startOffset() +
                previousMidPointLocalPos * previousSegment->length()
            );
            nextSegment->setMiddleOffset(
                nextSegment->startOffset() +
                nextMidPointLocalPos * nextSegment->length()
            );
        } else {
            if (!previousSegment) {
                nextSegment->setStartOffset(0.0);
            }
            if (!nextSegment) {
                previousSegment->setEndOffset(1.0);
            }
        }
        emit selectedHandleChanged();
        emit updateRequested();
    } else if (handle.type == HandleType_MidPoint) {
        KoGradientSegment *segment = m_gradient->segments()[handle.index];
        segment->setMiddleOffset(qBound(segment->startOffset(), segment->middleOffset() + distance, segment->endOffset()));
        emit selectedHandleChanged();
        emit updateRequested();
    }
}

void KisSegmentGradientSlider::moveHandleLeft(Handle handle, qreal distance, bool useShrinkEpsilon)
{
    moveHandle(handle, -distance, useShrinkEpsilon);
}

void KisSegmentGradientSlider::moveHandleRight(Handle handle, qreal distance, bool useShrinkEpsilon)
{
    moveHandle(handle, distance, useShrinkEpsilon);
}

void KisSegmentGradientSlider::moveSelectedHandle(qreal distance, bool useShrinkEpsilon)
{
    moveHandle(m_selectedHandle, distance, useShrinkEpsilon);
}

void KisSegmentGradientSlider::moveSelectedHandleLeft(qreal distance, bool useShrinkEpsilon)
{
    moveSelectedHandle(-distance, useShrinkEpsilon);
}

void KisSegmentGradientSlider::moveSelectedHandleRight(qreal distance, bool useShrinkEpsilon)
{
    moveSelectedHandle(distance, useShrinkEpsilon);
}

bool KisSegmentGradientSlider::deleteHandleImpl(Handle handle)
{
    if (handle.type == HandleType_Segment) {
        if (m_gradient->removeSegment(m_gradient->segments()[handle.index])) {
            if (m_selectedHandle.index > 0) {
                --m_selectedHandle.index;
            }
            return true;
        }
    } else if (m_selectedHandle.type == HandleType_Stop) {
        if (m_selectedHandle.index <= 0 || m_selectedHandle.index >= m_gradient->segments().size()) {
            return false;
        }
        KoGradientSegment *previousSegment = m_gradient->segments()[m_selectedHandle.index - 1];
        KoGradientSegment *nextSegment = m_gradient->segments()[m_selectedHandle.index];
        const qreal middleOffset = previousSegment->endOffset();
        previousSegment->setEndType(nextSegment->endType());
        previousSegment->setEndColor(nextSegment->endColor());
        m_gradient->removeSegment(nextSegment);
        previousSegment->setMiddleOffset(middleOffset);
        m_selectedHandle.type = HandleType_Segment;
        m_selectedHandle.index = m_selectedHandle.index - 1;
        return true;
    }
    return false;
}

void KisSegmentGradientSlider::deleteHandle(Handle handle)
{
    if (deleteHandleImpl(handle)) {
        emit selectedHandleChanged();
        emit updateRequested();
    }
}

void KisSegmentGradientSlider::deleteSelectedHandle()
{
    deleteHandle(m_selectedHandle);
}

void KisSegmentGradientSlider::collapseSelectedSegment()
{
    if (m_selectedHandle.type != HandleType_Segment) {
        return;
    }
    if (m_gradient->collapseSegment(m_gradient->segments()[m_selectedHandle.index])) {
        emit selectedHandleChanged();
        emit updateRequested();
    }
}

void KisSegmentGradientSlider::centerSelectedHandle() 
{
    if (m_selectedHandle.type == HandleType_Segment) {
        KoGradientSegment *segment = m_gradient->segments()[m_selectedHandle.index];
        KoGradientSegment *previousSegment = m_selectedHandle.index == 0 ? nullptr : m_gradient->segments()[m_selectedHandle.index - 1];
        KoGradientSegment *nextSegment = m_selectedHandle.index == m_gradient->segments().size() - 1 ? nullptr : m_gradient->segments()[m_selectedHandle.index + 1];
        if (previousSegment && nextSegment) {
            moveSelectedHandle(
                (previousSegment->startOffset() + nextSegment->endOffset()) / 2.0 -
                (segment->startOffset() + segment->endOffset()) / 2.0);
        }
    } else if (m_selectedHandle.type == HandleType_Stop) {
        KoGradientSegment *previousSegment = m_selectedHandle.index == 0 ? nullptr : m_gradient->segments()[m_selectedHandle.index - 1];
        KoGradientSegment *nextSegment = m_selectedHandle.index == m_gradient->segments().size() ? nullptr : m_gradient->segments()[m_selectedHandle.index];
        if (previousSegment && nextSegment) {
            moveSelectedHandle((previousSegment->startOffset() + nextSegment->endOffset()) / 2.0 - nextSegment->startOffset());
        }
    } else if (m_selectedHandle.type == HandleType_MidPoint) {
        KoGradientSegment *segment = m_gradient->segments()[m_selectedHandle.index];
        qDebug() << segment->startOffset() << segment->endOffset() << segment->middleOffset() <<
                ((segment->startOffset() + segment->endOffset()) / 2);
        moveSelectedHandle((segment->startOffset() + segment->endOffset()) / 2.0 - segment->middleOffset());
    }
}

void KisSegmentGradientSlider::splitSelectedSegment()
{
    if (m_selectedHandle.type != HandleType_Segment) {
        return;
    }
    m_gradient->splitSegment(m_gradient->segments()[m_selectedHandle.index]);
    emit selectedHandleChanged();
    emit updateRequested();
}

void KisSegmentGradientSlider::duplicateSelectedSegment()
{
    if (m_selectedHandle.type != HandleType_Segment) {
        return;
    }
    m_gradient->duplicateSegment(m_gradient->segments()[m_selectedHandle.index]);
    emit selectedHandleChanged();
    emit updateRequested();
}

void KisSegmentGradientSlider::mirrorSelectedSegment()
{
    if (m_selectedHandle.type != HandleType_Segment) {
        return;
    }
    m_gradient->mirrorSegment(m_gradient->segments()[m_selectedHandle.index]);
    emit selectedHandleChanged();
    emit updateRequested();
}

void KisSegmentGradientSlider::flipGradient()
{
    QList<KoGradientSegment*> oldSegments = m_gradient->segments();
    QList<KoGradientSegment*> newSegments;
    for (int i = oldSegments.size() - 1; i >= 0; --i) {
        KoGradientSegment* oldSegment = oldSegments[i];
        int interpolation = oldSegment->interpolation();
        int colorInterpolation = oldSegment->colorInterpolation();

        if (interpolation == INTERP_SPHERE_INCREASING) {
            interpolation = INTERP_SPHERE_DECREASING;
        }
        else if (interpolation == INTERP_SPHERE_DECREASING) {
            interpolation = INTERP_SPHERE_INCREASING;
        }
        if (colorInterpolation == COLOR_INTERP_HSV_CW) {
            colorInterpolation = COLOR_INTERP_HSV_CCW;
        }
        else if (colorInterpolation == COLOR_INTERP_HSV_CCW) {
            colorInterpolation = COLOR_INTERP_HSV_CW;
        }

        KoGradientSegment* newSegment = new KoGradientSegment(
            interpolation, colorInterpolation,
            { 1.0 - oldSegment->endOffset(), oldSegment->endColor(), oldSegment->endType() },
            { 1.0 - oldSegment->startOffset(), oldSegment->startColor(), oldSegment->startType() },
            1.0 - oldSegment->middleOffset()
        );

        newSegments.push_back(newSegment);
    }
    m_gradient->setSegments(newSegments);
    if (m_selectedHandle.type == HandleType_Stop) {
        m_selectedHandle.index = newSegments.size() - m_selectedHandle.index;
    } else {
        m_selectedHandle.index = newSegments.size() - 1 - m_selectedHandle.index;
    }
    emit selectedHandleChanged();
    emit updateRequested();
}

void KisSegmentGradientSlider::distributeStopsEvenly()
{
    const qreal size = 1.0 / m_gradient->segments().size();
    for (int i = 0; i < m_gradient->segments().size(); ++i) {
        KoGradientSegment *segment = m_gradient->segments()[i];
        const qreal relativeMidPointPosition =
            (segment->middleOffset() - segment->startOffset()) /
            (segment->endOffset() - segment->startOffset());
        segment->setStartOffset(i * size);
        segment->setEndOffset((i + 1) * size);
        segment->setMiddleOffset(
            segment->startOffset() + relativeMidPointPosition *
            (segment->endOffset() - segment->startOffset()));
    }
    emit selectedHandleChanged();
    emit updateRequested();
}

QRect KisSegmentGradientSlider::sliderRect() const
{
    const qreal handleWidthOverTwo = static_cast<qreal>(m_handleSize.width()) / 2.0;
    const int hMargin = static_cast<int>(std::ceil(handleWidthOverTwo)) + 2;
    return rect().adjusted(hMargin, 0, -hMargin, 0);
}

QRect KisSegmentGradientSlider::gradientStripeRect() const
{
    const QRect rc = sliderRect();
    return rc.adjusted(0, 0, 0, -m_handleSize.height() - 4);
}

QRect KisSegmentGradientSlider::handlesStripeRect() const
{
    const QRect rc = sliderRect();
    return rc.adjusted(0, rc.height() - (m_handleSize.height() + 2), 0, -2);
}

void KisSegmentGradientSlider::updateHandleSize()
{
    QFontMetrics fm(font());
    const int h = qMax(15, static_cast<int>(std::ceil(fm.height() * 0.75)));
    m_handleSize = QSize(h * 0.75, h);
}

int KisSegmentGradientSlider::minimalHeight() const
{
    QFontMetrics fm(font());
    const int h = fm.height();

    QStyleOptionToolButton opt;
    QSize sz = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(h, h), this);

    return qMax(32, sz.height()) + m_handleSize.height();
}

QSize KisSegmentGradientSlider::sizeHint() const
{
    const int h = minimalHeight();
    return QSize(2 * h, h);
}

QSize KisSegmentGradientSlider::minimumSizeHint() const
{
    const int h = minimalHeight();
    return QSize(h, h);
}

void KisSegmentGradientSlider::chooseSelectedStopColor()
{
    if (m_selectedHandle.type != HandleType_Stop) {
        return;
    }
    QList<KoGradientSegment*> segments = m_gradient->segments();
    if (m_selectedHandle.index < 0 || m_selectedHandle.index > segments.size()) {
        return;
    }

    KoColor color1, color2;
    KoGradientSegmentEndpointType endType1{COLOR_ENDPOINT}, endType2{COLOR_ENDPOINT};
    if (m_selectedHandle.index == 0) {
        endType1 = segments[0]->startType();
        color1 = segments[0]->startColor();
    } else {
        endType1 = segments[m_selectedHandle.index - 1]->endType();
        color1 = segments[m_selectedHandle.index - 1]->endColor();
        if (m_selectedHandle.index < segments.size()) {
            endType2 = segments[m_selectedHandle.index]->startType();
            color2 = segments[m_selectedHandle.index]->startColor();
        }
    }

#ifndef Q_OS_MACOS
    KisDlgInternalColorSelector::Config cfg;
    KisDlgInternalColorSelector *dialog = new KisDlgInternalColorSelector(this, color1, cfg, i18n("Choose a color"));
    dialog->setPreviousColor(color1);
    auto setColorFn = [dialog, segments, this]() mutable
                      {
                          if (m_selectedHandle.index == 0) {
                              segments[0]->setStartType(COLOR_ENDPOINT);
                              segments[0]->setStartColor(dialog->getCurrentColor());
                          } else {
                              segments[m_selectedHandle.index - 1]->setEndType(COLOR_ENDPOINT);
                              segments[m_selectedHandle.index - 1]->setEndColor(dialog->getCurrentColor());
                              if (m_selectedHandle.index < segments.size()) {
                                  segments[m_selectedHandle.index]->setStartType(COLOR_ENDPOINT);
                                  segments[m_selectedHandle.index]->setStartColor(dialog->getCurrentColor());
                              }
                          }
                          emit selectedHandleChanged();
                          emit updateRequested();
                      };
    connect(dialog, &KisDlgInternalColorSelector::signalForegroundColorChosen, setColorFn);
#else
    QColorDialog *dialog = new QColorDialog(this);
    dialog->setCurrentColor(color1.toQColor());
    auto setColorFn = [dialog, segments, this]() mutable
                      {
                          KoColor color;
                          color.fromQColor(dialog->currentColor());
                          if (m_selectedHandle.index == 0) {
                              segments[0]->setStartType(COLOR_ENDPOINT);
                              segments[0]->setStartColor(color);
                          } else {
                              segments[m_selectedHandle.index - 1]->setEndType(COLOR_ENDPOINT);
                              segments[m_selectedHandle.index - 1]->setEndColor(color);
                              if (m_selectedHandle.index < segments.size()) {
                                  segments[m_selectedHandle.index]->setStartType(COLOR_ENDPOINT);
                                  segments[m_selectedHandle.index]->setStartColor(color);
                              }
                          }
                          emit selectedHandleChanged();
                          emit updateRequested();
                      };
    connect(dialog, &QColorDialog::currentColorChanged, setColorFn);
#endif
    connect(dialog, &QDialog::accepted, setColorFn);
    connect(dialog, &QDialog::rejected, [endType1, endType2, color1, color2, segments, this]()
                                        {
                                            if (m_selectedHandle.index == 0) {
                                                segments[0]->setStartType(endType1);
                                                segments[0]->setStartColor(color1);
                                            } else {
                                                segments[m_selectedHandle.index - 1]->setEndType(endType1);
                                                segments[m_selectedHandle.index - 1]->setEndColor(color1);
                                                if (m_selectedHandle.index < segments.size()) {
                                                    segments[m_selectedHandle.index]->setStartType(endType2);
                                                    segments[m_selectedHandle.index]->setStartColor(color2);
                                                }
                                            }
                                            emit selectedHandleChanged();
                                            emit updateRequested();
                                        });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}
