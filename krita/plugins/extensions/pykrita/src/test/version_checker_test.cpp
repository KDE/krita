// This file is part of PyKrita, Krita' Python scripting plugin.
//
// Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) version 3.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; see the file COPYING.LIB.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#include "version_checker_test.h"
#include "../version_checker.h"

using namespace PyKrita;

void version_checker_tests::version_ctor_test()
{
    {
        version v;
        QVERIFY(v.isValid());
    }
    {
        version v(0);
        QVERIFY(v.isValid());
    }
    {
        version v(0, 0);
        QVERIFY(v.isValid());
    }
    {
        version v(0, 0, 0);
        QVERIFY(v.isValid());
    }
    {
        version v(1);
        QVERIFY(v.isValid());
    }
    {
        version v(1, 2);
        QVERIFY(v.isValid());
    }
    {
        version v(1, 2, 3);
        QVERIFY(v.isValid());
    }
}

void version_checker_tests::version_ops_test()
{
    {
        version v1;
        version v2;
        QVERIFY(v1 == v2);
    }
    {
        version v1;
        version v2(1);
        QVERIFY(v1 < v2);
        QVERIFY(v2 > v1);
        QVERIFY(v1 != v2);
        QVERIFY(v1 <= v2);
        QVERIFY(v2 >= v1);
    }
    {
        version v1(0, 0, 1);
        version v2(0, 0, 2);
        QVERIFY(v1 < v2);
        QVERIFY(v2 > v1);
        QVERIFY(v1 != v2);
        QVERIFY(v1 <= v2);
        QVERIFY(v2 >= v1);
    }
    {
        version v1(0, 0, 1);
        version v2(0, 1, 0);
        QVERIFY(v1 < v2);
        QVERIFY(v2 > v1);
        QVERIFY(v1 != v2);
        QVERIFY(v1 <= v2);
        QVERIFY(v2 >= v1);
    }
    {
        version v1(0, 0, 1);
        version v2(1, 0, 0);
        QVERIFY(v1 < v2);
        QVERIFY(v2 > v1);
        QVERIFY(v1 != v2);
        QVERIFY(v1 <= v2);
        QVERIFY(v2 >= v1);
    }
    {
        version v1(0, 1, 0);
        version v2(1, 0, 0);
        QVERIFY(v1 < v2);
        QVERIFY(v2 > v1);
        QVERIFY(v1 != v2);
        QVERIFY(v1 <= v2);
        QVERIFY(v2 >= v1);
    }
}

void version_checker_tests::version_string_test()
{
    QCOMPARE(QString(version(10)), QString("10.0.0"));
    QCOMPARE(QString(version(10, 200)), QString("10.200.0"));
    QCOMPARE(QString(version(1, 2, 3)), QString("1.2.3"));
    QCOMPARE(QString(version(11, 222, 3333)), QString("11.222.3333"));
}

void version_checker_tests::version_checker_test()
{
    {
        version rhs;
        version_checker chkr(version_checker::equal);
        QVERIFY(chkr.isValid());
        chkr.bind_second(rhs);
        QVERIFY(chkr.isValid());
        QVERIFY(chkr(version()));
    }
    {
        version rhs(1);
        version_checker chkr(version_checker::less);
        QVERIFY(chkr.isValid());
        chkr.bind_second(rhs);
        QVERIFY(chkr.isValid());
        QVERIFY(chkr(version(0, 1)));
    }
    {
        version rhs(1);
        version_checker chkr(version_checker::greather);
        QVERIFY(chkr.isValid());
        chkr.bind_second(rhs);
        QVERIFY(chkr.isValid());
        QVERIFY(chkr(version(1, 1)));
    }
}

void version_checker_tests::version_checker_string_test()
{
    {
        version_checker chkr = version_checker::fromString(">0.0.1");
        QVERIFY(chkr.isValid());
        QCOMPARE(QString(chkr.required()), QString("0.0.1"));
        QVERIFY(chkr(version(1)));
    }
    {
        version_checker chkr = version_checker::fromString(">=0.0.1");
        QVERIFY(chkr.isValid());
        QCOMPARE(QString(chkr.required()), QString("0.0.1"));
        QVERIFY(chkr(version(0, 0, 1)));
    }
    {
        version_checker chkr = version_checker::fromString("<=1.0.1");
        QVERIFY(chkr.isValid());
        QCOMPARE(QString(chkr.required()), QString("1.0.1"));
        QVERIFY(chkr(version(0, 0, 1)));
    }
    {
        version_checker chkr = version_checker::fromString("=1.0.1");
        QVERIFY(chkr.isValid());
        QCOMPARE(QString(chkr.required()), QString("1.0.1"));
        QVERIFY(chkr(version(1, 0, 1)));
    }
    {
        version_checker chkr = version_checker::fromString("1.0.1");
        QVERIFY(chkr.isValid());
        QCOMPARE(QString(chkr.required()), QString("1.0.1"));
        QVERIFY(chkr(version(1, 0, 1)));
    }
}

QTEST_MAIN(version_checker_tests)
