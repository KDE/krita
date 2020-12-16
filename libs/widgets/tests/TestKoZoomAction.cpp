/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestKoZoomAction.h"

#include <QTest>
#include <QDialog>

#include <QVBoxLayout>

#include "kis_debug.h"
#include "KoZoomMode.h"

void TestKoZoomAction::test()
{
    KoZoomAction* action = new KoZoomAction(KoZoomMode::ZOOM_CONSTANT, "zoom", 0);
    qreal prev, after;
    do {
        prev = action->effectiveZoom();
        action->zoomIn();
        after = action->effectiveZoom();
        //qDebug() << prev << after;
    } while (after != prev);
    //qDebug() << action->maximumZoom();
    //qDebug() << action->effectiveZoom();
    //qDebug() << action->nextZoomLevel();

    QVERIFY(action->nextZoomLevel() == action->effectiveZoom());
    delete action;
}


QTEST_MAIN(TestKoZoomAction)
