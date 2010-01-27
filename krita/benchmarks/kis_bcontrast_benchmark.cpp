/*
 *  Copyright (c) 2010 Lukáš Tvrdý lukast.dev@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qtest_kde.h>

#include "kis_bcontrast_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <kis_image.h>

#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter.h"

#include "kis_processing_information.h"

#include "kis_selection.h"

#define GMP_IMAGE_WIDTH 3274
#define GMP_IMAGE_HEIGHT 2067 

void KisBContrastBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();    
    m_device = new KisPaintDevice(m_colorSpace);
    m_color = KoColor(m_colorSpace);
    
    QColor qcolor(Qt::red);
    srand(31524744);
    
    int r,g,b;
    
    KisRectIterator it = m_device->createRectIterator(0,0,GMP_IMAGE_WIDTH, GMP_IMAGE_HEIGHT);
    while (!it.isDone()) {
        r = rand() % 255;
        g = rand() % 255;
        b = rand() % 255;
        
        m_color.fromQColor(QColor(r,g,b));
        memcpy(it.rawData(), m_color.data(), m_colorSpace->pixelSize());
        ++it;
    }
    
}

void KisBContrastBenchmark::cleanupTestCase()
{
}


void KisBContrastBenchmark::benchmarkFilter()
{
    KisFilterSP filter = KisFilterRegistry::instance()->value("brightnesscontrast");
    KisFilterConfiguration * kfc = filter->defaultConfiguration(m_device);

    // Get the predefined configuration from a file
    QFile file(QString(FILES_DATA_DIR) + QDir::separator() + filter->id() + ".cfg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << kfc->toXML();
    } else {
        QString s;
        QTextStream in(&file);
        s = in.readAll();
        kfc->fromXML(s);
    }

    KisConstProcessingInformation src(m_device,  QPoint(0, 0), 0);
    KisProcessingInformation dst(m_device, QPoint(0, 0), 0);

    QBENCHMARK{
        filter->process(src, dst, QSize(GMP_IMAGE_WIDTH,GMP_IMAGE_HEIGHT), kfc);
    }
}



QTEST_KDEMAIN(KisBContrastBenchmark, GUI)
#include "kis_bcontrast_benchmark.moc"
