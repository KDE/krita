/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestKoZoomAction.h"

#include <simpletest.h>

#include <QVBoxLayout>

#include "kis_debug.h"
#include "KoZoomMode.h"

#include <KoZoomActionState.h>

using LightZoomItem = std::tuple<KoZoomMode::Mode, qreal>;

LightZoomItem zoomItem(KoZoomMode::Mode mode) {
    return std::make_tuple(mode, -1.0);
}

LightZoomItem zoomItem(qreal zoom) {
    return std::make_tuple(KoZoomMode::ZOOM_CONSTANT, zoom);
}

QDebug operator<<(QDebug dbg, const LightZoomItem &item)
{
    dbg.nospace() << "LightZoomItem("
                  << std::get<0>(item) << ", "
                  << std::get<1>(item) << ")";

    return dbg.space();
}


bool fuzzyCompareZoom(qreal lhs, qreal rhs) {
    return qRound(lhs * 10000) == qRound(rhs * 10000);
};

bool compareZoomLevels(const QVector<qreal> &real, const QVector<qreal> &expected) {
    if (real.size() != expected.size()) {
        qWarning() << "Zoom level vectors have different size!";
        qWarning() << "    " << ppVar(real);
        qWarning() << "    " << ppVar(expected);
        return false;
    }

    auto mismatch = std::mismatch(real.begin(), real.end(),
        expected.begin(),
        fuzzyCompareZoom);

    if (mismatch.first != real.end()) {
        const int index = std::distance(real.begin(), mismatch.first);

        qWarning() << "Zoom level mismatch at index" << index;
        qWarning() << "    real:    " << *mismatch.first;
        qWarning() << "    expected:" << *mismatch.second;
        qWarning() << "   " << ppVar(real);
        qWarning() << "   " << ppVar(expected);
        return false;
    }

    return true;
};

bool compareZoomItems(const QVector<KoZoomActionState::ZoomItem> &real, const QVector<LightZoomItem> &expected) {
    if (real.size() != expected.size()) {
        qWarning() << "Zoom level vectors have different size!";
        qWarning() << "    " << ppVar(real);
        qWarning() << "    " << ppVar(expected);
        return false;
    }

    auto rit = real.begin();
    auto eit = expected.begin();

    for (; rit != real.end() && eit != expected.end();
         ++rit, ++eit) {
        if (std::get<0>(*rit) != std::get<0>(*eit) ||
            !fuzzyCompareZoom(std::get<1>(*rit), std::get<1>(*eit))) {

                const int index = std::distance(real.begin(), rit);

                qWarning() << "Zoom level mismatch at index" << index;
                qWarning() << "    real:    " << *rit;
                qWarning() << "    expected:" << *eit;
                qWarning() << "   " << ppVar(real);
                qWarning() << "   " << ppVar(expected);
                return false;
        }
    }

    return true;
};

void TestKoZoomAction::testZoomActionState_data()
{
    QTest::addColumn<KoZoomState>("zoomState");
    QTest::addColumn<QVector<qreal>>("expectedStandardLevels");
    QTest::addColumn<int>("expectedStandardLevelIndex");
    QTest::addColumn<QVector<LightZoomItem>>("expectedGuiLevels");
    QTest::addColumn<QVector<LightZoomItem>>("expectedRealLevels");
    QTest::addColumn<int>("expectedRealIndex");

    auto zi = [] (auto v) { return zoomItem(v);};

    QTest::addRow("constant-middle")
        << KoZoomState(KoZoomMode::ZOOM_CONSTANT, 0.45, 0.1, 2.0)
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1.0, 1.33333, 2.0}
        << 4
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.45), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << 5;

    QTest::addRow("constant-exact")
        << KoZoomState(KoZoomMode::ZOOM_CONSTANT, 0.5, 0.1, 2.0)
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1.0, 1.33333, 2.0}
        << 4
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << 5;

    QTest::addRow("constant-fuzzy-left")
        << KoZoomState(KoZoomMode::ZOOM_CONSTANT, 0.5 - 1e-7, 0.1, 2.0)
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1.0, 1.33333, 2.0}
        << 4
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << 5;

    QTest::addRow("constant-fuzzy-right")
        << KoZoomState(KoZoomMode::ZOOM_CONSTANT, 0.5 - 1e-7, 0.1, 2.0)
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1.0, 1.33333, 2.0}
        << 4
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << 5;

    QTest::addRow("constant-low")
        << KoZoomState(KoZoomMode::ZOOM_CONSTANT, 0.11, 0.1, 2.0)
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1.0, 1.33333, 2.0}
        << 0
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.11), zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << 3;

    QTest::addRow("constant-high")
        << KoZoomState(KoZoomMode::ZOOM_CONSTANT, 2.1, 0.1, 2.0)
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1.0, 1.33333, 2.0}
        << 8
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0), zi(2.1)
        }
        << 10;

    QTest::addRow("page")
        << KoZoomState(KoZoomMode::ZOOM_PAGE, 0.5, 0.1, 2.0)
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1.0, 1.33333, 2.0}
        << 4
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << 0;

    QTest::addRow("width")
        << KoZoomState(KoZoomMode::ZOOM_WIDTH, 0.5, 0.1, 2.0)
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1.0, 1.33333, 2.0}
        << 4
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << 1;

    QTest::addRow("height")
        << KoZoomState(KoZoomMode::ZOOM_HEIGHT, 0.5, 0.1, 2.0)
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1.0, 1.33333, 2.0}
        << 4
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << QVector<LightZoomItem>{
            zi(KoZoomMode::ZOOM_PAGE), zi(KoZoomMode::ZOOM_WIDTH), zi(KoZoomMode::ZOOM_HEIGHT),
            zi(0.25), zi(0.333333), zi(0.5), zi(0.666667), zi(1.0), zi(1.33333), zi(2.0)
        }
        << 2;
}

void TestKoZoomAction::testZoomActionState()
{
    QFETCH(KoZoomState, zoomState);
    QFETCH(QVector<qreal>, expectedStandardLevels);
    QFETCH(int, expectedStandardLevelIndex);
    QFETCH(QVector<LightZoomItem>, expectedGuiLevels);
    QFETCH(QVector<LightZoomItem>, expectedRealLevels);
    QFETCH(int, expectedRealIndex);

    KoZoomActionState actionState(zoomState);

    QCOMPARE(actionState.zoomState, zoomState);

    if (!compareZoomLevels(actionState.standardLevels, expectedStandardLevels)) {
        QFAIL("Failed to compare original standard zoom levels");
    }

    if (!compareZoomItems(actionState.guiLevels, expectedGuiLevels)) {
        QFAIL("Failed to compare original GUI levels");
    }

    if (!compareZoomItems(actionState.realGuiLevels, expectedRealLevels)) {
        QFAIL("Failed to compare original real levels");
    }

    QCOMPARE(actionState.currentRealLevelIndex, expectedRealIndex);
    QCOMPARE(actionState.calcNearestStandardLevel(actionState.zoomState.zoom), expectedStandardLevelIndex);
}


SIMPLE_TEST_MAIN(TestKoZoomAction)
