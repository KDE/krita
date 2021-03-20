/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_psd_layer_style_test.h"
#include "kis_psd_layer_style.h"

#include <QString>
#include <QFile>
#include <QByteArray>
#include <QBuffer>

#include <simpletest.h>

void KisPSDLayerStyleTest::testRoundTrip()
{
    KisPSDLayerStyle layerStyle;
}

SIMPLE_TEST_MAIN(KisPSDLayerStyleTest)

