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

#include "kis_asl_parser_test.h"

#include <qtest_kde.h>

#include "testutil.h"

#include <boost/bind.hpp>

#include <QDomDocument>

#include "KoPattern.h"

#include "../kis_asl_reader.h"
#include "../kis_asl_xml_parser.h"
#include "../kis_asl_object_catcher.h"
#include "../kis_asl_callback_object_catcher.h"


void KisAslParserTest::test()
{
    QString fileName(TestUtil::fetchDataFileLazy("freebie.asl"));
    QFile aslFile(fileName);
    aslFile.open(QIODevice::ReadOnly);

    KisAslReader reader;
    QDomDocument doc = reader.readFile(&aslFile);

    qDebug() << ppVar(doc.toString());

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

    void setPattern(KoPattern *pattern) {
        qDebug() << ppVar(pattern->name());
        qDebug() << ppVar(pattern->filename());

        //QCOMPARE(text, QString("11adf7a2-a120-11e1-957c-d1ee226781a4"));
        m_numCallsHappened++;
    }

    int m_numCallsHappened;
};

void KisAslParserTest::testWithCallbacks()
{
    QString fileName(TestUtil::fetchDataFileLazy("freebie.asl"));
    QFile aslFile(fileName);
    aslFile.open(QIODevice::ReadOnly);


    KisAslReader reader;
    QDomDocument doc = reader.readFile(&aslFile);

    KisAslCallbackObjectCatcher c;

    CallbackVerifier verifier;

    c.subscribeColor("/Styl/Lefx/IrGl/Clr ", boost::bind(&CallbackVerifier::setColor, &verifier, _1));
    c.subscribeUnitFloat("/Styl/Lefx/IrGl/Opct", "#Prc", boost::bind(&CallbackVerifier::setOpacity, &verifier, _1));
    c.subscribeEnum("/Styl/Lefx/IrGl/Md  ", "BlnM", boost::bind(&CallbackVerifier::setBlendingMode, &verifier, _1));
    c.subscribeBoolean("/Styl/Lefx/IrGl/enab", boost::bind(&CallbackVerifier::setEnabled, &verifier, _1));
    c.subscribeCurve("/Styl/Lefx/OrGl/TrnS", boost::bind(&CallbackVerifier::setCurve, &verifier, _1, _2));
    c.subscribeText("/null/Idnt", boost::bind(&CallbackVerifier::setText, &verifier, _1));

    KisAslXmlParser parser;
    parser.parseXML(doc, c);

    QCOMPARE(verifier.m_numCallsHappened, 6);
}

#include "../kis_asl_xml_writer.h"

void KisAslParserTest::testASLXMLWriter()
{
    KisAslXmlWriter w;

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

    qDebug() << ppVar(w.document().toString());

}

#include "../kis_asl_writer.h"

void KisAslParserTest::testASLWriter()
{
    QString srcFileName(TestUtil::fetchDataFileLazy("freebie.asl"));

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
    QDir dir(FILES_DATA_DIR + QDir::separator() + "testset");

    QFileInfoList files = dir.entryInfoList(QStringList() << "*.asl", QDir::Files);


    int index = 0;

    foreach (const QFileInfo &fileInfo, files) {

        //if (index != 12) {index++; continue;}

        qDebug() << "===" << index << "===";
        qDebug() << ppVar(fileInfo.fileName());

        QFile aslFile(fileInfo.absoluteFilePath());
        aslFile.open(QIODevice::ReadOnly);

        KisAslReader reader;
        QDomDocument doc = reader.readFile(&aslFile);

        QFile xmlFile("mydata.xml");
        xmlFile.open(QIODevice::WriteOnly);
        xmlFile.write(doc.toByteArray());

        //qDebug() << ppVar(doc.toString());

        CallbackVerifier verifier;
        KisAslCallbackObjectCatcher c;

        c.subscribePattern("/Patterns/KisPattern", boost::bind(&CallbackVerifier::setPattern, &verifier, _1));

        KisAslXmlParser parser;
        parser.parseXML(doc, c);

        //QCOMPARE(verifier.m_numCallsHappened, 7);

        index++;
        //break;
    }
}

QTEST_KDEMAIN(KisAslParserTest, GUI)
