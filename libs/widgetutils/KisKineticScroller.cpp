/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2018 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 * SPDX-FileCopyrightText: 2018 Eoin O 'Neill <eoinoneill1991@gmail.com>
 * SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KisKineticScroller.h>

#include <QAbstractItemView>
#include <QEvent>
#include <QScrollBar>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

namespace {
    /**
     * Event filter to be installed on the scroll bars of QAbstractScrollArea,
     * used to detect cursor entering/leaving the scrollbars in order to
     * temporarily disable/re-enable the QScroller. This allows the scroll bars
     * to be dragged normally even when kinetic scrolling is enabled.
     */
    class KisKineticScrollerEventFilter : public QObject
    {
        Q_OBJECT

        QAbstractScrollArea *m_scrollArea;
        QScroller::ScrollerGestureType m_gestureType;

    public:
        KisKineticScrollerEventFilter(QScroller::ScrollerGestureType gestureType, QAbstractScrollArea *parent)
            : QObject(parent)
            , m_scrollArea(parent)
            , m_gestureType(gestureType)
        {
        }

    protected:
        bool eventFilter(QObject *watched, QEvent *event) override {
            switch (event->type()) {
            case QEvent::Enter:
                QScroller::ungrabGesture(m_scrollArea);
                break;
            case QEvent::Leave:
                QScroller::grabGesture(m_scrollArea, m_gestureType);
                break;
            default:
                break;
            }
            return QObject::eventFilter(watched, event);
        }
    };
} /* namespace */

QScroller* KisKineticScroller::createPreconfiguredScroller(QAbstractScrollArea *scrollArea) {
    KConfigGroup config = KSharedConfig::openConfig()->group("");
    int sensitivity = config.readEntry("KineticScrollingSensitivity", 75);
    bool enabled = config.readEntry("KineticScrollingEnabled", true);
    bool hideScrollBars = config.readEntry("KineticScrollingHideScrollbar", false);
    float resistanceCoefficient = config.readEntry("KineticScrollingResistanceCoefficient", 10.0f);
    float dragVelocitySmoothFactor = config.readEntry("KineticScrollingDragVelocitySmoothingFactor", 1.0f);
    float minimumVelocity = config.readEntry("KineticScrollingMinimumVelocity", 0.0f);
    float axisLockThresh = config.readEntry("KineticScrollingAxisLockThreshold", 1.0f);
    float maximumClickThroughVelocity = config.readEntry("KineticScrollingMaxClickThroughVelocity", 0.0f);
    float flickAccelerationFactor = config.readEntry("KineticScrollingFlickAccelerationFactor", 1.5f);
    float overshootDragResistanceFactor = config.readEntry("KineticScrollingOvershotDragResistanceFactor", 0.1f);
    float overshootDragDistanceFactor = config.readEntry("KineticScrollingOvershootDragDistanceFactor", 0.3f);
    float overshootScrollDistanceFactor = config.readEntry("KineticScrollingOvershootScrollDistanceFactor", 0.1f);
    float overshootScrollTime = config.readEntry("KineticScrollingOvershootScrollTime", 0.4f);
    QScroller::ScrollerGestureType gestureType = getConfiguredGestureType();

    if (enabled && scrollArea) {
        if (hideScrollBars) {
            scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
            scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        } else {
            auto *filter = new KisKineticScrollerEventFilter(gestureType, scrollArea);
            scrollArea->horizontalScrollBar()->installEventFilter(filter);
            scrollArea->verticalScrollBar()->installEventFilter(filter);
        }

        QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>(scrollArea);
        if (itemView) {
            itemView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        }

        QScroller *scroller = QScroller::scroller(scrollArea);
        QScroller::grabGesture(scrollArea, gestureType);

        QScrollerProperties properties;

        // DragStartDistance seems to be based on meter per second; though it's
        // not explicitly documented, other QScroller values are in that metric.
        // To start kinetic scrolling, with minimal sensitity, we expect a drag
        // of 10 mm, with minimum sensitity any > 0 mm.
        const float mm = 0.001f;
        const float resistance = 1.0f - (sensitivity / 100.0f);
        const float mousePressEventDelay = config.readEntry("KineticScrollingMousePressDelay", 1.0f - 0.75f * resistance);

        properties.setScrollMetric(QScrollerProperties::DragStartDistance, resistance * resistanceCoefficient * mm);
        properties.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, dragVelocitySmoothFactor);
        properties.setScrollMetric(QScrollerProperties::MinimumVelocity, minimumVelocity);
        properties.setScrollMetric(QScrollerProperties::AxisLockThreshold, axisLockThresh);
        properties.setScrollMetric(QScrollerProperties::MaximumClickThroughVelocity, maximumClickThroughVelocity);
        properties.setScrollMetric(QScrollerProperties::MousePressEventDelay, mousePressEventDelay);
        properties.setScrollMetric(QScrollerProperties::AcceleratingFlickSpeedupFactor, flickAccelerationFactor);

        properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOn);
        properties.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, overshootDragResistanceFactor);
        properties.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, overshootDragDistanceFactor);
        properties.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, overshootScrollDistanceFactor);
        properties.setScrollMetric(QScrollerProperties::OvershootScrollTime, overshootScrollTime);

        scroller->setScrollerProperties(properties);

        return scroller;
    }

    return nullptr;
}

QScroller::ScrollerGestureType KisKineticScroller::getConfiguredGestureType() {
    KConfigGroup config = KSharedConfig::openConfig()->group("");
#ifdef Q_OS_ANDROID
    // Use a different default. Shouldn't we use KisConfig::kineticScrollingGesture?
    int gesture = config.readEntry("KineticScrollingGesture", 1);
#else
    int gesture = config.readEntry("KineticScrollingGesture", 0);
#endif

    switch (gesture) {
    case 0: {
        return QScroller::TouchGesture;
    }
    case 1: {
        return QScroller::LeftMouseButtonGesture;
    }
    case 2: {
        return QScroller::MiddleMouseButtonGesture;
    }
    case 3: {
        return QScroller::RightMouseButtonGesture;
    }
    default:
        return QScroller::MiddleMouseButtonGesture;
    }
}

void KisKineticScroller::updateCursor(QWidget *source, QScroller::State state) {
    if( state == QScroller::State::Pressed ) {
        source->setCursor(Qt::OpenHandCursor);
    } else if (state == QScroller::State::Dragging) {
        source->setCursor(Qt::ClosedHandCursor);
    } else {
        source->setCursor(Qt::ArrowCursor);
    }
}

#include "KisKineticScroller.moc"
