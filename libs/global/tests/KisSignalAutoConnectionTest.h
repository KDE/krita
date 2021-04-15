/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SIGNAL_AUTO_CONNECTION_TEST_H_
#define KIS_SIGNAL_AUTO_CONNECTION_TEST_H_

#include <simpletest.h>

class KisSignalAutoConnectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMacroConnection();
    void testMemberFunctionConnection();
    void testOverloadConnection();
    void testSignalToSignalConnection();
    void testDestroyedObject();
};

class TestClass : public QObject
{
    Q_OBJECT
public:
    TestClass(QObject *parent = 0);
    ~TestClass() override;

Q_SIGNALS:
    void sigTest1();
    void sigTest2(const QString &arg1, const QString &arg2);
    void sigTest2(int arg);

public Q_SLOTS:
    void slotTest1();
    void slotTest2(const QString &arg1, const QString &arg2);
    void slotTest2(const QString &arg);
    void slotTest2(int arg);

public:
    bool m_test1Called;
    QString m_str1;
    QString m_str2;
    int m_number;
};

#endif // KIS_SIGNAL_AUTO_CONNECTION_TEST_H_
