/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_asl_layer_style_serializer_test.h"

#include <qtest_kde.h>

#include <QDomDocument>

#include <KoCompositeOpRegistry.h>
#include <KoAbstractGradient.h>
#include <KoPattern.h>


#include "testutil.h"
#include "kis_psd_layer_style.h"
#include "../kis_asl_layer_style_serializer.h"
#include "../kis_asl_reader.h"


#define CMP(object, method, value) QCOMPARE(style->object()->method(), value)

void KisAslLayerStyleSerializerTest::testReading()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());

    KisAslLayerStyleSerializer s(style.data());

//    QString srcFileName(TestUtil::fetchDataFileLazy("test_all_style.asl"));
    QString srcFileName(TestUtil::fetchDataFileLazy("test_all_and_pattern.asl"));
    QFile aslFile(srcFileName);
    aslFile.open(QIODevice::ReadOnly);
    s.readFromDevice(&aslFile);

    CMP(dropShadow, effectEnabled, true);
    CMP(dropShadow, blendMode, COMPOSITE_MULT);
    CMP(dropShadow, color, QColor(Qt::black));
    CMP(dropShadow, opacity, 15);
    CMP(dropShadow, angle, -120);
    CMP(dropShadow, useGlobalLight, false);
    CMP(dropShadow, distance, 2);
    CMP(dropShadow, spread, 1);
    CMP(dropShadow, size, 7);
    CMP(dropShadow, antiAliased, true);
    CMP(dropShadow, noise, 3);
    // CMP(dropShadow, contourLookupTable,);

    CMP(innerShadow, effectEnabled, true);
    CMP(innerShadow, blendMode, COMPOSITE_DARKEN);
    CMP(innerShadow, color, QColor(Qt::black));
    CMP(innerShadow, opacity, 28);
    CMP(innerShadow, angle, 120);
    CMP(innerShadow, useGlobalLight, true);
    CMP(innerShadow, distance, 8);
    CMP(innerShadow, spread, 15);
    CMP(innerShadow, size, 27);
    CMP(innerShadow, antiAliased, false);
    CMP(innerShadow, noise, 10);
    // CMP(innerShadow, contourLookupTable,);

    CMP(outerGlow, effectEnabled, true);
    CMP(outerGlow, blendMode, COMPOSITE_SCREEN);
    CMP(outerGlow, color, QColor(255,255,189.997));
    CMP(outerGlow, opacity, 43);
    CMP(outerGlow, spread, 23);
    CMP(outerGlow, size, 109);
    CMP(outerGlow, antiAliased, true);
    CMP(outerGlow, noise, 29);
    // CMP(outerGlow, contourLookupTable,);
    // CMP(outerGlow, gradient,);
    CMP(outerGlow, fillType, psd_fill_solid_color);
    CMP(outerGlow, technique, psd_technique_precise);
    CMP(outerGlow, range, 69);
    CMP(outerGlow, jitter, 18);

    CMP(innerGlow, effectEnabled, true);
    CMP(innerGlow, blendMode, COMPOSITE_SCREEN);
    CMP(innerGlow, color, QColor(255,255,189.997));
    CMP(innerGlow, opacity, 55);
    CMP(innerGlow, spread, 21);
    CMP(innerGlow, size, 128);
    CMP(innerGlow, antiAliased, true);
    CMP(innerGlow, noise, 33);
    // CMP(innerGlow, contourLookupTable,);
    // CMP(innerGlow, gradient,);
    CMP(innerGlow, fillType, psd_fill_solid_color);
    CMP(innerGlow, technique, psd_technique_softer);
    CMP(innerGlow, range, 32);
    CMP(innerGlow, jitter, 22);
    CMP(innerGlow, source, psd_glow_edge);

    CMP(satin, effectEnabled, true);
    CMP(satin, blendMode, COMPOSITE_MULT);
    CMP(satin, color, QColor(Qt::black));
    CMP(satin, opacity, 68);
    CMP(satin, angle, 19);
    CMP(satin, distance, 11);
    CMP(satin, size, 14);
    CMP(satin, antiAliased, false);
    CMP(satin, invert, true);
    // CMP(satin, contourLookupTable,);

    CMP(colorOverlay, effectEnabled, true);
    CMP(colorOverlay, blendMode, COMPOSITE_OVER);
    CMP(colorOverlay, color, QColor(Qt::red));
    CMP(colorOverlay, opacity, 63);

    CMP(gradientOverlay, effectEnabled, true);
    CMP(gradientOverlay, blendMode, COMPOSITE_OVER);
    CMP(gradientOverlay, opacity, 100);
    CMP(gradientOverlay, angle, 90);
    CMP(gradientOverlay, style, psd_gradient_style_linear);
    CMP(gradientOverlay, reverse, false);
    CMP(gradientOverlay, alignWithLayer, true);
    CMP(gradientOverlay, scale, 100);
    CMP(gradientOverlay, gradientXOffset, 0);
    CMP(gradientOverlay, gradientYOffset, 0);
    //CMP(gradientOverlay, dither, );
    CMP(gradientOverlay, gradient()->name, QString("Two Color"));

    CMP(stroke, effectEnabled, true);
    CMP(stroke, blendMode, COMPOSITE_OVER);
    CMP(stroke, opacity, 67);
    CMP(stroke, size, 13);
    CMP(stroke, fillType, psd_fill_solid_color);
    CMP(stroke, position, psd_stroke_outside);
    CMP(stroke, color, QColor(210.0, 33.7665, 87.6887));

    CMP(bevelAndEmboss, effectEnabled, true);

    CMP(bevelAndEmboss, highlightBlendMode, COMPOSITE_SCREEN);
    CMP(bevelAndEmboss, highlightOpacity, 75);
    CMP(bevelAndEmboss, highlightColor, QColor(255.0, 255.0, 255.0));

    CMP(bevelAndEmboss, shadowBlendMode, COMPOSITE_MULT);
    CMP(bevelAndEmboss, shadowOpacity, 75);
    CMP(bevelAndEmboss, shadowColor, QColor(Qt::black));

    CMP(bevelAndEmboss, technique, psd_technique_softer);
    CMP(bevelAndEmboss, style, psd_bevel_inner_bevel);

    CMP(bevelAndEmboss, useGlobalLight, true);
    CMP(bevelAndEmboss, angle, 120);
    CMP(bevelAndEmboss, altitude, 30);

    CMP(bevelAndEmboss, depth, 83);
    CMP(bevelAndEmboss, size, 49);

    CMP(bevelAndEmboss, direction, psd_direction_up);

    // FIXME: contour
    CMP(bevelAndEmboss, glossAntiAliased, false);
    CMP(bevelAndEmboss, soften, 2);
    CMP(bevelAndEmboss, contourEnabled, true);
    // FIXME: contour curve

    CMP(bevelAndEmboss, antiAliased, true);
    CMP(bevelAndEmboss, contourRange, 60);
    CMP(bevelAndEmboss, textureEnabled, false);

    CMP(patternOverlay, effectEnabled, true);
    CMP(patternOverlay, blendMode, COMPOSITE_OVER);
    CMP(patternOverlay, opacity, 100);
    CMP(patternOverlay, alignWithLayer, true);
    CMP(patternOverlay, scale, 100);
    CMP(patternOverlay, horizontalPhase, 201);
    CMP(patternOverlay, verticalPhase, 162);

    CMP(patternOverlay, pattern()->name, QString("$$$/Presets/Patterns/Patterns_pat/Bubbles=Bubbles"));
    CMP(patternOverlay, pattern()->filename, QString("b7334da0-122f-11d4-8bb5-e27e45023b5f.pat"));

}

