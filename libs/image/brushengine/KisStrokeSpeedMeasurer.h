/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISSTROKESPEEDMEASURER_H
#define KISSTROKESPEEDMEASURER_H

#include "kritaimage_export.h"
#include <QScopedPointer>

#include <QtGlobal>

class QPointF;


class KRITAIMAGE_EXPORT KisStrokeSpeedMeasurer
{
public:
    KisStrokeSpeedMeasurer(int timeSmoothWindow);
    ~KisStrokeSpeedMeasurer();

    void addSample(const QPointF &pt, int time);
    void addSamples(const QVector<QPointF> &points, int time);

    qreal averageSpeed() const;
    qreal currentSpeed() const;
    qreal maxSpeed() const;

    void reset();

private:
    void sampleMaxSpeed();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSTROKESPEEDMEASURER_H
