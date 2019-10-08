/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisSignalCompressorTest.h"

#include "QTimer"
#include "kis_signal_compressor.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include "kis_debug.h"

struct CompressorTester : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void start() {
        if (!m_timer.isValid()) {
            m_timer.start();
        } else {
            m_acc(m_timer.restart());
        }
    }
public:
    void dump(const QString &testName) {
        qDebug() << testName
                 << "cnt:" << boost::accumulators::count(m_acc)
                 << "min:" << boost::accumulators::min(m_acc)
                 << "max:" << boost::accumulators::max(m_acc)
                 << "mean:" << boost::accumulators::mean(m_acc)
                 << "var:" << std::sqrt(boost::accumulators::variance(m_acc));
    }

private:
    typedef boost::accumulators::stats<
        boost::accumulators::tag::min,
        boost::accumulators::tag::max,
        boost::accumulators::tag::mean,
        boost::accumulators::tag::variance> stats;

    boost::accumulators::accumulator_set<qreal, stats> m_acc;
    QElapsedTimer m_timer;
};

void testCompression(int timerInterval, int compressorInterval)
{
    CompressorTester tester;
    KisSignalCompressor compressor(compressorInterval, KisSignalCompressor::FIRST_ACTIVE);
    QTimer timer;
    timer.setInterval(timerInterval);
    timer.setTimerType(Qt::PreciseTimer);
    timer.setSingleShot(false);

    QObject::connect(&timer, SIGNAL(timeout()), &compressor, SLOT(start()));
    QObject::connect(&compressor, SIGNAL(timeout()), &tester, SLOT(start()));

    timer.start();

    QTest::qWait(500);


    timer.stop();
    QTest::qWait(compressorInterval * 2);
    compressor.stop();

    tester.dump(QString("timer %1 compressor %2").arg(timerInterval).arg(compressorInterval));

    QTest::qWait(compressorInterval * 10);
}

void KisSignalCompressorTest::test()
{
    for (int i = 10; i < 50; i++) {
        testCompression(i, 25);
    }
    //testCompression(10, 25);
}

QTEST_MAIN(KisSignalCompressorTest)

#include "KisSignalCompressorTest.moc"
