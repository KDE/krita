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

#include "KisSignalAutoConnectionTest.h"

#include <kis_signal_auto_connection.h>

void KisSignalAutoConnectionTest::testMacroConnection()
{
    QScopedPointer<TestClass> test1(new TestClass());
    QScopedPointer<TestClass> test2(new TestClass());
    KisSignalAutoConnectionsStore conn;
    conn.addConnection(test1.data(), SIGNAL(sigTest1()), test2.data(), SLOT(slotTest1()));
    emit test1->sigTest1();
    QVERIFY(test2->m_test1Called);
    test2->m_test1Called = false;
    conn.clear();
    emit test1->sigTest1();
    QVERIFY(test2->m_test1Called == false);
}

void KisSignalAutoConnectionTest::testMemberFunctionConnection()
{
    QScopedPointer<TestClass> test1(new TestClass());
    QScopedPointer<TestClass> test2(new TestClass());
    KisSignalAutoConnectionsStore conn;
    conn.addConnection(test1.data(), &TestClass::sigTest1, test2.data(), &TestClass::slotTest1);
    emit test1->sigTest1();
    QVERIFY(test2->m_test1Called);
    test2->m_test1Called = false;
    conn.clear();
    emit test1->sigTest1();
    QVERIFY(test2->m_test1Called == false);
}

void KisSignalAutoConnectionTest::testOverloadConnection()
{
    QScopedPointer<TestClass> test1(new TestClass());
    QScopedPointer<TestClass> test2(new TestClass());
    KisSignalAutoConnectionsStore conn;
    conn.addConnection(test1.data(), QOverload<const QString &, const QString &>::of(&TestClass::sigTest2),
                       test2.data(), QOverload<const QString &, const QString &>::of(&TestClass::slotTest2));
    conn.addConnection(test1.data(), SIGNAL(sigTest2(int)), test2.data(), SLOT(slotTest2(int)));
    emit test1->sigTest2("foo", "bar");
    QVERIFY(test2->m_str1 == "foo");
    QVERIFY(test2->m_str2 == "bar");
    emit test1->sigTest2(5);
    QVERIFY(test2->m_number == 5);
    conn.clear();
    emit test1->sigTest2("1", "2");
    QVERIFY(test2->m_str1 == "foo");
    QVERIFY(test2->m_str2 == "bar");
    conn.addConnection(test1.data(), SIGNAL(sigTest2(const QString &, const QString &)),
                       test2.data(), SLOT(slotTest2(const QString &)));
    emit test1->sigTest2("3", "4");
    QVERIFY(test2->m_str1 == "3");
    QVERIFY(test2->m_str2 == "");
}

void KisSignalAutoConnectionTest::testSignalToSignalConnection()
{
    QScopedPointer<TestClass> test1(new TestClass());
    QScopedPointer<TestClass> test2(new TestClass());
    KisSignalAutoConnectionsStore conn;
    conn.addConnection(test1.data(), QOverload<int>::of(&TestClass::sigTest2),
                       test2.data(), QOverload<int>::of(&TestClass::sigTest2));
    conn.addConnection(test2.data(), SIGNAL(sigTest2(int)), test2.data(), SLOT(slotTest2(int)));
    emit test1->sigTest2(10);
    QVERIFY(test2->m_number == 10);
    conn.clear();
    conn.addConnection(test1.data(), SIGNAL(sigTest2(int)), test2.data(), SIGNAL(sigTest2(int)));
    conn.addConnection(test2.data(), QOverload<int>::of(&TestClass::sigTest2),
                       test2.data(), QOverload<int>::of(&TestClass::slotTest2));
    emit test1->sigTest2(50);
    QVERIFY(test2->m_number == 50);
}

void KisSignalAutoConnectionTest::testDestroyedObject()
{
    QScopedPointer<TestClass> test1(new TestClass());
    QScopedPointer<TestClass> test2(new TestClass());
    KisSignalAutoConnectionsStore conn;
    conn.addConnection(test1.data(), QOverload<int>::of(&TestClass::sigTest2),
                       test2.data(), QOverload<int>::of(&TestClass::slotTest2));
    emit test1->sigTest2(10);
    QVERIFY(test2->m_number == 10);
    test2.reset(0);
    conn.clear();
}

TestClass::TestClass(QObject *parent)
    : QObject(parent)
    , m_test1Called(false)
    , m_str1()
    , m_str2()
    , m_number(0)
{
}

TestClass::~TestClass()
{
}

void TestClass::slotTest1()
{
    m_test1Called = true;
}

void TestClass::slotTest2(const QString &arg1, const QString &arg2)
{
    m_str1 = arg1;
    m_str2 = arg2;
}

void TestClass::slotTest2(const QString &arg)
{
    m_str1 = arg;
    m_str2 = QString();
}

void TestClass::slotTest2(int arg)
{
    m_number = arg;
}

QTEST_MAIN(KisSignalAutoConnectionTest)
