/*
 *  Copyright (c) 2019 Tusooa Zhu <tusooa@vista.aero>
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

#ifndef KIS_SIGNAL_AUTO_CONNECTION_TEST_H_
#define KIS_SIGNAL_AUTO_CONNECTION_TEST_H_

#include <QtTest>

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
