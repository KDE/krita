/*
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCANVASNAVIGATIONACTIONSTRATEGY_H
#define KISCANVASNAVIGATIONACTIONSTRATEGY_H

#include <QtGlobal>
#include <QFlags>

class QEvent;

class KisCanvasNavigationActionStrategy
{
public:
    enum Flag {
        None = 0x0,
        PanEnabled = 0x1,
        ZoomEnabled = 0x2,
        RotationEnabled = 0x4,
        ZoomDescrete = 0x8,
        RotationDescrete = 0x10,
    };
    Q_DECLARE_FLAGS(Flags, Flag)

public:
    KisCanvasNavigationActionStrategy(const Flags &flags);
    virtual ~KisCanvasNavigationActionStrategy() = default;

    virtual bool supportsEvent(QEvent* event) const = 0;
    virtual void inputEvent(QEvent* event) = 0;

    Flags flags() const;

public:
    struct SnappedRotationData {
        qreal previousAngle {0.0};
        qreal initialReferenceAngle {0.0};
        qreal accumRotationAngle {0.0};
    };

    static qreal canvasRotationAngleContinuous(qreal currentRotationOffset, qreal currectCanvasRotation, SnappedRotationData &data);
    static qreal canvasRotationAngleDescrete(qreal currentRotationOffset, SnappedRotationData &data);
private:
    Flags m_flags {None};
};



#endif // KISCANVASNAVIGATIONACTIONSTRATEGY_H
