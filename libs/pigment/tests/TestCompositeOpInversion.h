/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTCOMPOSITEOPINVERSION_H
#define TESTCOMPOSITEOPINVERSION_H

#include <QObject>

class TestCompositeOpInversion : public QObject
{
    Q_OBJECT
public:
private Q_SLOTS:
    void test();
    void test_data();

#if 0
    void testCmyk();
    void testCmyk_data();
#endif
private:
    void test(const QString &id);
};

#endif // TESTCOMPOSITEOPINVERSION_H
