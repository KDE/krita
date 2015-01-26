/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TABLET_EVENT_RATE_LOGGER_H
#define __KIS_TABLET_EVENT_RATE_LOGGER_H


template <>
inline QPoint qAbs<QPoint>(const QPoint &pt)
{
    return QPoint(qAbs(pt.x()), qAbs(pt.y()));
}

template <>
inline QPointF qAbs<QPointF>(const QPointF &pt)
{
    return QPointF(qAbs(pt.x()), qAbs(pt.y()));
}


class KisTabletEventRateLogger
{
public:
    KisTabletEventRateLogger(int period, const QString &id)
        : m_period(period),
          m_numSamples(0),
          m_avgTime(0),
          m_id(id)
    {
    }


    void logTabletEvent(KisTabletEvent *tevent) {
        QPointF newPos = tevent->hiResGlobalPos();

        if (!m_numSamples) {
            m_numSamples++;
            m_time.start();
            m_lastPos = newPos;
            return;
        }


        qreal oldCoeff = m_numSamples;
        qreal newCoeff = ++m_numSamples;

        m_avgTime = (m_avgTime * oldCoeff + m_time.restart()) / newCoeff;

        QPointF posDiff = qAbs(newPos - m_lastPos);
        m_avgPosDiff = (m_avgPosDiff * oldCoeff + posDiff) / newCoeff;

        m_lastPos = newPos;

        if (m_numSamples % m_period == 0) {
            dumpValues();
        }
    }

    void dumpValues() {
        qDebug() << "########################";
        qDebug() << ppVar(m_id);
        qDebug() << ppVar(m_avgTime);
        qDebug() << ppVar(m_avgPosDiff);
        qDebug() << "spd:" << m_avgPosDiff / m_avgTime;
        qDebug() << "########################";
    }

private:
    int m_period;
    int m_numSamples;

    qreal m_avgTime;
    QPointF m_avgPosDiff;
    QPointF m_lastPos;

    QTime m_time;
    QString m_id;
};

#endif /* __KIS_TABLET_EVENT_RATE_LOGGER_H */
