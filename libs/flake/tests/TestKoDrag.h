/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTKODRAG_H
#define TESTKODRAG_H

#include <QObject>
#include <QTest>

class TestKoDrag : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

#endif // TESTKODRAG_H
