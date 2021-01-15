/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

public:
    CompressorTester(int handlerDelay)
        : m_handlerDelay(handlerDelay)
    {
    }

public Q_SLOTS:
    void start() {
        if (!m_timer.isValid()) {
            m_timer.start();
        } else {
            m_acc(m_timer.restart());
        }

        if (m_handlerDelay > 0) {
            QTest::qSleep(m_handlerDelay);
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
    int m_handlerDelay;
};

void testCompression(int timerInterval, int compressorInterval,
                     int handlerDelay = 0,
                     KisSignalCompressor::SlowHandlerMode slowHandlerMode = KisSignalCompressor::PRECISE_INTERVAL)
{
    CompressorTester tester(handlerDelay);
    KisSignalCompressor compressor(compressorInterval,
                                   KisSignalCompressor::FIRST_ACTIVE,
                                   slowHandlerMode);
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

    tester.dump(QString("timer %1 compressor %2 handler delay %3")
                .arg(timerInterval).arg(compressorInterval).arg(handlerDelay));

    QTest::qWait(compressorInterval * 10);
}

void KisSignalCompressorTest::test()
{
    for (int i = 10; i < 50; i++) {
        testCompression(i, 25);
    }
    //testCompression(10, 25);
}

void KisSignalCompressorTest::testSlowHandlerPrecise()
{
    for (int i = 0; i < 31; i++) {
        testCompression(6, 10, i, KisSignalCompressor::PRECISE_INTERVAL);
    }
}

void KisSignalCompressorTest::testSlowHandlerAdditive()
{
    for (int i = 0; i < 31; i++) {
        testCompression(6, 10, i, KisSignalCompressor::ADDITIVE_INTERVAL);
    }
}

void testIdleChecksImpl(int compressorInterval,
                        int timerInterval,
                        int idleCheckInterval,
                        int idleDelay)
{
    const int handlerDelay = 0;

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    CompressorTester tester(handlerDelay);
    KisSignalCompressor compressor(compressorInterval,
                                   KisSignalCompressor::FIRST_ACTIVE,
                                   KisSignalCompressor::PRECISE_INTERVAL);

    compressor.setDelay(
        [idleDelay, &elapsedTimer]() {
            return elapsedTimer.elapsed() > idleDelay;
        },
        idleCheckInterval,
        compressorInterval);

    QTimer timer;
    timer.setInterval(timerInterval);
    timer.setTimerType(Qt::PreciseTimer);
    timer.setSingleShot(false);

    QObject::connect(&timer, SIGNAL(timeout()), &compressor, SLOT(start()));
    QObject::connect(&compressor, SIGNAL(timeout()), &tester, SLOT(start()));
    QObject::connect(&compressor, &KisSignalCompressor::timeout,
                     [&elapsedTimer] () { elapsedTimer.restart(); });

    timer.start();

    QTest::qWait(500);

    timer.stop();
    QTest::qWait(compressorInterval * 2);
    compressor.stop();

    tester.dump(QString("timer %1 compressor %2 idle check %3 idle delay %4")
                .arg(timerInterval).arg(compressorInterval)
                .arg(idleCheckInterval).arg(idleDelay));

    QTest::qWait(compressorInterval * 10);
}

void KisSignalCompressorTest::testIdleChecks()
{
    for (int i = 0; i < 40; i += 3) {
        testIdleChecksImpl(50, 5, 5, qMax(1, i));
    }
}

QTEST_MAIN(KisSignalCompressorTest)

#include "KisSignalCompressorTest.moc"
