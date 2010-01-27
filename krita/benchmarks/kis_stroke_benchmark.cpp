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

#include "kis_stroke_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>

#include <kis_paint_information.h>
#include <kis_paintop_preset.h>

#define GMP_IMAGE_WIDTH 3274
#define GMP_IMAGE_HEIGHT 2067
#include <kis_painter.h>
#include <kis_paintop_registry.h>

void KisStrokeBenchmark::initTestCase()
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

void KisStrokeBenchmark::cleanupTestCase()
{
}

void KisStrokeBenchmark::benchmarkStroke()
{
    const QString presetFileName = "AutoBrush_70px_rotated.kpp";
    int width = TEST_IMAGE_WIDTH;
    int height = TEST_IMAGE_HEIGHT;

    KisLayerSP layer = new KisPaintLayer(0, "temporary for stroke sample", OPACITY_OPAQUE, m_colorSpace);
    KisImageSP image = new KisImage(0, width, height, m_colorSpace, "stroke sample image", false);
    KisPainter painter(layer->paintDevice());
    painter.setPaintColor(KoColor(Qt::black, m_colorSpace));

    KisPaintOpPresetSP preset = new KisPaintOpPreset(QString(FILES_DATA_DIR) + QDir::separator() + presetFileName);
    preset->settings()->setNode(layer);
    painter.setPaintOpPreset(preset, image);

    QPointF p1(0                , 7.0 / 12.0 * height);
    QPointF p2(1.0 / 2.0 * width  , 7.0 / 12.0 * height);
    QPointF p3(width - 4.0, height - 4.0);

    KisPaintInformation pi1(p1, 0.0);
    KisPaintInformation pi2(p2, 0.95);
    KisPaintInformation pi3(p3, 0.0);

    QPointF c1(1.0 / 4.0 * width, height - 2.0);
    QPointF c2(3.0 / 4.0 * width, 0);
    
    QBENCHMARK{
        painter.paintBezierCurve(pi1, c1, c1, pi2, 0);
        painter.paintBezierCurve(pi2, c2, c2, pi3, 0);
    }
}



QTEST_KDEMAIN(KisStrokeBenchmark, GUI)
#include "kis_stroke_benchmark.moc"
