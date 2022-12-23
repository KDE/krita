/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisMplTest.h"

#include <simpletest.h>

#include <optional>
#include <QDebug>

#include <KisMpl.h>


void KisMplTest::test()
{
    std::optional<int> a(0x1);
    std::optional<int> b(0x2);
    std::optional<int> c(0x4);
    std::optional<int> d;
    std::optional<int> e;

    QCOMPARE(kismpl::fold_optional(std::plus{}, a, b, c, d, e), std::optional<int>(7));
    QCOMPARE(kismpl::fold_optional(std::plus{}, e, a, b, c, d), std::optional<int>(7));
    QCOMPARE(kismpl::fold_optional(std::plus{}, d, e, a, b, c), std::optional<int>(7));
    QCOMPARE(kismpl::fold_optional(std::plus{}, c, d, e, a, b), std::optional<int>(7));
    QCOMPARE(kismpl::fold_optional(std::plus{}, b, c, d, e, a), std::optional<int>(7));

    QCOMPARE(kismpl::fold_optional(std::plus{}, b, c, d, e), std::optional<int>(6));
    QCOMPARE(kismpl::fold_optional(std::plus{}, c, d, e), std::optional<int>(4));
    QCOMPARE(kismpl::fold_optional(std::plus{}, d, e), std::optional<int>());
    QCOMPARE(kismpl::fold_optional(std::plus{}, e), std::optional<int>());

}

SIMPLE_TEST_MAIN(KisMplTest)
