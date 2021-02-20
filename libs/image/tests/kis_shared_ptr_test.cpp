/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_shared_ptr_test.h"
#include <QTest>


#include "kis_shared_ptr.h"
#include "kis_shared.h"

class TestClassWatcher
{
public:

    TestClassWatcher() {
        deleted = false;
    }


    bool deleted;
};

class TestClass : public KisShared
{
public:

    TestClass(TestClassWatcher * tcw) {
        m_tcw = tcw;
    }

    ~TestClass() {
        m_tcw->deleted = true;
    }

    TestClassWatcher * m_tcw;
};

typedef KisSharedPtr<TestClass> TestClassSP;
typedef KisWeakSharedPtr<TestClass> TestClassWSP;
typedef QVector<TestClass> vTestClassSP;
typedef vTestClassSP::iterator vTestClassSP_it;
typedef vTestClassSP::const_iterator vTestClassSP_cit;

void KisSharedPtrTest::testRefTwoSharedPointersOneInstance()
{
    TestClassWatcher * tcw = new TestClassWatcher();
    TestClass * instance = new TestClass(tcw);

    {
        TestClassSP instanceSP(instance);
        {
            // Create a second shared ptr to the pointer in the first one,
            // pointing to the same class.
            TestClassSP instanceSP2 = instanceSP.data();
        }
        // Even though the second owner of the pointer has gone out of
        // scope, the pointer shouldn't have been deleted.
        QVERIFY(instanceSP.data() != 0);
        QVERIFY(instance != 0);
    }
    QVERIFY(tcw->deleted == true);
    delete tcw;
}

void KisSharedPtrTest::testCopy()
{

    TestClassWatcher * tcw = new TestClassWatcher();
    TestClass * instance = new TestClass(tcw);

    {
        TestClassSP instanceSP(instance);
        {
            // Copy the shared pointer; refcount should be 2 by now
            TestClassSP instanceSP2 = instance;
        }
        // Even though the second owner of the pointer has gone out of
        // scope, the pointer shouldn't have been deleted.
        QVERIFY(instanceSP.data() != 0);
        QVERIFY(instance != 0);
    }

    // The first shared pointer went out of scope; the object should
    // have been deleted and the pointer set to 0
    QVERIFY(tcw->deleted = true);
}

void KisSharedPtrTest::testCopy2()
{

    TestClassWatcher * tcw = new TestClassWatcher();
    TestClass * instance = new TestClass(tcw);

    {
        TestClassSP instanceSP(instance);
        {
            // Copy the shared pointer; refcount should be 2 by now.
            // This happens a lot in Krita code!
            TestClassSP instanceSP2(instanceSP.data());
        }
        // Even though the second owner of the pointer has gone out of
        // scope, the pointer shouldn't have been deleted.
        QVERIFY(instanceSP.data() != 0);
        QVERIFY(instance != 0);
    }

    // The first shared pointer went out of scope; the object should
    // have been deleted and the pointer set to 0
    QVERIFY(tcw->deleted = true);
}

void KisSharedPtrTest::testCopy0()
{
    TestClassSP null = 0;
    QVERIFY(null == 0);
    TestClassSP null2 = null;
    QVERIFY(null2 == 0);
    TestClassSP null3;
    null3 = null;
    QVERIFY(null3 == null);
}

void KisSharedPtrTest::testClear()
{
    TestClassWatcher * tcw = new TestClassWatcher();
    TestClassSP instance = new TestClass(tcw);
    TestClassSP instance2 = instance;
    instance.clear();
    QVERIFY(tcw->deleted = true);
    QVERIFY(instance.data() == 0);
}

void KisSharedPtrTest::testWeakSP()
{

    TestClassWatcher * tcw = new TestClassWatcher();
    TestClass * instance = new TestClass(tcw);

    {
        TestClassWSP instanceWSP(instance);
        {
            // Copy the shared pointer; refcount should be 2 by now.
            // This happens a lot in Krita code!
            TestClassSP instanceSP(instance);
        }
        // The wsp doesn't prevent the sp from deleting the instance
        QVERIFY(!instanceWSP.isValid());
        QVERIFY(tcw->deleted = true);
    }


}

