/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <qtest_kde.h>
#include <limits.h>

#include <QList>

#include "KoColorSpace.h"

#include "kis_complex_color.h"
#include "kis_complex_color_test.h"
#include "kis_rgbks_colorspace.h"

#include "kis_types.h"
#include "kis_global.h"
#include "kis_base_node.h"
#include "kis_merge_visitor.h"


void KisComplexColorTest::testCreation()
{
	KoColorSpace *cs = new KisRGBKSColorSpace;
	KisComplexColorSP test1 = new KisComplexColor(cs);
	QVERIFY(test1 != 0);
	KisComplexColorSP test2 = new KisComplexColor(cs, KoColor(Qt::black, cs));
	QVERIFY(test2 != 0);
}

void KisComplexColorTest::testUse()
{
	KoColorSpace *cs = new KisRGBKSColorSpace;
	KisComplexColorSP test = new KisComplexColor(cs);
	QList<KoColor> vColors;
	vColors.append(KoColor(QColor(0xFFFF0000), cs)); // Red
	vColors.append(KoColor(QColor(0xFF00FF00), cs)); // Green
	vColors.append(KoColor(QColor(0xFF0000FF), cs)); // Blue
	vColors.append(KoColor(QColor(0xFF00BABA), cs)); // Whatever :)
	vColors.append(KoColor(QColor(0xFFFFFF00), cs)); // Yellow
	vColors.append(KoColor(QColor(0xFFFF00FF), cs)); // Violet
	vColors.append(KoColor(QColor(0xFFFFFFFF), cs)); // White
	vColors.append(KoColor(QColor(0xFF303030), cs)); // Dark gray
	vColors.append(KoColor(QColor(0xFF000000), cs)); // Black

	test->setSize(QSize(3,3));

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			test->setColor(i-1,j-1,vColors[3*i+j]);

	test->dab(3,3)->convertToQImage(0).save("test.png");

	QVERIFY(memcmp(test->rawData(-1,-1), vColors[0].data(), cs->pixelSize()) == 0);
	QVERIFY(memcmp(test->rawData(-1, 1), vColors[2].data(), cs->pixelSize()) == 0);
	QVERIFY(memcmp(test->rawData( 1, 1), vColors[8].data(), cs->pixelSize()) == 0);
}


QTEST_KDEMAIN(KisComplexColorTest, GUI)
#include "kis_complex_color_test.moc"
