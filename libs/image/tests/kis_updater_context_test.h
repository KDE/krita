/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_UPDATER_CONTEXT_TEST_H
#define KIS_UPDATER_CONTEXT_TEST_H

#include <simpletest.h>

class KisUpdaterContextTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testJobInterference();
    void testSnapshot();
    void stressTestExclusiveJobs();
};

#endif /* KIS_UPDATER_CONTEXT_TEST_H */

