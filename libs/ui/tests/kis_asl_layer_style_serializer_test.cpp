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

#include <QTest>

#include <QDomDocument>

#include <KoCompositeOpRegistry.h>
#include <resources/KoAbstractGradient.h>
#include <resources/KoStopGradient.h>

#include <resources/KoPattern.h>

#include "kis_global.h"

#include "testutil.h"
#include "kis_psd_layer_style.h"
#include "kis_asl_layer_style_serializer.h"
#include <asl/kis_asl_reader.h>


#define CMP(object, method, value) QCOMPARE(style->object()->method(), value)

void KisAslLayerStyleSerializerTest::testReading()
{
    KisAslLayerStyleSerializer s;

//    QString srcFileName(TestUtil::fetchDataFileLazy("asl/test_all_style.asl"));
    QString srcFileName(TestUtil::fetchDataFileLazy("asl/test_all_and_pattern.asl"));
    QFile aslFile(srcFileName);
    aslFile.open(QIODevice::ReadOnly);
    s.readFromDevice(&aslFile);

    QVector<KisPSDLayerStyleSP> styles = s.styles();

    QVERIFY(styles.size() == 1);
    KisPSDLayerStyleSP style = styles.first();

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
    CMP(outerGlow, color, QColor(255, 255, 189));
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
    CMP(innerGlow, color, QColor(255, 255, 189));
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
    CMP(stroke, color, QColor(210, 33, 87));

    CMP(bevelAndEmboss, effectEnabled, true);

    CMP(bevelAndEmboss, highlightBlendMode, COMPOSITE_SCREEN);
    CMP(bevelAndEmboss, highlightOpacity, 75);
    CMP(bevelAndEmboss, highlightColor, QColor(255, 255, 255));

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
    QVector<KisPSDLayerStyleSP> styles;

    QByteArray refXMLDoc;

    {
        KisAslLayerStyleSerializer s;

        QString srcFileName(TestUtil::fetchDataFileLazy("asl/test_all_and_pattern.asl"));
        QFile aslFile(srcFileName);
        aslFile.open(QIODevice::ReadOnly);
        s.readFromDevice(&aslFile);

        styles = s.styles();

        {
            aslFile.seek(0);

            KisAslReader reader;
            QDomDocument doc = reader.readFile(&aslFile);
            refXMLDoc = doc.toByteArray();
        }
    }

    // now we have an initialized KisPSDLayerStyle object
    {
        KisAslLayerStyleSerializer s;

        s.setStyles(styles);

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

    QEXPECT_FAIL("", "Tried to compare two xml files, which are not the same. The order of attributes when serializing is undefined", Continue);
    QCOMPARE(resultXMLDoc, refXMLDoc);
}

#include <KoResourceServerProvider.h>


void KisAslLayerStyleSerializerTest::testWritingGlobalPatterns()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());

    KoResourceServer<KoPattern> *server = KoResourceServerProvider::instance()->patternServer();
    QList<KoPatternSP> sortedResources = server->sortedResources();

    KoPatternSP pattern = sortedResources.first();
    Q_ASSERT(pattern);

    dbgKrita << ppVar(pattern->name());
    dbgKrita << ppVar(pattern->filename());

    style->patternOverlay()->setEffectEnabled(true);
    style->patternOverlay()->setPattern(pattern);

    {
        KisAslLayerStyleSerializer s;

        s.setStyles(QVector<KisPSDLayerStyleSP>() << style);

        QFile dstFile("test_written_pattern_only.asl");
        dstFile.open(QIODevice::WriteOnly);
        s.saveToDevice(&dstFile);
        dstFile.close();
    }
/*
    QByteArray resultXMLDoc;

    {
        QFile resultFile("test_written.asl");
        resultFile.open(QIODevice::ReadOnly);

        KisAslReader reader;
        QDomDocument doc = reader.readFile(&resultFile);
        resultXMLDoc = doc.toByteArray();
    }
*/

}

