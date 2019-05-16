/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "TestSvgText.h"

#include <QTest>

#include "SvgParserTestingUtils.h"
#include <text/KoSvgText.h>
#include <text/KoSvgTextProperties.h>
#include "KoSvgTextShapeMarkupConverter.h"

#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <QFont>

void addProp(SvgLoadingContext &context,
             KoSvgTextProperties &props,
             const QString &attribute,
             const QString &value,
             KoSvgTextProperties::PropertyId id,
             int newValue)
{
    props.parseSvgTextAttribute(context, attribute, value);
    if (props.property(id).toInt() != newValue) {
        qDebug() << "Failed to load the property:";
        qDebug() << ppVar(attribute) << ppVar(value);
        qDebug() << ppVar(newValue);
        qDebug() << ppVar(props.property(id));
        QFAIL("Fail :(");
    }
}

void addProp(SvgLoadingContext &context,
             KoSvgTextProperties &props,
             const QString &attribute,
             const QString &value,
             KoSvgTextProperties::PropertyId id,
             KoSvgText::AutoValue newValue)
{
    props.parseSvgTextAttribute(context, attribute, value);
    if (props.property(id).value<KoSvgText::AutoValue>() != newValue) {
        qDebug() << "Failed to load the property:";
        qDebug() << ppVar(attribute) << ppVar(value);
        qDebug() << ppVar(newValue);
        qDebug() << ppVar(props.property(id));
        QFAIL("Fail :(");
    }
    QCOMPARE(props.property(id), QVariant::fromValue(newValue));
}

void addProp(SvgLoadingContext &context,
             KoSvgTextProperties &props,
             const QString &attribute,
             const QString &value,
             KoSvgTextProperties::PropertyId id,
             qreal newValue)
{
    props.parseSvgTextAttribute(context, attribute, value);
    if (props.property(id).toReal() != newValue) {
        qDebug() << "Failed to load the property:";
        qDebug() << ppVar(attribute) << ppVar(value);
        qDebug() << ppVar(newValue);
        qDebug() << ppVar(props.property(id));
        QFAIL("Fail :(");
    }
}


void TestSvgText::testTextProperties()
{
    KoDocumentResourceManager resourceManager;
    SvgLoadingContext context(&resourceManager);
    context.pushGraphicsContext();

    KoSvgTextProperties props;

    addProp(context, props, "writing-mode", "tb-rl", KoSvgTextProperties::WritingModeId, KoSvgText::TopToBottom);
    addProp(context, props, "writing-mode", "rl", KoSvgTextProperties::WritingModeId, KoSvgText::RightToLeft);

    addProp(context, props, "glyph-orientation-vertical", "auto", KoSvgTextProperties::GlyphOrientationVerticalId, KoSvgText::AutoValue());
    addProp(context, props, "glyph-orientation-vertical", "0", KoSvgTextProperties::GlyphOrientationVerticalId, KoSvgText::AutoValue(0));
    addProp(context, props, "glyph-orientation-vertical", "90", KoSvgTextProperties::GlyphOrientationVerticalId, KoSvgText::AutoValue(M_PI_2));
    addProp(context, props, "glyph-orientation-vertical", "95", KoSvgTextProperties::GlyphOrientationVerticalId, KoSvgText::AutoValue(M_PI_2));
    addProp(context, props, "glyph-orientation-vertical", "175", KoSvgTextProperties::GlyphOrientationVerticalId, KoSvgText::AutoValue(M_PI));
    addProp(context, props, "glyph-orientation-vertical", "280", KoSvgTextProperties::GlyphOrientationVerticalId, KoSvgText::AutoValue(3 * M_PI_2));
    addProp(context, props, "glyph-orientation-vertical", "350", KoSvgTextProperties::GlyphOrientationVerticalId, KoSvgText::AutoValue(0));
    addProp(context, props, "glyph-orientation-vertical", "105", KoSvgTextProperties::GlyphOrientationVerticalId, KoSvgText::AutoValue(M_PI_2));

    addProp(context, props, "glyph-orientation-horizontal", "0", KoSvgTextProperties::GlyphOrientationHorizontalId, 0.0);
    addProp(context, props, "glyph-orientation-horizontal", "90", KoSvgTextProperties::GlyphOrientationHorizontalId, M_PI_2);
    addProp(context, props, "glyph-orientation-horizontal", "95", KoSvgTextProperties::GlyphOrientationHorizontalId, M_PI_2);
    addProp(context, props, "glyph-orientation-horizontal", "175", KoSvgTextProperties::GlyphOrientationHorizontalId, M_PI);
    addProp(context, props, "glyph-orientation-horizontal", "280", KoSvgTextProperties::GlyphOrientationHorizontalId, 3 * M_PI_2);

    addProp(context, props, "direction", "rtl", KoSvgTextProperties::WritingModeId, KoSvgText::DirectionRightToLeft);
    addProp(context, props, "unicode-bidi", "embed", KoSvgTextProperties::UnicodeBidiId, KoSvgText::BidiEmbed);
    addProp(context, props, "unicode-bidi", "bidi-override", KoSvgTextProperties::UnicodeBidiId, KoSvgText::BidiOverride);


    addProp(context, props, "text-anchor", "middle", KoSvgTextProperties::TextAnchorId, KoSvgText::AnchorMiddle);
    addProp(context, props, "dominant-baseline", "ideographic", KoSvgTextProperties::DominantBaselineId, KoSvgText::DominantBaselineIdeographic);
    addProp(context, props, "alignment-baseline", "alphabetic", KoSvgTextProperties::AlignmentBaselineId, KoSvgText::AlignmentBaselineAlphabetic);
    addProp(context, props, "baseline-shift", "sub", KoSvgTextProperties::BaselineShiftModeId, KoSvgText::ShiftSub);
    addProp(context, props, "baseline-shift", "super", KoSvgTextProperties::BaselineShiftModeId, KoSvgText::ShiftSuper);
    addProp(context, props, "baseline-shift", "baseline", KoSvgTextProperties::BaselineShiftModeId, KoSvgText::ShiftNone);

    addProp(context, props, "baseline-shift", "10%", KoSvgTextProperties::BaselineShiftModeId, KoSvgText::ShiftPercentage);
    QCOMPARE(props.property(KoSvgTextProperties::BaselineShiftValueId).toDouble(), 0.1);

    context.currentGC()->font.setPointSizeF(180);

    addProp(context, props, "baseline-shift", "36", KoSvgTextProperties::BaselineShiftModeId, KoSvgText::ShiftPercentage);
    QCOMPARE(props.property(KoSvgTextProperties::BaselineShiftValueId).toDouble(), 0.2);

    addProp(context, props, "kerning", "auto", KoSvgTextProperties::KerningId, KoSvgText::AutoValue());
    addProp(context, props, "kerning", "20", KoSvgTextProperties::KerningId, KoSvgText::AutoValue(20.0));

    addProp(context, props, "letter-spacing", "normal", KoSvgTextProperties::LetterSpacingId, KoSvgText::AutoValue());
    addProp(context, props, "letter-spacing", "20", KoSvgTextProperties::LetterSpacingId, KoSvgText::AutoValue(20.0));

    addProp(context, props, "word-spacing", "normal", KoSvgTextProperties::WordSpacingId, KoSvgText::AutoValue());
    addProp(context, props, "word-spacing", "20", KoSvgTextProperties::WordSpacingId, KoSvgText::AutoValue(20.0));
}

