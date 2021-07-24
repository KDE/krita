/*
 * SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TOUCH_DOCKER_QQUICKWIDGET_H
#define TOUCH_DOCKER_QQUICKWIDGET_H

#include <QtQuickWidgets/QQuickWidget>

/**
 * This is a hack on top of QQuickWidget to drop mouse events that might have
 * been synthesized by touch events. The aim is to work around a bug in the
 * Qt Windows QPA which sometimes misclassifies such events as non-synthesized
 * when handling Windows pointer input events.
 * 
 * See also:
 * - https://bugs.kde.org/show_bug.cgi?id=437196
 * - https://bugreports.qt.io/browse/QTBUG-95058
 */
class TouchDockerQQuickWidget : public QQuickWidget
{
    Q_OBJECT

    ulong m_lastTouchEventTimeMills { 0 };
    bool m_shouldBlockUntilMouseUp { false };
    bool m_shouldBlockNextDblClick { false };

public:
    TouchDockerQQuickWidget(QWidget *parent = nullptr)
        : QQuickWidget(parent)
    {
    }
    ~TouchDockerQQuickWidget() override;

    bool event(QEvent *event) override;
};

#endif // TOUCH_DOCKER_QQUICKWIDGET_H
