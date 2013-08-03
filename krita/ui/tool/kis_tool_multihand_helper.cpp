/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tool_multihand_helper.h"

#include <QTransform>

#include "kis_painter.h"


struct KisToolMultihandHelper::Private
{
    QVector<QTransform> transformations;
};

KisToolMultihandHelper::KisToolMultihandHelper(KisPaintingInformationBuilder *infoBuilder,
                                               KisRecordingAdapter *recordingAdapter)
    : KisToolFreehandHelper(infoBuilder, recordingAdapter),
      m_d(new Private)
{
}

void KisToolMultihandHelper::setupTransformations(const QVector<QTransform> &transformations)
{
    m_d->transformations = transformations;
}

void KisToolMultihandHelper::createPainters(QVector<PainterInfo*> &painterInfos,
                                            const QPointF &lastPosition,
                                            int lastTime)
{
    for (int i = 0; i < m_d->transformations.size(); i++) {
        painterInfos <<
            new PainterInfo(new KisPainter(),
                            new KisDistanceInformation(lastPosition, lastTime));
    }
}

void KisToolMultihandHelper::paintAt(const QVector<PainterInfo*> &painterInfos,
                                     const KisPaintInformation &pi)
{
    Q_ASSERT(painterInfos.size() == m_d->transformations.size());

    for (int i = 0; i < m_d->transformations.size(); i++) {
        const QTransform &transform = m_d->transformations[i];

        KisPaintInformation __pi = pi;
        __pi.setPos(transform.map(__pi.pos()));

        paintAt(painterInfos[i], __pi);
    }
}

void KisToolMultihandHelper::paintLine(const QVector<PainterInfo*> &painterInfos,
                                       const KisPaintInformation &pi1,
                                       const KisPaintInformation &pi2)
{
    Q_ASSERT(painterInfos.size() == m_d->transformations.size());

    for (int i = 0; i < m_d->transformations.size(); i++) {
        const QTransform &transform = m_d->transformations[i];

        KisPaintInformation __pi1 = pi1;
        KisPaintInformation __pi2 = pi2;
        __pi1.setPos(transform.map(__pi1.pos()));
        __pi2.setPos(transform.map(__pi2.pos()));

        paintLine(painterInfos[i], __pi1, __pi2);
    }
}

void KisToolMultihandHelper::paintBezierCurve(const QVector<PainterInfo*> &painterInfos,
                                              const KisPaintInformation &pi1,
                                              const QPointF &control1,
                                              const QPointF &control2,
                                              const KisPaintInformation &pi2)
{
    Q_ASSERT(painterInfos.size() == m_d->transformations.size());

    for (int i = 0; i < m_d->transformations.size(); i++) {
        const QTransform &transform = m_d->transformations[i];

        KisPaintInformation __pi1 = pi1;
        KisPaintInformation __pi2 = pi2;
        __pi1.setPos(transform.map(__pi1.pos()));
        __pi2.setPos(transform.map(__pi2.pos()));

        QPointF __control1 = transform.map(control1);
        QPointF __control2 = transform.map(control2);

        paintBezierCurve(painterInfos[i], __pi1, __control1, __control2, __pi2);
    }
}
