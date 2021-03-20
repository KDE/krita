/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SIMPLE_UPDATE_QUEUE_TEST_H
#define KIS_SIMPLE_UPDATE_QUEUE_TEST_H

#include <simpletest.h>


class KisSimpleUpdateQueueTest : public QObject
{
    Q_OBJECT

private:
    void testSplit(bool useFullRefresh);

private Q_SLOTS:
    void testJobProcessing();
    void testSplitUpdate();
    void testSplitFullRefresh();
    void testChecksum();
    void testMixingTypes();
    void testSpontaneousJobsCompression();
};

#endif /* KIS_SIMPLE_UPDATE_QUEUE_TEST_H */