void TestSvgText::testDefaultTextProperties()
{
    KoSvgTextProperties props;

    QVERIFY(props.isEmpty());
    QVERIFY(!props.hasProperty(KoSvgTextProperties::UnicodeBidiId));

    QVERIFY(KoSvgTextProperties::defaultProperties().hasProperty(KoSvgTextProperties::UnicodeBidiId));
    QCOMPARE(KoSvgTextProperties::defaultProperties().property(KoSvgTextProperties::UnicodeBidiId).toInt(), int(KoSvgText::BidiNormal));

    props = KoSvgTextProperties::defaultProperties();

    QVERIFY(props.hasProperty(KoSvgTextProperties::UnicodeBidiId));
    QCOMPARE(props.property(KoSvgTextProperties::UnicodeBidiId).toInt(), int(KoSvgText::BidiNormal));
}

void TestSvgText::testTextPropertiesDifference()
{
    using namespace KoSvgText;

    KoSvgTextProperties props;

    props.setProperty(KoSvgTextProperties::WritingModeId, RightToLeft);
    props.setProperty(KoSvgTextProperties::DirectionId, DirectionRightToLeft);
    props.setProperty(KoSvgTextProperties::UnicodeBidiId, BidiEmbed);
    props.setProperty(KoSvgTextProperties::TextAnchorId, AnchorEnd);
    props.setProperty(KoSvgTextProperties::DominantBaselineId, DominantBaselineNoChange);
    props.setProperty(KoSvgTextProperties::AlignmentBaselineId, AlignmentBaselineIdeographic);
    props.setProperty(KoSvgTextProperties::BaselineShiftModeId, ShiftPercentage);
    props.setProperty(KoSvgTextProperties::BaselineShiftValueId, 0.5);
    props.setProperty(KoSvgTextProperties::KerningId, fromAutoValue(AutoValue(10)));
    props.setProperty(KoSvgTextProperties::GlyphOrientationVerticalId, fromAutoValue(AutoValue(90)));
    props.setProperty(KoSvgTextProperties::GlyphOrientationHorizontalId, fromAutoValue(AutoValue(180)));
    props.setProperty(KoSvgTextProperties::LetterSpacingId, fromAutoValue(AutoValue(20)));
    props.setProperty(KoSvgTextProperties::WordSpacingId, fromAutoValue(AutoValue(30)));

    KoSvgTextProperties newProps = props;

    newProps.setProperty(KoSvgTextProperties::KerningId, fromAutoValue(AutoValue(11)));
    newProps.setProperty(KoSvgTextProperties::LetterSpacingId, fromAutoValue(AutoValue(21)));

    KoSvgTextProperties diff = newProps.ownProperties(props);

    QVERIFY(diff.hasProperty(KoSvgTextProperties::KerningId));
    QVERIFY(diff.hasProperty(KoSvgTextProperties::LetterSpacingId));

    QVERIFY(!diff.hasProperty(KoSvgTextProperties::WritingModeId));
    QVERIFY(!diff.hasProperty(KoSvgTextProperties::DirectionId));


}

