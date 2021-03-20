/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asl_parser_test.h"

#include <simpletest.h>

#include <testutil.h>
#include "testimage.h"

#include <QDomDocument>

#include <resources/KoPattern.h>

#include <asl/kis_asl_reader.h>
#include <asl/kis_asl_xml_parser.h>
#include <asl/kis_asl_object_catcher.h>
#include <asl/kis_asl_callback_object_catcher.h>

#include <kis_debug.h>

void KisAslParserTest::test()
{
    QString fileName(TestUtil::fetchDataFileLazy("asl/freebie.asl"));
    QFile aslFile(fileName);
    aslFile.open(QIODevice::ReadOnly);

    KisAslReader reader;
    QDomDocument doc = reader.readFile(&aslFile);

    dbgKrita << ppVar(doc.toString());

    KisAslObjectCatcher trivialCatcher;
    KisAslXmlParser parser;
    parser.parseXML(doc, trivialCatcher);
}

struct CallbackVerifier {
    CallbackVerifier() : m_numCallsHappened(0) {}

    void setColor(const QColor &color) {
        QVERIFY(color == QColor(Qt::white));
        m_numCallsHappened++;
    }

    void setOpacity(double opacity) {
        QVERIFY(qFuzzyCompare(opacity, 75));
        m_numCallsHappened++;
    }

    void setBlendingMode(const QString &mode) {
        QVERIFY(mode == "Scrn");
        m_numCallsHappened++;
    }

    void setEnabled(bool value) {
        QVERIFY(value);
        m_numCallsHappened++;
    }

    void setCurve(const QString &name, const QVector<QPointF> &points) {
        QCOMPARE(name, QString("Linear"));
        QCOMPARE(points[0], QPointF());
        QCOMPARE(points[1], QPointF(255.0, 255.0));
        m_numCallsHappened++;
    }

    void setText(const QString &text) {
        QCOMPARE(text, QString("11adf7a2-a120-11e1-957c-d1ee226781a4"));
        m_numCallsHappened++;
    }

    void setPattern(const KoPatternSP pattern) {
        dbgKrita << ppVar(pattern->name());
        dbgKrita << ppVar(pattern->filename());

        //QCOMPARE(text, QString("11adf7a2-a120-11e1-957c-d1ee226781a4"));
        m_numCallsHappened++;
    }

    int m_numCallsHappened;
};

void KisAslParserTest::testWithCallbacks()
{
    using namespace std::placeholders;
    QString fileName(TestUtil::fetchDataFileLazy("asl/freebie.asl"));
    QFile aslFile(fileName);
    aslFile.open(QIODevice::ReadOnly);


    KisAslReader reader;
    QDomDocument doc = reader.readFile(&aslFile);

    KisAslCallbackObjectCatcher c;

    CallbackVerifier verifier;

    c.subscribeColor("/Styl/Lefx/IrGl/Clr ", std::bind(&CallbackVerifier::setColor, &verifier, _1));
    c.subscribeUnitFloat("/Styl/Lefx/IrGl/Opct", "#Prc", std::bind(&CallbackVerifier::setOpacity, &verifier, _1));
    c.subscribeEnum("/Styl/Lefx/IrGl/Md  ", "BlnM", std::bind(&CallbackVerifier::setBlendingMode, &verifier, _1));
    c.subscribeBoolean("/Styl/Lefx/IrGl/enab", std::bind(&CallbackVerifier::setEnabled, &verifier, _1));
    c.subscribeCurve("/Styl/Lefx/OrGl/TrnS", std::bind(&CallbackVerifier::setCurve, &verifier, _1, _2));
    c.subscribeText("/null/Idnt", std::bind(&CallbackVerifier::setText, &verifier, _1));

    KisAslXmlParser parser;
    parser.parseXML(doc, c);

    QCOMPARE(verifier.m_numCallsHappened, 6);
}

#include <asl/kis_asl_xml_writer.h>

void KisAslParserTest::testASLXMLWriter()
{
    KisAslXmlWriter w;

    QImage testImage(QSize(16, 16), QImage::Format_ARGB32);
    KoPatternSP testPattern1(new KoPattern(testImage, "Some very nice name ;)", ""));
    KoPatternSP testPattern2(new KoPattern(testImage, "Another very nice name ;P", ""));

    w.enterList(ResourceType::Patterns);
    w.writePattern("", testPattern1);
    w.writePattern("", testPattern2);
    w.leaveList();

    w.enterDescriptor("", "", "null");
    w.writeText("Nm  ", "www.designpanoply.com - Freebie 5");
    w.writeText("Idnt", "11adf7a2-a120-11e1-957c-d1ee226781a4");
    w.leaveDescriptor();

    w.enterDescriptor("", "", "Styl");

    w.enterDescriptor("documentMode", "", "documentMode");
    w.leaveDescriptor();

    w.enterDescriptor("Lefx", "", "Lefx");

    w.writeUnitFloat("Scl ", "#Prc", 100);
    w.writeBoolean("masterFxSwitch", true);

    w.enterDescriptor("DrSh", "", "DrSh");

    w.writeBoolean("enab", true);
    w.writeEnum("Md  ", "BlnM", "Mltp");
    w.writeColor("Clr ", Qt::green);


    w.writeUnitFloat("Opct", "#Prc", 16);
    w.writeBoolean("uglg", false);
    w.writeUnitFloat("lagl", "#Prc", 100);
    w.writeUnitFloat("Dstn", "#Pxl", 100);
    w.writeUnitFloat("Ckmt", "#Pxl", 100);
    w.writeUnitFloat("blur", "#Pxl", 100);
    w.writeUnitFloat("Nose", "#Prc", 100);

    w.writeBoolean("anta", true);

    w.writeCurve("TrnS",
                 "Linear",
                 QVector<QPointF>() << QPointF() << QPointF(255, 255));

    w.writeBoolean("layerConceals", true);

    w.leaveDescriptor();

    w.leaveDescriptor();

    w.leaveDescriptor();

    dbgKrita << ppVar(w.document().toString());

}

