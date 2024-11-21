/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISHISTORYLISTTEST_H
#define KISHISTORYLISTTEST_H

#include <QObject>

class KisHistoryListTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRotation();
    void testBubbleUp();
    void testSortedList();
};

#endif // KISHISTORYLISTTEST_H
