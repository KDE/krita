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

#ifndef __KIS_STROKE_TEST_H
#define __KIS_STROKE_TEST_H

#include <QtTest/QtTest>
#include "kis_stroke_strategy.h"
#include "kis_stroke_job.h"


class KisStrokeTest : public QObject
{
    Q_OBJECT
public:
    static inline QString getName(KisStrokeJob *job);

private slots:
    void testRegularStroke();
    void testCancelStrokeCase1();
    void testCancelStrokeCase2and3();
    void testCancelStrokeCase4();
};


class KisNoopDabStrategy : public KisDabProcessingStrategy
{
public:
KisNoopDabStrategy(QString name, bool sequential = true)
        : KisDabProcessingStrategy(sequential),
          m_name(name)
    {}

    void processDab(DabProcessingData *data) {
        Q_UNUSED(data);
    }

    QString name() {
        return m_name;
    }

private:
    QString m_name;
};

class KisTestingStrokeStrategy : public KisStrokeStrategy
{
public:
    KisTestingStrokeStrategy(const QString &prefix = QString(),
                             bool exclusive = false,
                             bool inhibitServiceJobs = false)
        : m_prefix(prefix),
          m_inhibitServiceJobs(inhibitServiceJobs)
    {
        setExclusive(exclusive);
    }

    KisDabProcessingStrategy* createInitStrategy() {
        return !m_inhibitServiceJobs ?
            new KisNoopDabStrategy(m_prefix + "init", true) : 0;
    }

    KisDabProcessingStrategy* createFinishStrategy() {
        return !m_inhibitServiceJobs ?
            new KisNoopDabStrategy(m_prefix + "finish", true) : 0;
    }

    KisDabProcessingStrategy* createCancelStrategy() {
        return !m_inhibitServiceJobs ?
            new KisNoopDabStrategy(m_prefix + "cancel", true) : 0;
    }

    KisDabProcessingStrategy* createDabStrategy() {
        return new KisNoopDabStrategy(m_prefix + "dab", false);
    }

private:
    QString m_prefix;
    bool m_inhibitServiceJobs;
};

#define SCOMPARE(s1, s2) QCOMPARE(QString(s1), QString(s2))

inline QString KisStrokeTest::getName(KisStrokeJob *job) {
    KisNoopDabStrategy *pointer =
        dynamic_cast<KisNoopDabStrategy*>(job->testingGetDabStrategy());
    Q_ASSERT(pointer);

    return pointer->name();
}

#endif /* __KIS_STROKE_TEST_H */
