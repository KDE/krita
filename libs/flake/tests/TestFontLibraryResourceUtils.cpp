/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestFontLibraryResourceUtils.h"

#include <kis_debug.h>
#include <kis_global.h>
#include <KoFontLibraryResourceUtils.h>

struct LibraryResource {
    LibraryResource() {
    }

    ~LibraryResource() {
        KIS_ASSERT(refCounter == 0);
    }

    int refCounter = 0;
};

LibraryResource* allocLibraryResource() {
    LibraryResource *res = new LibraryResource();
    res->refCounter++;
    return res;
}

void destroyLibraryResource(LibraryResource *res) {
    KIS_ASSERT(res);
    KIS_ASSERT(res->refCounter == 1);
    res->refCounter--;
    delete res;
}

int failingDestroyLibraryResource(LibraryResource *res) {
    KIS_ASSERT(res->refCounter == 1);
    res->refCounter--;
    delete res;

    // always report that we failed to destroy
    return -1;
}



using LibraryResourceSP = KisLibraryResourcePointer<LibraryResource, destroyLibraryResource>;


void TestFontLibraryResourceUtils::initTestCase()
{
    qputenv("KRITA_NO_ASSERT_MSG", "1");
}

void TestFontLibraryResourceUtils::testCreation()
{
    LibraryResourceSP res(allocLibraryResource());
    QVERIFY(res.data());
    QCOMPARE(res->refCounter, 1);
}

void TestFontLibraryResourceUtils::testCopy()
{
    LibraryResourceSP res(allocLibraryResource());
    QVERIFY(res.data());
    QCOMPARE(res->refCounter, 1);

    LibraryResourceSP res2(res);
    QVERIFY(res2.data());
    QCOMPARE(res2->refCounter, 1);
}

void TestFontLibraryResourceUtils::testCopyAssignment()
{
    LibraryResourceSP res(allocLibraryResource());
    QVERIFY(res.data());
    QCOMPARE(res->refCounter, 1);

    LibraryResourceSP res2(allocLibraryResource());
    QVERIFY(res2.data());
    QCOMPARE(res2->refCounter, 1);

    res = res2;

    QVERIFY(res.data());
    QCOMPARE(res->refCounter, 1);

    QVERIFY(res2.data());
    QCOMPARE(res2->refCounter, 1);
}

void TestFontLibraryResourceUtils::testMove()
{
    LibraryResourceSP res(allocLibraryResource());
    QVERIFY(res.data());
    QCOMPARE(res->refCounter, 1);

    LibraryResourceSP res2(std::move(res));
    QVERIFY(res2.data());
    QCOMPARE(res2->refCounter, 1);
}

void TestFontLibraryResourceUtils::testReset()
{
    LibraryResourceSP res;
    res.reset(allocLibraryResource());
    QVERIFY(res.data());
    QCOMPARE(res->refCounter, 1);

    res.reset();

}

void TestFontLibraryResourceUtils::testAssignNull()
{
    LibraryResourceSP res;

    LibraryResourceSP res2 = res;

    LibraryResourceSP res3(nullptr);

}

void TestFontLibraryResourceUtils::testFailedDestruction()
{
    using FailingLibraryResourceSP =
        KisLibraryResourcePointerWithSanityCheck<LibraryResource,
                                                 failingDestroyLibraryResource>;

    {
        qInfo() << "# Null resource destruction should succeed:";
        FailingLibraryResourceSP res;
    }

    {
        qInfo() << "# Non-null resource destruction should succeed:";
        FailingLibraryResourceSP res(allocLibraryResource());
    }
}

SIMPLE_TEST_MAIN(TestFontLibraryResourceUtils)
