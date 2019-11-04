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

#ifndef KIS_STROKE_BENCHMARK_H
#define KIS_STROKE_BENCHMARK_H

#include <QtTest>
#include <kis_types.h>
#include <KoColor.h>
#include <kis_painter.h>
#include <brushengine/kis_paint_information.h>
#include <kis_image.h>
#include <kis_layer.h>


const QString PRESET_FILE_NAME = "hairy-benchmark1.kpp";

class KisStrokeBenchmark : public QObject
{
    Q_OBJECT
private:
    const KoColorSpace * m_colorSpace;
    KoColor m_color;
    KisImageSP m_image;
    KisLayerSP m_layer;

    KisPainter * m_painter;

    KisPaintInformation m_pi1;
    KisPaintInformation m_pi2;
    KisPaintInformation m_pi3;

    QPointF m_c1;
    QPointF m_c2;

    QVector<QPointF> m_startPoints;
    QVector<QPointF> m_endPoints;

    QVector<QPoint> m_rectangleLeftLowerCorners;
    QVector<QPoint> m_rectangleRightUpperCorners;


    void initCurvePoints(int width, int height);
    void initLines(int width, int height);
    void initRectangles(int width, int height);

    QString m_dataPath;
    QString m_outputPath;

    private:
        inline void benchmarkRandomLines(QString presetFileName);
        inline void benchmarkStroke(QString presetFileName);
        inline void benchmarkLine(QString presetFileName);
        inline void benchmarkCircle(QString presetFileName);
        inline void benchmarkRectangle(QString presetFileName);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();

    // AutoBrush
    void pixelbrush300px();
    void pixelbrush300pxRL();

    // Soft brush benchmarks
    void softbrushDefault30();
    void softbrushDefault30RL();
    void softbrushCircle30();
    void softbrushFullFeatures30();
    void softbrushFullFeatures30RL();

    void softbrushSoftness();
    void softbrushOpacity();

    // Hairy brush benchmarks
    void hairy30pxDefault();
    void hairy30pxDefaultRL();

    void hairy30pxAntiAlias();
    void hairy30pxAntiAliasRL();

    void hairy30px30density();
    void hairy30px30densityRL();

    void hairy30InkDepletion();
    void hairy30InkDepletionRL();

    // Spray brush benchmark1
    void spray30px21particles();
    void spray30px21particlesRL();

    void sprayPencil();
    void sprayPencilRL();

    void sprayPixels();
    void sprayPixelsRL();

    void sprayTexture();
    void sprayTextureRL();

    void dynabrush();
    void dynabrushRL();

    void deformBrush();
    void deformBrushRL();

    void experimental();
    void experimentalCircle();

    void colorsmudge();
    void colorsmudgeRL();

    void roundMarker();
    void roundMarkerRandomLines();
    void roundMarkerRectangle();

    void roundMarkerHalfPixel();
    void roundMarkerRandomLinesHalfPixel();
    void roundMarkerRectangleHalfPixel();

/*
    void predefinedBrush();
    void predefinedBrushRL();
*/
    void benchmarkRand();
    void benchmarkRand48();

    void becnhmarkPresetCloning();
};

#endif