void KisAslLayerStyleSerializerTest::testWriting()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());

    QByteArray refXMLDoc;

    {
        KisAslLayerStyleSerializer s(style.data());

        QString srcFileName(TestUtil::fetchDataFileLazy("test_all_and_pattern.asl"));
        QFile aslFile(srcFileName);
        aslFile.open(QIODevice::ReadOnly);
        s.readFromDevice(&aslFile);

        {
            aslFile.seek(0);

            KisAslReader reader;
            QDomDocument doc = reader.readFile(&aslFile);
            refXMLDoc = doc.toByteArray();
        }
    }

    // now we have an initialized KisPSDLayerStyle object
    {
        KisAslLayerStyleSerializer s(style.data());
        QFile dstFile("test_written.asl");
        dstFile.open(QIODevice::WriteOnly);
        s.saveToDevice(&dstFile);
        dstFile.close();
    }

    QByteArray resultXMLDoc;

    {
        QFile resultFile("test_written.asl");
        resultFile.open(QIODevice::ReadOnly);

        KisAslReader reader;
        QDomDocument doc = reader.readFile(&resultFile);
        resultXMLDoc = doc.toByteArray();
    }

    QFile refXMLFile("save_round_trip_src.xml");
    refXMLFile.open(QIODevice::WriteOnly);
    refXMLFile.write(refXMLDoc);
    refXMLFile.close();

    QFile resultXMLFile("save_round_trip_dst.xml");
    resultXMLFile.open(QIODevice::WriteOnly);
    resultXMLFile.write(resultXMLDoc);
    resultXMLFile.close();

    QCOMPARE(resultXMLDoc, refXMLDoc);
}

QTEST_KDEMAIN(KisAslLayerStyleSerializerTest, GUI)
