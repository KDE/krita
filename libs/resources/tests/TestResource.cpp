/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestResource.h"

#include <simpletest.h>

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

SIMPLE_TEST_MAIN(TestResource)
