/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_MEMORY_WINDOW_TEST_H
#define KIS_MEMORY_WINDOW_TEST_H

#include <QtTest>


class KisMemoryWindowTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testWindow();

private:
    // disabled since long-running
    void testTopReports();
};

#endif /* KIS_MEMORY_WINDOW_TEST_H */