void KisSharedPtrTest::testBoolOnInvalidWeakPointer()
{
    TestClassWatcher * tcw = new TestClassWatcher();
    TestClass * instance = new TestClass(tcw);

    TestClassWSP instanceWSP(instance);
    {
        // Copy the shared pointer; refcount should be 2 by now.
        // This happens a lot in Krita code!
        TestClassSP instanceSP(instance);
    }

    QString result1 = instanceWSP.isValid() ? "should not happen" : "good";
    QString result2 = instanceWSP ? "should not happen" : "good";
    QString result3 = !instanceWSP ? "good" : "should not happen";

    QCOMPARE(result1, QString("good"));
    QCOMPARE(result2, QString("good"));
    QCOMPARE(result3, QString("good"));
}

void KisSharedPtrTest::testInvalidWeakSPAssignToSP()
{
    TestClassWatcher * tcw = new TestClassWatcher();
    TestClass *instance = new TestClass(tcw);

    TestClassWSP instanceWSP(instance);
    {
        TestClassSP sp(instance);
    }

    // instanceWSP should be invalid but we should be able to assign it
    // to a new shared pointer
    TestClassSP instanceSP = instanceWSP;

    // Since the weak pointer was invalid, the shared pointer should be null
    QVERIFY(!instanceSP);
}

void KisSharedPtrTest::testInvalidWeakSPToSPCopy()
{
    TestClassWatcher * tcw = new TestClassWatcher();
    TestClass *instance = new TestClass(tcw);

    TestClassWSP instanceWSP(instance);
    {
        TestClassSP sp(instance);
    }

    // Same as above but we test the copy constructor
    TestClassSP instanceSP(instanceWSP);

    QVERIFY(!instanceSP);
}

void KisSharedPtrTest::testWeakSPAssignToWeakSP()
{
    TestClassWatcher * tcw = new TestClassWatcher();
    TestClass *instance = new TestClass(tcw);

    TestClassWSP instanceWSP(instance);

    TestClassWSP newValidInstanceWSP = instanceWSP;

    // The assignment should give us a valid weak pointer
    QVERIFY(newValidInstanceWSP.isValid());

    {
        TestClassSP sp(instance);
    }

    // instanceWSP should be invalid but we should be able to assign it
    // to a new weak shared pointer
    TestClassWSP newInvalidInstanceWSP = instanceWSP;

    // Since instanceWSP was invalid, the newInstanceWSP should be invalid
    QVERIFY(!newInvalidInstanceWSP.isValid());
}

void KisSharedPtrTest::testWeakSPToWeakSPCopy()
{
    TestClassWatcher * tcw = new TestClassWatcher();
    TestClass *instance = new TestClass(tcw);

    TestClassWSP instanceWSP(instance);

    // Same as above but we test the copy constructor
    TestClassWSP newValidInstanceWSP(instanceWSP);

    QVERIFY(newValidInstanceWSP.isValid());

    {
        TestClassSP sp(instance);
    }

    // Same as above but we test the copy constructor
    TestClassWSP newInvalidInstanceWSP(instanceWSP);

    QVERIFY(!newInvalidInstanceWSP.isValid());
}

#include "kis_restricted_shared_ptr.h"

void KisSharedPtrTest::testRestrictedPointer()
{
    QScopedPointer<TestClassWatcher> tcw(new TestClassWatcher());

    TestClass * instance = new TestClass(tcw.data());

    TestClassSP instanceSP(instance);

    typedef KisRestrictedSharedPtr<TestClass> TestClassRestrictedSP;

    TestClassRestrictedSP restricted(instanceSP);

    TestClassSP instanceSP2(restricted);
    QVERIFY(!restricted->m_tcw->deleted);

    // this line should cause a build failure!
    //TestClassRestrictedSP restricted2(instance);
}

#include "kis_pinned_shared_ptr.h"

void KisSharedPtrTest::testRestrictedPointerNoBackward()
{
    QScopedPointer<TestClassWatcher> tcw(new TestClassWatcher());

    TestClass * instance = new TestClass(tcw.data());
    TestClassSP instanceSP(instance);

    typedef KisPinnedSharedPtr<TestClass> TestClassPinnedSP;

    TestClassPinnedSP pinned(instanceSP);

    TestClassSP instanceSP2 = pinned;
    // TestClass *instance2 = pinned;
    // delete pinned;
}


QTEST_MAIN(KisSharedPtrTest)


