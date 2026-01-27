/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAdaptedLockTest.h"

#include "simpletest.h"

#include "KisAdaptedLock.h"

/**
 * A simple test adapter that tracks lock/unlock calls
 */
class TestAdapter
{
public:
    struct State {
        int lockCount = 0;
        int unlockCount = 0;
        bool isLocked = false;
    };

    TestAdapter(State* state) : m_state(state) {}

    void lock() {
        m_state->lockCount++;
        m_state->isLocked = true;
    }

    void unlock() {
        m_state->unlockCount++;
        m_state->isLocked = false;
    }

    bool try_lock() {
        m_state->lockCount++;
        m_state->isLocked = true;
        return true;
    }

private:
    State* m_state;
};

KIS_DECLARE_ADAPTED_LOCK(TestAdaptedLock, TestAdapter)

void KisAdaptedLockTest::testConstructorDefault()
{
    TestAdapter::State state;
    {
        TestAdaptedLock lock(&state);
        QCOMPARE(state.lockCount, 1);
        QCOMPARE(state.isLocked, true);
    }
    QCOMPARE(state.unlockCount, 1);
    QCOMPARE(state.isLocked, false);
}

void KisAdaptedLockTest::testConstructorTryLock()
{
    TestAdapter::State state;
    {
        TestAdaptedLock lock(&state, std::try_to_lock);
        QCOMPARE(state.lockCount, 1);
        QCOMPARE(state.isLocked, true);
        QCOMPARE(lock.owns_lock(), true);
    }
    QCOMPARE(state.unlockCount, 1);
    QCOMPARE(state.isLocked, false);
}

void KisAdaptedLockTest::testConstructorDeferLock()
{
    TestAdapter::State state;
    {
        TestAdaptedLock lock(&state, std::defer_lock);
        // defer_lock should not call lock()
        QCOMPARE(state.lockCount, 0);
        QCOMPARE(state.isLocked, false);
        QCOMPARE(lock.owns_lock(), false);

        // now manually lock
        lock.lock();
        QCOMPARE(state.lockCount, 1);
        QCOMPARE(state.isLocked, true);
    }
    QCOMPARE(state.unlockCount, 1);
    QCOMPARE(state.isLocked, false);
}

void KisAdaptedLockTest::testConstructorAdoptLock()
{
    TestAdapter::State state;
    {
        // manually lock first
        TestAdapter adapter(&state);
        adapter.lock();
        QCOMPARE(state.lockCount, 1);

        // adopt_lock assumes the lock is already held
        TestAdaptedLock lock(&state, std::adopt_lock);
        QCOMPARE(state.lockCount, 1); // no additional lock call
        QCOMPARE(lock.owns_lock(), true);
    }
    QCOMPARE(state.unlockCount, 1);
    QCOMPARE(state.isLocked, false);
}

void KisAdaptedLockTest::testMoveConstructor()
{
    TestAdapter::State state;
    {
        TestAdaptedLock lock1(&state);
        QCOMPARE(state.lockCount, 1);
        QCOMPARE(lock1.owns_lock(), true);

        // move constructor
        TestAdaptedLock lock2(std::move(lock1));
        QCOMPARE(lock1.owns_lock(), false); // original should not own lock
        QCOMPARE(lock2.owns_lock(), true);  // new lock should own it
        QCOMPARE(state.unlockCount, 0);     // should not unlock yet
    }
    // only one unlock when lock2 goes out of scope
    QCOMPARE(state.unlockCount, 1);
    QCOMPARE(state.isLocked, false);
}

void KisAdaptedLockTest::testInPlaceMoveConstructor()
{
    TestAdapter::State state;
    {
        TestAdaptedLock lock1(&state);
        QCOMPARE(state.lockCount, 1);
        QCOMPARE(lock1.owns_lock(), true);

        // in-place move constructor
        TestAdaptedLock lock2 = std::move(lock1);
        QCOMPARE(lock1.owns_lock(), false); // original should not own lock
        QCOMPARE(lock2.owns_lock(), true);  // new lock should own it
        QCOMPARE(state.unlockCount, 0);     // should not unlock yet
    }
    // only one unlock when lock2 goes out of scope
    QCOMPARE(state.unlockCount, 1);
    QCOMPARE(state.isLocked, false);
}

void KisAdaptedLockTest::testMoveAssignmentOperator()
{
    TestAdapter::State state1;
    TestAdapter::State state2;
    {
        TestAdaptedLock lock1(&state1);
        QCOMPARE(state1.lockCount, 1);

        TestAdaptedLock lock2(&state2);
        QCOMPARE(state2.lockCount, 1);

        // move assignment
        lock2 = std::move(lock1);

        // lock2 should now own the lock from state1
        QCOMPARE(lock2.owns_lock(), true);
        QCOMPARE(lock1.owns_lock(), false);

        // state1 was locked by constructor and the move assignment
        // didn't change that
        QCOMPARE(state1.lockCount, 1);
        QCOMPARE(state1.unlockCount, 0);
        QCOMPARE(state1.isLocked, true);
        // state2 was unlocked by move assingment
        QCOMPARE(state2.lockCount, 1);
        QCOMPARE(state2.unlockCount, 1);
        QCOMPARE(state2.isLocked, false);
    }
    QCOMPARE(state1.unlockCount, 1);
    QCOMPARE(state1.isLocked, false);
}

SIMPLE_TEST_MAIN(KisAdaptedLockTest);
