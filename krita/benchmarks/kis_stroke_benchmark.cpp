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

#include <stdlib.h>

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

//#define SAVE_OUTPUT

static const int LINES = 20;
const QString OUTPUT_FORMAT = ".png";

void KisStrokeBenchmark::initTestCase()
{
    m_dataPath = QString(FILES_DATA_DIR) + QDir::separator();
    m_outputPath = QString(FILES_OUTPUT_DIR) + QDir::separator();

    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_color = KoColor(m_colorSpace);

    int width = TEST_IMAGE_WIDTH;
    int height = TEST_IMAGE_HEIGHT;

    m_image = new KisImage(0, width, height, m_colorSpace, "stroke sample image", false);
    m_layer = new KisPaintLayer(m_image, "temporary for stroke sample", OPACITY_OPAQUE_U8, m_colorSpace);


    m_painter = new KisPainter(m_layer->paintDevice());
    m_painter->setPaintColor(KoColor(Qt::black, m_colorSpace));

    // for bezier curve test
    initCurvePoints(width, height);
    // for the lines test
    initLines(width,height);
}

void KisStrokeBenchmark::init()
{
    KoColor white(m_colorSpace);
    white.fromQColor(Qt::white);
    m_layer->paintDevice()->fill(0,0, m_image->width(), m_image->height(),white.data());
}


void KisStrokeBenchmark::initCurvePoints(int width, int height)
{
    QPointF p1(0                , 7.0 / 12.0 * height);
    QPointF p2(1.0 / 2.0 * width  , 7.0 / 12.0 * height);
    QPointF p3(width - 4.0, height - 4.0);

    m_c1 = QPointF(1.0 / 4.0 * width, height - 2.0);
    m_c2 = QPointF(3.0 / 4.0 * width, 0);

    m_pi1 = KisPaintInformation(p1, 0.0);
    m_pi2 = KisPaintInformation(p2, 0.95);
    m_pi3 = KisPaintInformation(p3, 0.0);
}


void KisStrokeBenchmark::initLines(int width, int height)
{
    srand(12345678);
    for (int i = 0; i < LINES; i++){
        qreal sx = rand() / qreal(RAND_MAX - 1);
        qreal sy = rand() / qreal(RAND_MAX - 1);
        m_startPoints.append(QPointF(sx * width,sy * height));
        qreal ex = rand() / qreal(RAND_MAX - 1);
        qreal ey = rand() / qreal(RAND_MAX - 1);
        m_endPoints.append(QPointF(ex * width,ey * height));
    }
}


void KisStrokeBenchmark::cleanupTestCase()
{
}

void KisStrokeBenchmark::sprayPixels()
{
    QString presetFileName = "spray_wu_pixels1.kpp";
    benchmarkStroke(presetFileName);
}

void KisStrokeBenchmark::sprayPixelsRL()
{
    QString presetFileName = "spray_wu_pixels1.kpp";
    benchmarkRandomLines(presetFileName);
}


void KisStrokeBenchmark::sprayTexture()
{
    QString presetFileName = "spray_21_textures1.kpp";
    benchmarkStroke(presetFileName);
}


void KisStrokeBenchmark::sprayTextureRL()
{
    QString presetFileName = "spray_21_textures1.kpp";
    benchmarkRandomLines(presetFileName);
}


void KisStrokeBenchmark::spray30px21particles()
{
    QString presetFileName = "spray_30px21rasterParticles.kpp";
    benchmarkStroke(presetFileName);
}

void KisStrokeBenchmark::spray30px21particlesRL()
{
    QString presetFileName = "spray_30px21rasterParticles.kpp";
    benchmarkRandomLines(presetFileName);
}

void KisStrokeBenchmark::sprayPencil()
{
    QString presetFileName = "spray_scaled2rasterParticles.kpp";
    benchmarkStroke(presetFileName);
}

void KisStrokeBenchmark::sprayPencilRL()
{
    QString presetFileName = "spray_scaled2rasterParticles.kpp";
    benchmarkRandomLines(presetFileName);
}

void KisStrokeBenchmark::softbrushDefault30()
{
    QString presetFileName = "softbrush_30px.kpp";
    benchmarkStroke(presetFileName);
}


void KisStrokeBenchmark::softbrushDefault30RL()
{
    QString presetFileName = "softbrush_30px.kpp";
    benchmarkRandomLines(presetFileName);}


void KisStrokeBenchmark::softbrushFullFeatures30()
{
    QString presetFileName = "softbrush_30px_full.kpp";
    benchmarkStroke(presetFileName);
}


void KisStrokeBenchmark::softbrushFullFeatures30RL()
{
    QString presetFileName = "softbrush_30px_full.kpp";
    benchmarkRandomLines(presetFileName);
}

void KisStrokeBenchmark::hairy30pxDefault()
{
    QString presetFileName = "hairybrush_thesis30px1.kpp";
    benchmarkStroke(presetFileName);
}

