/*
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCANVASNAVIGATIONACTIONSTRATEGY_H
#define KISCANVASNAVIGATIONACTIONSTRATEGY_H

#include <QtGlobal>
class QEvent;

class KisCanvasNavigationActionStrategy
{
public:
    virtual ~KisCanvasNavigationActionStrategy() = default;

    virtual bool supportsEvent(QEvent* event) const = 0;
    virtual void inputEvent(QEvent* event) = 0;

public:
    struct SnappedRotationData {
        qreal previousAngle {0.0};
        qreal initialReferenceAngle {0.0};
        qreal accumRotationAngle {0.0};
    };

    static qreal canvasRotationAngleContinuous(qreal currentRotationOffset, qreal currectCanvasRotation, SnappedRotationData &data);
    static qreal canvasRotationAngleDescrete(qreal currentRotationOffset, SnappedRotationData &data);
};



#endif // KISCANVASNAVIGATIONACTIONSTRATEGY_H