void TestSvgText::testParseFontStyles()
{
    const QString data =
            "<text x=\"7\" y=\"7\""
            "    font-family=\"Verdana , \'Times New Roman\', serif\" font-size=\"15\" font-style=\"oblique\" fill=\"blue\""
            "    font-stretch=\"extra-condensed\""
            "    font-size-adjust=\"0.56\"" //// not implemented! should issue a warning!
            "    font=\"bold italic large Palatino, serif\"" //// not implemented! should issue a warning!
            "    font-variant=\"small-caps\" font-weight=\"600\" >"
            "    Hello, out there"
            "</text>";

    KoXmlDocument doc;
    QVERIFY(doc.setContent(data.toLatin1()));
    KoXmlElement root = doc.documentElement();

    KoDocumentResourceManager resourceManager;
    SvgLoadingContext context(&resourceManager);
    context.pushGraphicsContext();

    SvgStyles styles = context.styleParser().collectStyles(root);
    context.styleParser().parseFont(styles);

    //QCOMPARE(styles.size(), 3);

    // TODO: multiple fonts!
    QCOMPARE(context.currentGC()->font.family(), QString("Verdana"));

    {
        QStringList expectedFonts = {"Verdana", "Times New Roman", "serif"};
        QCOMPARE(context.currentGC()->fontFamiliesList, expectedFonts);
    }

    QCOMPARE(context.currentGC()->font.pointSizeF(), 15.0);
    QCOMPARE(context.currentGC()->font.style(), QFont::StyleOblique);
    QCOMPARE(context.currentGC()->font.capitalization(), QFont::SmallCaps);
    QCOMPARE(context.currentGC()->font.weight(), 66);

    {
        SvgStyles fontModifier;
        fontModifier["font-weight"] = "bolder";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(context.currentGC()->font.weight(), 75);
    }

    {
        SvgStyles fontModifier;
        fontModifier["font-weight"] = "lighter";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(context.currentGC()->font.weight(), 66);
    }

    QCOMPARE(context.currentGC()->font.stretch(), int(QFont::ExtraCondensed));

    {
        SvgStyles fontModifier;
        fontModifier["font-stretch"] = "narrower";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(context.currentGC()->font.stretch(), int(QFont::UltraCondensed));
    }

    {
        SvgStyles fontModifier;
        fontModifier["font-stretch"] = "wider";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(context.currentGC()->font.stretch(), int(QFont::ExtraCondensed));
    }

    {
        SvgStyles fontModifier;
        fontModifier["text-decoration"] = "underline";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(context.currentGC()->font.underline(), true);
    }

    {
        SvgStyles fontModifier;
        fontModifier["text-decoration"] = "overline";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(context.currentGC()->font.overline(), true);
    }

    {
        SvgStyles fontModifier;
        fontModifier["text-decoration"] = "line-through";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(context.currentGC()->font.strikeOut(), true);
    }

    {
        SvgStyles fontModifier;
        fontModifier["text-decoration"] = " line-through overline";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(context.currentGC()->font.underline(), false);
        QCOMPARE(context.currentGC()->font.strikeOut(), true);
        QCOMPARE(context.currentGC()->font.overline(), true);
    }

}

void TestSvgText::testParseTextStyles()
{
    const QString data =
            "<text x=\"7\" y=\"7\""
            "    font-family=\"Verdana\" font-size=\"15\" font-style=\"oblique\" fill=\"blue\""
            "    writing-mode=\"tb-rl\" "
            "    glyph-orientation-vertical=\"90\" >"
            "    Hello, out there"
            "</text>";

    KoXmlDocument doc;
    QVERIFY(doc.setContent(data.toLatin1()));
    KoXmlElement root = doc.documentElement();

    KoDocumentResourceManager resourceManager;
    SvgLoadingContext context(&resourceManager);
    context.pushGraphicsContext();

    SvgStyles styles = context.styleParser().collectStyles(root);
    context.styleParser().parseFont(styles);

    QCOMPARE(context.currentGC()->font.family(), QString("Verdana"));

    KoSvgTextProperties &props = context.currentGC()->textProperties;

    QCOMPARE(props.property(KoSvgTextProperties::WritingModeId).toInt(), int(KoSvgText::TopToBottom));
    QCOMPARE(props.property(KoSvgTextProperties::GlyphOrientationVerticalId).value<KoSvgText::AutoValue>(), KoSvgText::AutoValue(M_PI_2));

}

#include <text/KoSvgTextShape.h>
#include <text/KoSvgTextChunkShape.h>
#include <text/KoSvgTextChunkShapeLayoutInterface.h>

void TestSvgText::testSimpleText()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"7\" y=\"27\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        Hello, out there!"
            "    </text>"

            "</g>"

            "</svg>";

    QFont testFont("DejaVu Sans");
    if (!QFontInfo(testFont).exactMatch()) {
        QEXPECT_FAIL(0, "DejaVu Sans is *not* found! Text rendering might be broken!", Continue);
    }

    SvgRenderTester t (data);
    t.test_standard("text_simple", QSize(175, 40), 72.0);

    KoShape *shape = t.findShape("testRect");
    KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
    QVERIFY(chunkShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(shape));

    QCOMPARE(chunkShape->shapeCount(), 0);
    QCOMPARE(chunkShape->layoutInterface()->isTextNode(), true);

    QCOMPARE(chunkShape->layoutInterface()->numChars(), 17);
    QCOMPARE(chunkShape->layoutInterface()->nodeText(), QString("Hello, out there!"));

    QVector<KoSvgText::CharTransformation> transform = chunkShape->layoutInterface()->localCharTransformations();
    QCOMPARE(transform.size(), 1);
    QVERIFY(bool(transform[0].xPos));
    QVERIFY(bool(transform[0].yPos));
    QVERIFY(!transform[0].dxPos);
    QVERIFY(!transform[0].dyPos);
    QVERIFY(!transform[0].rotate);

    QCOMPARE(*transform[0].xPos, 7.0);
    QCOMPARE(*transform[0].yPos, 27.0);

    QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> subChunks =
            chunkShape->layoutInterface()->collectSubChunks();

    QCOMPARE(subChunks.size(), 1);
    QCOMPARE(subChunks[0].text.size(), 17);
    //qDebug() << ppVar(subChunks[0].text);
    //qDebug() << ppVar(subChunks[0].transformation);
    //qDebug() << ppVar(subChunks[0].format);

}

