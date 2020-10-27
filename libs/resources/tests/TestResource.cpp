/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
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
#include "TestResource.h"

#include <QTest>

#include "DummyResource.h"

void TestResource::testCopyResource()
{
    DummyResource r1("filename");
    DummyResource r2(r1);
    QCOMPARE(r1.filename(), r2.filename());
    r2.setFilename("other");
    QVERIFY(r1.filename() == "filename");
    QVERIFY(r2.filename() == "other");
}

QTEST_MAIN(TestResource)
