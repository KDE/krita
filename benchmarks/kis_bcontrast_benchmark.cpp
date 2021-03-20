/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý lukast.dev @gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <simpletest.h>

#include "kis_bcontrast_benchmark.h"
#include "kis_benchmark_values.h"


#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <kis_image.h>

#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter.h"

#include "kis_processing_information.h"

#include "kis_selection.h"
#include <kis_iterator_ng.h>
#include "krita_utils.h"
#include <KisGlobalResourcesInterface.h>

void KisBContrastBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_device = new KisPaintDevice(m_colorSpace);
    m_color = KoColor(m_colorSpace);

    srand(31524744);

    int r,g,b;

    KisSequentialIterator it(m_device, QRect(0, 0, GMP_IMAGE_WIDTH, GMP_IMAGE_HEIGHT));
    while (it.nextPixel()) {
        r = rand() % 255;
        g = rand() % 255;
        b = rand() % 255;

        m_color.fromQColor(QColor(r,g,b));
        memcpy(it.rawData(), m_color.data(), m_colorSpace->pixelSize());
    }

}

void KisBContrastBenchmark::cleanupTestCase()
{
}


void KisBContrastBenchmark::benchmarkFilter()
{
    KisFilterSP filter = KisFilterRegistry::instance()->value("brightnesscontrast");
    KisFilterConfigurationSP  kfc = filter->defaultConfiguration(KisGlobalResourcesInterface::instance());

    // Get the predefined configuration from a file
    QFile file(QString(FILES_DATA_DIR) + '/' + filter->id() + ".cfg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << kfc->toXML();
    } else {
        QString s;
        QTextStream in(&file);
        in.setCodec("UTF-8");
        s = in.readAll();
        kfc->fromXML(s);
    }

    QSize size = KritaUtils::optimalPatchSize();
    QVector<QRect> rects = KritaUtils::splitRectIntoPatches(QRect(0, 0, GMP_IMAGE_WIDTH,GMP_IMAGE_HEIGHT), size);

    QBENCHMARK{
        Q_FOREACH (const QRect &rc, rects) {
            filter->process(m_device, rc, kfc);
        }
    }
}



SIMPLE_TEST_MAIN(KisBContrastBenchmark)
