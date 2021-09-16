/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SPEED_SMOOTHER_H
#define __KIS_SPEED_SMOOTHER_H

#include <QScopedPointer>

class QPointF;


class KisSpeedSmoother
{
public:
    KisSpeedSmoother();
    ~KisSpeedSmoother();

    qreal lastSpeed() const;
    qreal getNextSpeed(const QPointF &pt, ulong timestamp);
    void clear();

    void updateSettings();

private:
    qreal getNextSpeedImpl(const QPointF &pt, qreal time);
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SPEED_SMOOTHER_H */
