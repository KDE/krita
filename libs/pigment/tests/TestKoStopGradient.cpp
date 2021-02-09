/*
 *  SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "TestKoStopGradient.h"

#include <QTest>

#include <QDomElement>

#include "KoColorModelStandardIds.h"

#include "KoStopGradient.h"

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorProfile.h"
#include "KoColorSpaceRegistry.h"
#include "DebugPigment.h"
#include "kis_debug.h"

#include "sdk/tests/testpigment.h"

void TestKoStopGradient::TestSVGStopGradientLoading()
{
    QHash <QString, const KoColorProfile *> profileList;
    KoStopGradient gradient;

    const KoColorSpace *cmyk = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id());
    QString cmykName = "sillyCMYKName";
    profileList.insert(cmykName, cmyk->profile());

    QList<KoGradientStop> stops;

    stops << KoGradientStop(0.0, KoColor::fromSVG11("#ff00ff icc-color(sillyCMYKName, 1.0, 0, 0, 0)", profileList));
    stops << KoGradientStop(0.5, KoColor::fromSVG11("#777777 icc-color(sillyCMYKName, 0, .5, 1, 0)", profileList));
    stops << KoGradientStop(0.5, KoColor::fromSVG11("#00ff00 icc-color(sillyCMYKName, 1.0, 0, 1, 0)", profileList));

    gradient.setStops(stops);

    // We need a better way to check if this worked.

    QString svgSerialization = gradient.saveSvgGradient();
    QVERIFY2(svgSerialization.contains("icc-color"), QString("icc-color not found in serialization of cmyk gradient.").toLatin1());
    QVERIFY2(svgSerialization.contains("color-profile"), QString("color-profile not found in serialization of cmyk gradient.").toLatin1());

}

KISTEST_MAIN(TestKoStopGradient)