inline KoSvgTextChunkShape* toChunkShape(KoShape *shape) {
    KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
    KIS_ASSERT(chunkShape);
    return chunkShape;
}

void TestSvgText::testComplexText()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"5\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"7\" y=\"27\" dx=\"0,1,2,3,4,5,6,7,8\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        Hello, <tspan fill=\"red\" x=\"20\" y=\"46\" text-anchor=\"start\">ou"
            "t</tspan> there <![CDATA[cool cdata --> nice work]]>"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard("text_complex", QSize(385, 56), 72.0);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));

    QCOMPARE(baseShape->shapeCount(), 4);
    QCOMPARE(baseShape->layoutInterface()->isTextNode(), false);
    QCOMPARE(baseShape->layoutInterface()->numChars(), 41);

    {   // chunk 0: "Hello, "
        KoSvgTextChunkShape *chunk = toChunkShape(baseShape->shapes()[0]);

        QCOMPARE(chunk->shapeCount(), 0);
        QCOMPARE(chunk->layoutInterface()->isTextNode(), true);

        QCOMPARE(chunk->layoutInterface()->numChars(), 7);
        QCOMPARE(chunk->layoutInterface()->nodeText(), QString("Hello, "));

        QVector<KoSvgText::CharTransformation> transform = chunk->layoutInterface()->localCharTransformations();
        QCOMPARE(transform.size(), 7);
        QVERIFY(bool(transform[0].xPos));
        QVERIFY(!bool(transform[1].xPos));

        for (int i = 0; i < 7; i++) {
            QVERIFY(!i || bool(transform[i].dxPos));

            if (i) {
                QCOMPARE(*transform[i].dxPos, qreal(i));
            }
        }

        QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> subChunks =
                chunk->layoutInterface()->collectSubChunks();

        QCOMPARE(subChunks.size(), 7);
        QCOMPARE(subChunks[0].text.size(), 1);
        QCOMPARE(*subChunks[0].transformation.xPos, 7.0);
        QVERIFY(!subChunks[1].transformation.xPos);
    }

    {   // chunk 1: "out"
        KoSvgTextChunkShape *chunk = toChunkShape(baseShape->shapes()[1]);

        QCOMPARE(chunk->shapeCount(), 0);
        QCOMPARE(chunk->layoutInterface()->isTextNode(), true);

        QCOMPARE(chunk->layoutInterface()->numChars(), 3);
        QCOMPARE(chunk->layoutInterface()->nodeText(), QString("out"));

        QVector<KoSvgText::CharTransformation> transform = chunk->layoutInterface()->localCharTransformations();
        QCOMPARE(transform.size(), 2);
        QVERIFY(bool(transform[0].xPos));
        QVERIFY(!bool(transform[1].xPos));

        for (int i = 0; i < 2; i++) {
            QVERIFY(bool(transform[i].dxPos));
            QCOMPARE(*transform[i].dxPos, qreal(i + 7));
        }

        QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> subChunks =
                chunk->layoutInterface()->collectSubChunks();

        QCOMPARE(subChunks.size(), 2);
        QCOMPARE(subChunks[0].text.size(), 1);
        QCOMPARE(subChunks[1].text.size(), 2);
    }

    {   // chunk 2: " there "
        KoSvgTextChunkShape *chunk = toChunkShape(baseShape->shapes()[2]);

        QCOMPARE(chunk->shapeCount(), 0);
        QCOMPARE(chunk->layoutInterface()->isTextNode(), true);

        QCOMPARE(chunk->layoutInterface()->numChars(), 7);
        QCOMPARE(chunk->layoutInterface()->nodeText(), QString(" there "));

        QVector<KoSvgText::CharTransformation> transform = chunk->layoutInterface()->localCharTransformations();
        QCOMPARE(transform.size(), 0);

        QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> subChunks =
                chunk->layoutInterface()->collectSubChunks();

        QCOMPARE(subChunks.size(), 1);
        QCOMPARE(subChunks[0].text.size(), 7);
    }

    {   // chunk 3: "cool cdata --> nice work"
        KoSvgTextChunkShape *chunk = toChunkShape(baseShape->shapes()[3]);

        QCOMPARE(chunk->shapeCount(), 0);
        QCOMPARE(chunk->layoutInterface()->isTextNode(), true);

        QCOMPARE(chunk->layoutInterface()->numChars(), 24);
        QCOMPARE(chunk->layoutInterface()->nodeText(), QString("cool cdata --> nice work"));

        QVector<KoSvgText::CharTransformation> transform = chunk->layoutInterface()->localCharTransformations();
        QCOMPARE(transform.size(), 0);

        QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> subChunks =
                chunk->layoutInterface()->collectSubChunks();

        QCOMPARE(subChunks.size(), 1);
        QCOMPARE(subChunks[0].text.size(), 24);
    }
}

