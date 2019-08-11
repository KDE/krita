/* This file is part of the KDE project
 * Copyright (C) 2018 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 * Copyright (C) 2018 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KisKineticScroller.h>
#include <QAbstractItemView>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

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
        }

        QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>(scrollArea);
        if (itemView) {
            itemView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        }

        QScroller *scroller = QScroller::scroller(scrollArea);
        scroller->grabGesture(scrollArea, gestureType);

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
    int gesture = config.readEntry("KineticScrollingGesture", 0);

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