#include <resources/KoStopGradient.h>
#include <resources/KoSegmentGradient.h>

void KisAslParserTest::testWritingGradients()
{
    KisAslXmlWriter w1;

    KoSegmentGradient segmentGradient;
    segmentGradient.createSegment(INTERP_LINEAR, COLOR_INTERP_RGB,
                                  0.0, 0.3, 0.15,
                                  Qt::black, Qt::red);
    segmentGradient.createSegment(INTERP_LINEAR, COLOR_INTERP_RGB,
                                  0.3, 0.6, 0.45,
                                  Qt::red, Qt::green);
    segmentGradient.createSegment(INTERP_LINEAR, COLOR_INTERP_RGB,
                                  0.6, 1.0, 0.8,
                                  Qt::green, Qt::white);

    w1.writeSegmentGradient("tstG", &segmentGradient);
    //dbgKrita << "===";
    //dbgKrita << ppVar(w1.document().toString());

    KisAslXmlWriter w2;

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QList<KoGradientStop> stops;
    stops << KoGradientStop(0.0, KoColor(Qt::black, cs), COLORSTOP);
    stops << KoGradientStop(0.3, KoColor(Qt::red, cs), COLORSTOP);
    stops << KoGradientStop(0.6, KoColor(Qt::green, cs), COLORSTOP);
    stops << KoGradientStop(1.0, KoColor(Qt::white, cs), COLORSTOP);

    KoStopGradient stopGradient;
    stopGradient.setStops(stops);

    w2.writeStopGradient("tstG", &stopGradient);

    //dbgKrita << "===";
    //dbgKrita << ppVar(w2.document().toString());

    QCOMPARE(w1.document().toString(),
             w2.document().toString());

}

#include <asl/kis_asl_writer.h>

void KisAslParserTest::testASLWriter()
{
    //QString srcFileName(TestUtil::fetchDataFileLazy("asl/testset/freebie_with_pattern.asl"));
    QString srcFileName(TestUtil::fetchDataFileLazy("asl/freebie.asl"));

    QDomDocument srcDoc;

    {
        QFile srcAslFile(srcFileName);
        srcAslFile.open(QIODevice::ReadOnly);

        KisAslReader reader;
        srcDoc = reader.readFile(&srcAslFile);


        QFile tfile("src_parsed.xml");
        tfile.open(QIODevice::WriteOnly);
        tfile.write(srcDoc.toByteArray());
        tfile.close();
    }

    QString dstFileName("test.asl");

    {
        QFile dstAslFile(dstFileName);
        dstAslFile.open(QIODevice::WriteOnly);

        KisAslWriter writer;
        writer.writeFile(&dstAslFile, srcDoc);

        dstAslFile.flush();
        dstAslFile.close();
    }

    QDomDocument dstDoc;

    {
        QFile roundTripAslFile(dstFileName);
        roundTripAslFile.open(QIODevice::ReadOnly);

        KisAslReader reader;
        dstDoc = reader.readFile(&roundTripAslFile);

        QFile tfile("dst_parsed.xml");
        tfile.open(QIODevice::WriteOnly);
        tfile.write(dstDoc.toByteArray());
        tfile.close();
    }

    QCOMPARE(srcDoc.toByteArray(), dstDoc.toByteArray());
}

void KisAslParserTest::testParserWithPatterns()
{
    QDir dir(QString(FILES_DATA_DIR) + '/' + "testset");

    QFileInfoList files = dir.entryInfoList(QStringList() << "*.asl", QDir::Files);


    int index = 0;

    Q_FOREACH (const QFileInfo &fileInfo, files) {

        //if (index != 12) {index++; continue;}

        dbgKrita << "===" << index << "===";
        dbgKrita << ppVar(fileInfo.fileName());

        QFile aslFile(fileInfo.absoluteFilePath());
        aslFile.open(QIODevice::ReadOnly);

        KisAslReader reader;
        QDomDocument doc = reader.readFile(&aslFile);

        QFile xmlFile("mydata.xml");
        xmlFile.open(QIODevice::WriteOnly);
        xmlFile.write(doc.toByteArray());

        //dbgKrita << ppVar(doc.toString());

        CallbackVerifier verifier;
        KisAslCallbackObjectCatcher c;

        c.subscribePattern("/Patterns/KisPattern", std::bind(&CallbackVerifier::setPattern, &verifier, std::placeholders::_1));
        c.subscribePattern("/patterns/KisPattern", std::bind(&CallbackVerifier::setPattern, &verifier, std::placeholders::_1));

        KisAslXmlParser parser;
        parser.parseXML(doc, c);

        //QCOMPARE(verifier.m_numCallsHappened, 7);

        index++;
        //break;
    }
}

KISTEST_MAIN(KisAslParserTest)
