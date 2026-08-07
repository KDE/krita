/*
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisLagerTest.h"

#include <simpletest.h>

#include <kis_debug.h>

#include <optional>

#include <lager/state.hpp>

#include <KisLager.h>
#include <KisZug.h>


void KisLagerTest::testOptionalHasValue()
{
    auto x = lager::state<std::optional<int>, lager::automatic_tag>{};
    x.set(11);

    // connect to the presence state
    auto isPresentReader = lager::reader<bool>{x.xform(kiszug::has_value)};
    auto valueReader = lager::reader<int>{x.zoom(lager::lenses::value_or(42))};

    QCOMPARE(isPresentReader.get(), true);
    QCOMPARE(valueReader.get(), 11);

    x.set(std::nullopt);

    QCOMPARE(isPresentReader.get(), false);
    QCOMPARE(valueReader.get(), 42);
}

SIMPLE_TEST_MAIN(KisLagerTest)
