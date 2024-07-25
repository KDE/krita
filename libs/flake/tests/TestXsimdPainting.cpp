/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestXsimdPainting.h"

#include <QPainter>

#include "KoClipMaskApplicatorBase.h"
#include "KoClipMaskPainter.h"

#include "kistest.h"
#include "SvgParserTestingUtils.h"

void TestXsimdPainting::testKoClipMaskPainting_data()
{
    QTest::addColumn<QColor>("colorSource");
    QTest::addColumn<QColor>("colorMask");
    QTest::addColumn<QColor>("colorFinal");

    QTest::addRow("visibleWhite") << QColor(255, 255, 255, 255) << QColor(255, 255, 255, 255)<< QColor(255, 255, 255, 255);
    QTest::addRow("completelyMasked") << QColor(255, 255, 255, 255) << QColor(0, 0, 0, 255) << QColor(0, 0, 0, 0);
    QTest::addRow("greyMask") << QColor(255, 255, 255, 255) << QColor(128, 128, 128, 255) << QColor(255, 255, 255, 128);
    QTest::addRow("semiTransparent") << QColor(255, 255, 255, 255) << QColor(255, 255, 255, 128) << QColor(255, 255, 255, 128);
    QTest::addRow("semiCyan") << QColor(255, 255, 255, 255) << QColor(128, 255, 255, 128) << QColor(255, 255, 255, 114);
    QTest::addRow("semiMagenta") << QColor(255, 255, 255, 255) << QColor(255, 128, 255, 128) << QColor(255, 255, 255, 82);
    QTest::addRow("semiYellow") << QColor(255, 255, 255, 255) << QColor(255, 255, 128, 128) << QColor(255, 255, 255, 123);
    QTest::addRow("color1") << QColor(255, 0, 0, 255) << QColor(64, 128, 255, 128) << QColor(255, 0, 0, 62);
    QTest::addRow("color2") << QColor(0, 255, 0, 255) << QColor(255, 128, 64, 128) << QColor(0, 255, 0, 75);
    QTest::addRow("color3") << QColor(0, 0, 255, 255) << QColor(128, 64, 255, 128) << QColor(0, 0, 255, 46);
}

void TestXsimdPainting::testKoClipMaskPainting()
{
    QFETCH(QColor, colorSource);
    QFETCH(QColor, colorMask);
    QFETCH(QColor, colorFinal);

    const QRect imgRect(0, 0, 5, 3);

    QImage compareImg = QImage(imgRect.size(), QImage::Format_ARGB32);
    compareImg.fill(colorFinal);
    QImage img = QImage(imgRect.size(), QImage::Format_ARGB32);
    QPainter p(&img);
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(imgRect, Qt::transparent);
    p.restore();

    KoClipMaskPainter clip (&p, imgRect);
    clip.shapePainter()->fillRect(imgRect, colorSource);
    clip.maskPainter()->fillRect(imgRect, colorMask);

    clip.renderOnGlobalPainter();

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, img, compareImg)) {
        QFAIL(QString("XSimd painting test failed, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

KISTEST_MAIN(TestXsimdPainting)
