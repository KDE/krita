/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_shared_ptr_test.h"
#include <qtest_kde.h>


#include "kis_shared_ptr.h"
#include "kis_shared_ptr_vector.h"
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
typedef KisSharedPtrVector<TestClass> vTestClassSP;
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


QTEST_KDEMAIN(KisSharedPtrTest, NoGUI)
#include "kis_shared_ptr_test.moc"


