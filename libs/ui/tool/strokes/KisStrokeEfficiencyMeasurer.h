/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSTROKEEFFICIENCYMEASURER_H
#define KISSTROKEEFFICIENCYMEASURER_H

#include "kritaui_export.h"
#include <QScopedPointer>

#include <QtGlobal>

class QPointF;

class KRITAUI_EXPORT KisStrokeEfficiencyMeasurer
{
public:
    KisStrokeEfficiencyMeasurer();
    ~KisStrokeEfficiencyMeasurer();

    void setEnabled(bool value);
    bool isEnabled() const;

    void addSample(const QPointF &pt);
    void addSamples(const QVector<QPointF> &points);

    qreal averageCursorSpeed() const;
    qreal averageRenderingSpeed() const;
    qreal averageFps() const;

    void notifyRenderingStarted();
    void notifyRenderingFinished();

    void notifyCursorMoveStarted();
    void notifyCursorMoveFinished();

    void notifyFrameRenderingStarted();

    void reset();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSTROKEEFFICIENCYMEASURER_H
