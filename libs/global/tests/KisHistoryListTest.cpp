/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisHistoryListTest.h"

#include <simpletest.h>
#include "KisHistoryList.h"


namespace {
auto refState = [](std::initializer_list<int> list) -> std::vector<int> {
    return list;
};

auto getState = [] (const KisHistoryList<int> &list) -> std::vector<int> {
    std::vector<int> state;
    state.reserve(list.size());
    for (int i = 0; i < list.size(); i++) {
        state.push_back(list.at(i));
    }
    return state;
};
}

void KisHistoryListTest::testRotation()
{
    KisHistoryList<int> list(5);

    QCOMPARE(getState(list), refState({}));

    list.append(1);
    QCOMPARE(getState(list), refState({1}));

    list.append(2);
    QCOMPARE(getState(list), refState({2, 1}));

    list.append(3);
    QCOMPARE(getState(list), refState({3, 2, 1}));

    list.append(4);
    QCOMPARE(getState(list), refState({4, 3, 2, 1}));

    list.append(5);
    QCOMPARE(getState(list), refState({5, 4, 3, 2, 1}));

    list.append(6);
    QCOMPARE(getState(list), refState({6, 5, 4, 3, 2}));

    list.clear();
    QCOMPARE(list.size(), 0);
    QCOMPARE(getState(list), refState({}));
}

void KisHistoryListTest::testBubbleUp()
{
    KisHistoryList<int> list(5);

    QCOMPARE(getState(list), refState({}));

    list.append(1);
    list.append(2);
    list.append(3);
    list.append(4);
    list.append(5);
    QCOMPARE(getState(list), refState({5, 4, 3, 2, 1}));

    list.append(3);

    QCOMPARE(getState(list), refState({3, 5, 4, 2, 1}));
}


SIMPLE_TEST_MAIN(KisHistoryListTest)