void KisStrokeBenchmark::hairy30pxDefaultRL()
{
    QString presetFileName = "hairybrush_thesis30px1.kpp";
    benchmarkRandomLines(presetFileName);
}

void KisStrokeBenchmark::hairy30pxAntiAlias()
{
    QString presetFileName = "hairybrush_thesis30px_antialiasing1.kpp";
    benchmarkStroke(presetFileName);
}

void KisStrokeBenchmark::hairy30pxAntiAliasRL()
{
    QString presetFileName = "hairybrush_thesis30px_antialiasing1.kpp";
    benchmarkRandomLines(presetFileName);
}

void KisStrokeBenchmark::hairy30px30density()
{
    QString presetFileName = "hairybrush_thesis30px_density301.kpp";
    benchmarkStroke(presetFileName);
}

void KisStrokeBenchmark::hairy30px30densityRL()
{
    QString presetFileName = "hairybrush_thesis30px_density301.kpp";
    benchmarkRandomLines(presetFileName);
}

void KisStrokeBenchmark::hairy30InkDepletion()
{
    QString presetFileName = "hairy30inkDepletion1.kpp";
    benchmarkStroke(presetFileName);

}

void KisStrokeBenchmark::hairy30InkDepletionRL()
{
    QString presetFileName = "hairy30inkDepletion1.kpp";
    benchmarkRandomLines(presetFileName);
}


void KisStrokeBenchmark::softbrushOpacity()
{
    QString presetFileName = "softbrush_opacity1.kpp";
    benchmarkLine(presetFileName);
}

void KisStrokeBenchmark::softbrushSoftness()
{
    QString presetFileName = "softbrush_softness1.kpp";
    benchmarkLine(presetFileName);
}


void KisStrokeBenchmark::benchmarkLine(QString presetFileName)
{
    KisPaintOpPresetSP preset = new KisPaintOpPreset(m_dataPath + presetFileName);
    preset->load();
    preset->settings()->setNode(m_layer);
    m_painter->setPaintOpPreset(preset, m_image);

    QPointF startPoint(0.10 * TEST_IMAGE_WIDTH, 0.5 * TEST_IMAGE_HEIGHT);
    QPointF endPoint(0.90 * TEST_IMAGE_WIDTH, 0.5 * TEST_IMAGE_HEIGHT);

    KisPaintInformation pi1(startPoint, 0.0);
    KisPaintInformation pi2(endPoint, 1.0);

    QBENCHMARK{
        m_painter->paintLine(pi1,pi2);
    }

#ifdef SAVE_OUTPUT
    m_layer->paintDevice()->convertToQImage(0).save(m_outputPath + presetFileName + "_line" + OUTPUT_FORMAT);
#endif

}


void KisStrokeBenchmark::benchmarkRandomLines(QString presetFileName)
{
    qDebug() << "preset : " << presetFileName;

    KisPaintOpPresetSP preset = new KisPaintOpPreset(m_dataPath + presetFileName);
    preset->load();
    preset->settings()->setNode(m_layer);
    m_painter->setPaintOpPreset(preset, m_image);

    QBENCHMARK{
        for (int i = 0; i < LINES; i++){
            KisPaintInformation pi1(m_startPoints[i], 0.0);
            KisPaintInformation pi2(m_endPoints[i], 1.0);
            m_painter->paintLine(pi1, pi2);
        }
    }

#ifdef SAVE_OUTPUT
    m_layer->paintDevice()->convertToQImage(0).save(m_outputPath + presetFileName + "_randomLines" + OUTPUT_FORMAT);
#endif
}

void KisStrokeBenchmark::benchmarkStroke(QString presetFileName)
{
    qDebug() << "preset : " << presetFileName;

    KisPaintOpPresetSP preset = new KisPaintOpPreset(m_dataPath + presetFileName);
    preset->load();
    preset->settings()->setNode(m_layer);
    m_painter->setPaintOpPreset(preset, m_image);

    QBENCHMARK{
        m_painter->paintBezierCurve(m_pi1, m_c1, m_c1, m_pi2, KisDistanceInformation());
        m_painter->paintBezierCurve(m_pi2, m_c2, m_c2, m_pi3, KisDistanceInformation());
    }

#ifdef SAVE_OUTPUT
    qDebug() << "Saving output " << m_outputPath + presetFileName + ".png";
    m_layer->paintDevice()->convertToQImage(0).save(m_outputPath + presetFileName + OUTPUT_FORMAT);
#endif
}

static const int COUNT = 1000000;
void KisStrokeBenchmark::benchmarkRand48()
{
QBENCHMARK
    {
        for (int i = 0 ; i < COUNT; i++){
            drand48();
        }
    }
}

void KisStrokeBenchmark::benchmarkRand()
{
    QBENCHMARK{
        for (int i = 0 ; i < COUNT; i++){
            rand() / (float)RAND_MAX;
        }
    }
}



QTEST_KDEMAIN(KisStrokeBenchmark, GUI)
#include "kis_stroke_benchmark.moc"