void TestSvgText::testHindiText()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"4\" y=\"24\""
            "        font-family=\"FreeSans\" font-size=\"15\" fill=\"blue\" >"
            "मौखिक रूप से हिंदी के काफी सामान"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    QFont testFont("FreeSans");
    if (!QFontInfo(testFont).exactMatch()) {
#ifdef USE_ROUND_TRIP
            return;
#else
            QEXPECT_FAIL(0, "FreeSans found is *not* found! Hindi rendering might be broken!", Continue);
#endif
    }

    t.test_standard("text_hindi", QSize(260, 30), 72);
}

void TestSvgText::testTextBaselineShift()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"5\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"4\" y=\"29\" "
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"

            "        <tspan>text<tspan baseline-shift=\"super\">super </tspan>normal<tspan baseline-shift=\"sub\">sub</tspan></tspan>"

            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard("text_baseline_shift", QSize(180, 40), 72);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));

}

void TestSvgText::testTextSpacing()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"5\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"5\" y=\"24\" "
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"

            "        <tspan x=\"5\" dy=\"0.0em\">Lorem ipsum</tspan>"
            "        <tspan x=\"5\" dy=\"1.5em\" letter-spacing=\"4.0\">Lorem ipsum (ls=4)</tspan>"
            "        <tspan x=\"5\" dy=\"1.5em\" letter-spacing=\"-2.0\">Lorem ipsum (ls=-2)</tspan>"

            "        <tspan x=\"5\" dy=\"2.0em\">Lorem ipsum</tspan>"
            "        <tspan x=\"5\" dy=\"1.5em\" word-spacing=\"4.0\">Lorem ipsum (ws=4)</tspan>"
            "        <tspan x=\"5\" dy=\"1.5em\" word-spacing=\"-2.0\">Lorem ipsum (ws=-2)</tspan>"

            "        <tspan x=\"5\" dy=\"2.0em\">Lorem ipsum</tspan>"
            "        <tspan x=\"5\" dy=\"1.5em\" kerning=\"0.0\">Lorem ipsum (k=0)</tspan>"
            "        <tspan x=\"5\" dy=\"1.5em\" kerning=\"2.0\">Lorem ipsum (k=2)</tspan>"
            "        <tspan x=\"5\" dy=\"1.5em\" kerning=\"2.0\" letter-spacing=\"2.0\">Lorem ipsum (k=2,ls=2)</tspan>"

            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.setFuzzyThreshold(5);
    t.test_standard("text_letter_word_spacing", QSize(340, 250), 72.0);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));

}

void TestSvgText::testTextTabSpacing()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"5\" y=\"5\" width=\"89\" height=\"120\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"5\" y=\"24\" "
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"

            "        <tspan x=\"10\" dy=\"1.0em\">  Lorem</tspan>"
            "        <tspan x=\"10\" dy=\"2.0em\">	ipsum</tspan>"
            "        <tspan x=\"10\" dy=\"2.0em\">dolor  sit	amet,</tspan>"
            "        <tspan x=\"10\" dy=\"2.0em\">		consectetur adipiscing elit.</tspan>"

            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.setFuzzyThreshold(5);
    t.test_standard("text_tab_spacing", QSize(400, 170), 72.0);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));
}

void TestSvgText::testTextDecorations()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"5\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"4\" y=\"24\" "
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"

            "        <tspan x=\"20\" dy=\"0.0em\" text-decoration=\"underline\">Lorem ipsum</tspan>"
            "        <tspan x=\"20\" dy=\"2.5em\" text-decoration=\"overline\">Lorem ipsum</tspan>"
            "        <tspan x=\"20\" dy=\"2.0em\" text-decoration=\"line-through\">Lorem ipsum</tspan>"
            "        <tspan x=\"20\" dy=\"2.0em\" text-decoration=\"underline\">Lorem <tspan fill=\"red\">ipsum</tspan> (WRONG!!!)</tspan>"

            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.setFuzzyThreshold(5);
    t.test_standard("text_decorations", QSize(290, 135), 72.0);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));

}

void TestSvgText::testRightToLeft()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"5\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"20\" y=\"34\" "
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" text-anchor=\"end\">"

            "        <tspan x=\"250\" dy=\"0.0em\" text-anchor=\"middle\" direction=\"rtl\">aa bb cc dd</tspan>"
            "        <tspan x=\"250\" dy=\"2.0em\" text-anchor=\"middle\" direction=\"rtl\">حادثتا السفينتين «بسين Bassein» و«فايبر Viper»</tspan>"

            "        <tspan x=\"250\" dy=\"2.0em\" text-anchor=\"middle\">*</tspan>"

            "        <tspan x=\"150\" dy=\"2.0em\" text-anchor=\"start\" direction=\"ltr\">aa bb حادثتا السفينتين بسين cc dd </tspan>"
            "        <tspan x=\"350\" dy=\"2.0em\" text-anchor=\"start\" direction=\"rtl\">aa bb حادثتا السفينتين بسين cc dd </tspan>"

            "        <tspan x=\"250\" dy=\"2.0em\" text-anchor=\"middle\">*</tspan>"

            "        <tspan x=\"150\" dy=\"2.0em\" text-anchor=\"start\" direction=\"ltr\">aa bb <tspan text-decoration=\"underline\">حادثتا</tspan> السفينتين بسين cc dd </tspan>"
            "        <tspan x=\"250\" dy=\"2.0em\" text-anchor=\"middle\" direction=\"ltr\">aa bb <tspan text-decoration=\"underline\">حادثتا</tspan> السفينتين بسين cc dd </tspan>"
            "        <tspan x=\"350\" dy=\"2.0em\" text-anchor=\"end\" direction=\"ltr\">aa bb <tspan text-decoration=\"underline\">حادثتا</tspan> السفينتين بسين cc dd </tspan>"

            "        <tspan x=\"250\" dy=\"2.0em\" text-anchor=\"middle\">*</tspan>"

            "        <tspan x=\"250\" dy=\"2.0em\" text-anchor=\"middle\" direction=\"ltr\">الناطقون: 295 مليون - 422 مليون</tspan>"
            "        <tspan x=\"250\" dy=\"2.0em\" text-anchor=\"middle\" direction=\"ltr\">Spoken: <tspan direction=\"rtl\" unicode-bidi=\"embed\">295 مليون - 422 مليون </tspan></tspan>"

            "        <tspan x=\"250\" dy=\"2.0em\" text-anchor=\"middle\">*</tspan>"

            "        <tspan x=\"250\" dy=\"2.0em\" text-anchor=\"middle\" direction=\"ltr\">aa bb <tspan direction=\"rtl\" unicode-bidi=\"bidi-override\">c1 c2 c3 c4</tspan> dd ee</tspan>"



            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard("text_right_to_left", QSize(500,450), 72.0);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));

}