void KisAslLayerStyleSerializerTest::testReadMultipleStyles()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());

    QVector<KisPSDLayerStyleSP> styles;

    {
        KisAslLayerStyleSerializer s;

        QString srcFileName(TestUtil::fetchDataFileLazy("asl/multiple_styles.asl"));
        QFile aslFile(srcFileName);
        aslFile.open(QIODevice::ReadOnly);
        s.readFromDevice(&aslFile);

        styles = s.styles();
    }


    {
        KisAslLayerStyleSerializer s;

        QString dstFileName("multiple_styles_out.asl");
        QFile aslFile(dstFileName);
        aslFile.open(QIODevice::WriteOnly);

        s.setStyles(styles);
        s.saveToDevice(&aslFile);
    }

    {
        KisAslLayerStyleSerializer s;

        QString srcFileName("multiple_styles_out.asl");
        QFile aslFile(srcFileName);
        aslFile.open(QIODevice::ReadOnly);
        s.readFromDevice(&aslFile);

        styles = s.styles();

        dbgKrita << ppVar(styles.size());
    }
}

void KisAslLayerStyleSerializerTest::testWritingGradients()
{
    KoStopGradient stopGradient("");

    {
        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        QList<KoGradientStop> stops;
        stops << KoGradientStop(0.0, KoColor(Qt::black, cs));
        stops << KoGradientStop(0.3, KoColor(Qt::red, cs));
        stops << KoGradientStop(0.6, KoColor(Qt::green, cs));
        stops << KoGradientStop(1.0, KoColor(Qt::white, cs));
        stopGradient.setStops(stops);
    }

    KisPSDLayerStyleSP style(new KisPSDLayerStyle());

    style->outerGlow()->setEffectEnabled(true);
    style->outerGlow()->setFillType(psd_fill_gradient);
    style->outerGlow()->setGradient(toQShared(new KoStopGradient(stopGradient)));

    style->innerGlow()->setEffectEnabled(true);
    style->innerGlow()->setFillType(psd_fill_gradient);
    style->innerGlow()->setGradient(toQShared(new KoStopGradient(stopGradient)));

    style->gradientOverlay()->setEffectEnabled(true);
    style->gradientOverlay()->setGradient(toQShared(new KoStopGradient(stopGradient)));

    style->stroke()->setEffectEnabled(true);
    style->stroke()->setFillType(psd_fill_gradient);
    style->stroke()->setGradient(toQShared(new KoStopGradient(stopGradient)));

    {
        KisAslLayerStyleSerializer s;

        s.setStyles(QVector<KisPSDLayerStyleSP>() << style);

        QFile dstFile("test_written_stop_gradient.asl");
        dstFile.open(QIODevice::WriteOnly);
        s.saveToDevice(&dstFile);
        dstFile.close();
    }

    QString xmlDoc;

    {
        QFile resultFile("test_written_stop_gradient.asl");
        resultFile.open(QIODevice::ReadOnly);

        KisAslReader reader;
        QDomDocument doc = reader.readFile(&resultFile);
        xmlDoc = doc.toString();
    }

    {
        // the reference document has stripped "Idnt" field which is random

        QRegExp rx("<node key=\"Idnt\" type=\"Text\" value=\".+\"/>");
        rx.setMinimal(true);

        int pos = 0;
        while ((pos = rx.indexIn(xmlDoc, pos)) != -1) {
            xmlDoc.remove(pos, rx.matchedLength());
        }

        {
            //QFile xmlFile("reference_gradients.asl.xml");
            //xmlFile.open(QIODevice::WriteOnly);
            //xmlFile.write(xmlDoc.toLatin1());
            //xmlFile.close();
        }

        QString refFileName(TestUtil::fetchDataFileLazy("reference_gradients.asl.xml"));
        QFile refFile(refFileName);
        refFile.open(QIODevice::ReadOnly);
        QString refDoc = QString(refFile.readAll());

        QEXPECT_FAIL("", "Tried to compare two gradient files, which are not the same. The order of attributes when serializing is undefined.", Continue);
        QCOMPARE(xmlDoc, refDoc);
    }
}

QTEST_MAIN(KisAslLayerStyleSerializerTest)
