/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestKoClipMaskPainter.h"

#include <QTest>
#include <KoClipMaskPainter.h>
#include <QPainter>

#include <kis_debug.h>

#include "../../sdk/tests/qimage_test_util.h"


void drawRect(QPainter *gc, const QRect &rc, const QColor &color)
{
    gc->save();

    gc->setBrush(color);
    gc->drawRect(rc);

    gc->restore();
}


void TestKoClipMaskPainter::test()
{
    QImage canvas(30, 30, QImage::Format_ARGB32);
    canvas.fill(0);

    const QRect rect1(4,4,15,15);
    const QRect clipRect1(4,4,13,15);
    const QRect rect2(10,10,15,15);

    QTransform t;
    t.rotate(90);
    t = t * QTransform::fromTranslate(30, 0);

    QPainter painter(&canvas);
    painter.setPen(Qt::NoPen);
    painter.setTransform(t);

    painter.setClipRect(clipRect1);

    KoClipMaskPainter clipPainter(&painter, painter.transform().mapRect(rect1));

    // please debug using:
    //drawRect(&painter, rect1, Qt::blue);
    //drawRect(&painter, rect2, Qt::red);

    drawRect(clipPainter.shapePainter(), rect1, Qt::blue);
    drawRect(clipPainter.maskPainter(), rect2, Qt::red);

    clipPainter.renderOnGlobalPainter();

    QVERIFY(TestUtil::checkQImage(canvas, "", "clip_mask", "render_mask"));
}

QTEST_MAIN(TestKoClipMaskPainter)