#include <QTextLayout>
#include <QPainter>

void TestSvgText::testQtBidi()
{
    // Arabic text sample from Wikipedia:
    // https://ar.wikipedia.org/wiki/%D8%A5%D9%85%D8%A7%D8%B1%D8%A7%D8%AA_%D8%A7%D9%84%D8%B3%D8%A7%D8%AD%D9%84_%D8%A7%D9%84%D9%85%D8%AA%D8%B5%D8%A7%D9%84%D8%AD

    QStringList ltrText;
    ltrText << "aa bb cc dd";
    ltrText << "aa bb حادثتا السفينتين بسين cc dd";
    ltrText << "aa bb \u202ec1c2 d3d4\u202C ee ff";

    QStringList rtlText;
    rtlText << "حادثتا السفينتين «بسين Bassein» و«فايبر Viper»";
    rtlText << "حادثتا السفينتين «بسين aa bb cc dd» و«فايبر Viper»";


    QImage canvas(500,500,QImage::Format_ARGB32);
    QPainter gc(&canvas);
    QPointF pos(15,15);


    QVector<QStringList> textSamples;
    textSamples << ltrText;
    textSamples << rtlText;

    QVector<Qt::LayoutDirection> textDirections;
    textDirections << Qt::LeftToRight;
    textDirections << Qt::RightToLeft;

    for (int i = 0; i < textSamples.size(); i++) {
        Q_FOREACH (const QString str, textSamples[i]) {
            QTextOption option;
            option.setTextDirection(textDirections[i]);
            option.setUseDesignMetrics(true);

            QTextLayout layout;

            layout.setText(str);
            layout.setFont(QFont("serif", 15.0));
            layout.setCacheEnabled(true);
            layout.beginLayout();

            QTextLine line = layout.createLine();
            line.setPosition(pos);
            pos.ry() += 25;
            layout.endLayout();
            layout.draw(&gc, QPointF());
        }
    }

    canvas.save("test_bidi.png");
}

void TestSvgText::testQtDxDy()
{
    QImage canvas(500,500,QImage::Format_ARGB32);
    QPainter gc(&canvas);
    QPointF pos(15,15);

    QTextOption option;
    option.setTextDirection(Qt::LeftToRight);
    option.setUseDesignMetrics(true);
    option.setWrapMode(QTextOption::WrapAnywhere);

    QTextLayout layout;

    layout.setText("aa bb cc dd ee ff");
    layout.setFont(QFont("serif", 15.0));
    layout.setCacheEnabled(true);
    layout.beginLayout();
    layout.setTextOption(option);

    {
        QTextLine line = layout.createLine();
        line.setPosition(pos);
        line.setNumColumns(4);
    }
    pos.ry() += 25;
    pos.rx() += 30;
    {
        QTextLine line = layout.createLine();
        line.setPosition(pos);
    }

    layout.endLayout();
    layout.draw(&gc, QPointF());


    canvas.save("test_dxdy.png");
}


void TestSvgText::testTextOutlineSolid()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"2\" y=\"24\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" stroke=\"red\" stroke-width=\"1\">"
            "        SA"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard("text_outline_solid", QSize(30, 30), 72.0);
}

void TestSvgText::testNbspHandling()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"2\" y=\"24\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" stroke=\"red\" stroke-width=\"1\">"
            "        S\u00A0A"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard("text_nbsp", QSize(30, 30), 72.0);
}

void TestSvgText::testMulticolorText()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"2\" y=\"24\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        S<tspan fill=\"red\">A</tspan>"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.setFuzzyThreshold(5);
    t.test_standard("text_multicolor", QSize(30, 30), 72.0);
}

#include <KoColorBackground.h>

