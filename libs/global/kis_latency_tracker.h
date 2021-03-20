/*
 *  SPDX-FileCopyrightText: 2017 Bernhard Liebl <poke1024@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_KIS_SCALAR_TRACKER_H
#define KRITA_KIS_SCALAR_TRACKER_H

#include "kis_shared.h"
#include <kritaglobal_export.h>

#include <QQueue>
#include <QElapsedTimer>
#include <QDebug>

#include <boost/heap/fibonacci_heap.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/rolling_variance.hpp>

template<typename T>
class KisRollingMax {
public:
    KisRollingMax(int windowSize) : m_windowSize(windowSize) {
    }

    void push(T value) {
        while (m_samples.size() > m_windowSize) {
            m_values.erase(m_samples.dequeue());
        }

        m_samples.enqueue(m_values.push(value));
    }

    T max() const {
        if (m_values.empty()) {
            throw std::runtime_error("no values to get max of");
        } else {
            return m_values.top();
        }
    }

private:
    const int m_windowSize;

    typedef boost::heap::fibonacci_heap<T> heap_type;

    QQueue<typename heap_type::handle_type> m_samples;
    heap_type m_values;
};

template<typename T>
class KisScalarTracker : public KisShared {
public:
    /**
     * Create a tracker with the given window size.
     * @param window The maximum number of elements to take into account for calculation
     * of max, mean and variance values.
     */
    KisScalarTracker(const QString &name, int windowSize = 500) :
        m_name(name),
        m_windowSize(windowSize),
        m_addCount(0),
        m_max(windowSize),
        m_acc(boost::accumulators::tag::rolling_window::window_size = windowSize)
    {
        m_printTimer.start();
    }

    virtual ~KisScalarTracker() {
    }

    /**
     * Add a scalar.
     * @param value the scalar to be added.
     */
    virtual void push(T value) {
        m_max.push(value);
        m_acc(value);
        m_addCount++;

        if (m_addCount >= m_windowSize || m_printTimer.elapsed() >= 1000) {
            m_printTimer.restart();
            QString s = format(boost::accumulators::rolling_mean(m_acc),
                  boost::accumulators::rolling_variance(m_acc),
                  m_max.max());
            print(s);
            m_addCount = 0;
        }

    }

protected:
    /**
     * Print out a message.
     * @param message the message to print
     */
    virtual void print(const QString &message) {
        qInfo() << qUtf8Printable(message);
    }

    /**
     * Formats a message for printing.
     * @param mean the mean scalar in the window
     * @param variance the variance of the scalar in the window
     * @param max the max scalar in the window
     */
    virtual QString format(qint64 mean, qint64 variance, qint64 max) {
        return QString("%1: mean %2 ms, var %3, max %4 ms").arg(m_name).arg(mean).arg(variance).arg(max);
    }

private:
    const QString m_name;
    const int m_windowSize;
    int m_addCount;

    QElapsedTimer m_printTimer;

    KisRollingMax<T> m_max;

    // see https://svn.boost.org/trac10/ticket/11437
    typedef boost::accumulators::stats<
        boost::accumulators::tag::lazy_rolling_mean,
        boost::accumulators::tag::rolling_variance> stats;

    boost::accumulators::accumulator_set<T, stats> m_acc;
};

/**
 * KisLatencyTracker tracks the time it takes events to reach a certain point in the program.
 */

class KRITAGLOBAL_EXPORT KisLatencyTracker : public KisScalarTracker<qint64> {
public:
    /**
     * Create a tracker with the given window size.
     * @param window The maximum number of elements to take into account for calculation
     * of max, mean and variance values.
     */
    KisLatencyTracker(int windowSize = 500);

    /**
     * Register that an event with the given timestamp has arrived just now.
     * @param timestamp Timestamp of the event that just arrived (the difference to the
     * current time is the latency).
     */
    virtual void push(qint64 timestamp) override;

protected:
    /**
     * @return The timestamp of "right now" in a frame that is comparable to those
     * timestamps given to push().
     */
    virtual qint64 currentTimestamp() const = 0;
};

#endif //KRITA_KIS_SCALAR_TRACKER_H