void TestSvgText::testConvertToStrippedSvg()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text transform=\"translate(2)\" id=\"testRect\" x=\"2\" y=\"24\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        S<tspan fill=\"red\">A</tspan><![CDATA[some stuff<><><<<>]]>"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.parser.setResolution(QRectF(QPointF(), QSizeF(30,30)) /* px */, 72.0/* ppi */);
    t.run();

    KoSvgTextShape *baseShape = dynamic_cast<KoSvgTextShape*>(t.findShape("testRect"));
    QVERIFY(baseShape);

    {
        KoColorBackground *bg = dynamic_cast<KoColorBackground*>(baseShape->background().data());
        QVERIFY(bg);
        QCOMPARE(bg->color(), QColor(Qt::blue));
    }

    KoSvgTextShapeMarkupConverter converter(baseShape);

    QString svgText;
    QString stylesText;
    QVERIFY(converter.convertToSvg(&svgText, &stylesText));

    QCOMPARE(stylesText, QString("<defs/>"));
    QCOMPARE(svgText, QString("<text fill=\"#0000ff\" font-family=\"DejaVu Sans\" font-size=\"15\"><tspan x=\"2\" y=\"24\">S</tspan><tspan fill=\"#ff0000\">A</tspan><tspan>some stuff&lt;&gt;&lt;&gt;&lt;&lt;&lt;&gt;</tspan></text>"));

    // test loading

    svgText = "<text fill=\"#00ff00\" font-family=\"DejaVu Sans\" font-size=\"19\"><tspan x=\"2\" y=\"24\">S</tspan><tspan fill=\"#ff0000\">A</tspan><tspan>some stuff&lt;&gt;&lt;&gt;&lt;&lt;&lt;&gt;</tspan></text>";

    QVERIFY(converter.convertFromSvg(svgText, stylesText, QRectF(0,0,30,30), 72.0));

    {
        KoColorBackground *bg = dynamic_cast<KoColorBackground*>(baseShape->background().data());
        QVERIFY(bg);
        QCOMPARE(bg->color(), QColor(Qt::green));
    }

    {
        KoSvgTextProperties props = baseShape->textProperties();
        QVERIFY(props.hasProperty(KoSvgTextProperties::FontSizeId));

        const qreal fontSize = props.property(KoSvgTextProperties::FontSizeId).toReal();
        QCOMPARE(fontSize, 19.0);
    }

    QCOMPARE(baseShape->shapeCount(), 3);
}

void TestSvgText::testConvertToStrippedSvgNullOrigin()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text transform=\"translate(2)\" id=\"testRect\" x=\"0\" y=\"0\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        S<tspan fill=\"red\">A</tspan><![CDATA[some stuff<><><<<>]]>"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.parser.setResolution(QRectF(QPointF(), QSizeF(30,30)) /* px */, 72.0/* ppi */);
    t.run();

    KoSvgTextShape *baseShape = dynamic_cast<KoSvgTextShape*>(t.findShape("testRect"));
    QVERIFY(baseShape);

    KoSvgTextShapeMarkupConverter converter(baseShape);

    QString svgText;
    QString stylesText;
    QVERIFY(converter.convertToSvg(&svgText, &stylesText));

    QCOMPARE(stylesText, QString("<defs/>"));
    QCOMPARE(svgText, QString("<text fill=\"#0000ff\" font-family=\"DejaVu Sans\" font-size=\"15\"><tspan x=\"0\" y=\"0\">S</tspan><tspan fill=\"#ff0000\">A</tspan><tspan>some stuff&lt;&gt;&lt;&gt;&lt;&lt;&lt;&gt;</tspan></text>"));
}

void TestSvgText::testConvertFromIncorrectStrippedSvg()
{
    QScopedPointer<KoSvgTextShape> baseShape(new KoSvgTextShape());

    KoSvgTextShapeMarkupConverter converter(baseShape.data());

    QString svgText;
    QString stylesText;

    svgText = "<text>blah text</text>";
    QVERIFY(converter.convertFromSvg(svgText, stylesText, QRectF(0,0,30,30), 72.0));
    QCOMPARE(converter.errors().size(), 0);

    svgText = "<text>>><<><blah text</text>";
    QVERIFY(!converter.convertFromSvg(svgText, stylesText, QRectF(0,0,30,30), 72.0));
    qDebug() << ppVar(converter.errors());
    QCOMPARE(converter.errors().size(), 1);

    svgText = "<notext>blah text</notext>";
    QVERIFY(!converter.convertFromSvg(svgText, stylesText, QRectF(0,0,30,30), 72.0));
    qDebug() << ppVar(converter.errors());
    QCOMPARE(converter.errors().size(), 1);

    svgText = "<defs/>";
    QVERIFY(!converter.convertFromSvg(svgText, stylesText, QRectF(0,0,30,30), 72.0));
    qDebug() << ppVar(converter.errors());
    QCOMPARE(converter.errors().size(), 1);
}

void TestSvgText::testEmptyTextChunk()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"2\" y=\"24\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        " // no actual text! should not crash!
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    // it just shouldn't assert or fail when seeing an empty text block
    t.parser.setResolution(QRectF(QPointF(), QSizeF(30,30)) /* px */, 72.0/* ppi */);
    t.run();
}

void TestSvgText::testTrailingWhitespace()
{
    QStringList chunkA;
    chunkA << "aaa";
    chunkA << " aaa";
    chunkA << "aaa ";
    chunkA << " aaa ";

    QStringList chunkB;
    chunkB << "bbb";
    chunkB << " bbb";
    chunkB << "bbb ";
    chunkB << " bbb ";

    QStringList linkChunk;
    linkChunk << "";
    linkChunk << " ";
    linkChunk << "<tspan></tspan>";
    linkChunk << "<tspan> </tspan>";


    const QString dataTemplate =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"2\" y=\"24\""
            "        font-family=\"DejaVu Sans\" font-size=\"10\" fill=\"blue\" >"
            "        <tspan>%1</tspan>%2<tspan>%3</tspan>"
            "    </text>"

            "</g>"

            "</svg>";

    for (auto itL = linkChunk.constBegin(); itL != linkChunk.constEnd(); ++itL) {
        for (auto itA = chunkA.constBegin(); itA != chunkA.constEnd(); ++itA) {
            for (auto itB = chunkB.constBegin(); itB != chunkB.constEnd(); ++itB) {
                if (itA->rightRef(1) != " " &&
                    itB->leftRef(1) != " " &&
                    *itL != " " &&
                    *itL != linkChunk.last()) continue;

                QString cleanLink = *itL;
                cleanLink.replace('/', '_');

                qDebug() << "Testcase:" << *itA << cleanLink << *itB;

                const QString data = dataTemplate.arg(*itA, *itL, *itB);
                SvgRenderTester t (data);
                t.setFuzzyThreshold(5);
                //t.test_standard(QString("text_trailing_%1_%2_%3").arg(*itA).arg(cleanLink).arg(*itB), QSize(70, 30), 72.0);

                // all files should look exactly the same!
                t.test_standard(QString("text_whitespace"), QSize(70, 30), 72.0);
            }
        }
    }
}

void TestSvgText::testConvertHtmlToSvg()
{
    const QString html =
            "<?xml version=\"1.0\"?>"
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
            "<html>"
              "<head>"
                "<meta name=\"qrichtext\" content=\"1\"/>"
                "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style>"
              "</head>"
              "<body style=\" font-family:'Droid Sans'; font-size:9pt; font-weight:400; font-style:normal;\">"
                "<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "  <span style=\" font-family:'Times'; font-size:20pt;\">Lorem ipsum dolor</span>"
                "</p>"
                "<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">sit am"
                "<span style=\" font-weight:600;\">et, consectetur adipis</span>cing </p>"
                "<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "  <span style=\" font-style:italic;\">elit. </span>"
                "</p>"
              "</body>"
            "</html>";

    KoSvgTextShape shape;
    KoSvgTextShapeMarkupConverter converter(&shape);

    QString svg;
    QString defs;

    converter.convertFromHtml(html, &svg, &defs);


    bool r = converter.convertToSvg(&svg, &defs);

    qDebug() << r << svg << defs;

}

void TestSvgText::testTextWithMultipleRelativeOffsets()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"
            "    <text id=\"testRect\" x=\"10\" y=\"40\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" "
            "        dy=\"0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0\">"
            "        Lorem ipsum dolor sit amet"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.setFuzzyThreshold(5);
    t.test_standard("text_multiple_relative_offsets", QSize(300, 80), 72.0);
}

void TestSvgText::testTextWithMultipleAbsoluteOffsetsArabic()
{
    /**
     * According to the standard, each **absolute** offset defines a
     * new text chunk, therefore, the arabic text must become
     * ltr reordered
     */

    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"
            "    <text id=\"testRect\" x=\"10\" y=\"40\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" "
            "        y=\"40 45 50 55 50 45 40 35 30 25 30 35 40 45 50 55 50 45 40 35 30 25 30 35 40 45 50 55 50 45 40 35 30 25 30 35 40 45 50 55 50 45 40 35 30 25 30 35 40 45 50 55 50 45 40 35 30 25 30 35 40 45 50 55 50 45 40 35 30 25 30 35 \">"
            "        Lo rem اللغة العربية المعيارية الحديثة ip sum"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard("text_multiple_absolute_offsets_arabic", QSize(530, 70), 72.0);
}

void TestSvgText::testTextWithMultipleRelativeOffsetsArabic()
{
    /**
     * According to the standard, **relative** offsets must not define a new
     * text chunk, therefore, the arabic text must be written in native rtl order,
     * even though the individual letters are split.
     */

    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"
            "    <text id=\"testRect\" x=\"10\" y=\"40\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" "
            "        dy=\"0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0\">"
            "        Lo rem اللغة العربية المعيارية الحديثة ip sum"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    // we cannot expect more than one failure
#ifndef USE_ROUND_TRIP
    QEXPECT_FAIL("", "WARNING: in Krita relative offsets also define a new text chunk, that doesn't comply with SVG standard and must be fixed", Continue);
    t.test_standard("text_multiple_relative_offsets_arabic", QSize(530, 70), 72.0);
#endif
}

void TestSvgText::testTextOutline()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"7\" y=\"27\""
            " "
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        normal "
            "        <tspan text-decoration=\"line-through\">strikethrough</tspan>"
            "        <tspan text-decoration=\"overline\">overline</tspan>"
            "        <tspan text-decoration=\"underline\">underline</tspan>"
            "    </text>"

            "</g>"

            "</svg>";

    QRect renderRect(0, 0, 450, 40);

    SvgRenderTester t (data);
    t.setFuzzyThreshold(5);
    t.test_standard("text_outline", renderRect.size(), 72.0);

    KoShape *shape = t.findShape("testRect");
    KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
    QVERIFY(chunkShape);

    KoSvgTextShape *textShape = dynamic_cast<KoSvgTextShape*>(shape);

    QImage canvas(renderRect.size(), QImage::Format_ARGB32);
    canvas.fill(0);
    QPainter gc(&canvas);
    gc.setPen(Qt::NoPen);
    gc.setBrush(Qt::black);
    gc.setRenderHint(QPainter::Antialiasing, true);
    gc.drawPath(textShape->textOutline());

    QVERIFY(TestUtil::checkQImage(canvas, "svg_render", "load_text_outline", "converted_to_path", 3, 5));
}




QTEST_MAIN(TestSvgText)
