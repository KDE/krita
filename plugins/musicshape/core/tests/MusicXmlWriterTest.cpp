/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "Sheet.h"
#include "Part.h"
#include "PartGroup.h"
#include "MusicXmlWriter.h"
#include "VoiceBar.h"
#include "Bar.h"
#include "Chord.h"
#include "Note.h"
#include <KoXmlWriter.h>
#include <KoXmlReader.h>

#include <QtTest/QtTest>
#include <QBuffer>

using namespace MusicCore;

bool compareNodes(KoXmlNode& valid, KoXmlNode& result, QString path = QString());
bool validateOutput(MusicCore::Sheet* sheet, const char* fname);

class MusicXmlWriterTest : public QObject
{
    Q_OBJECT
private slots:
    void init() 
    {
    }
    
    void cleanup() 
    {
    }
    
    void testParts()
    {
        Sheet* sheet = new Sheet();
        Part* p = sheet->addPart("first part");
        p->setShortName("part1");
        sheet->addPart("second part");
        
        sheet->addBar();
        
        validateOutput(sheet, "parts.xml");
        delete sheet;
    }

    void testPartGroups()
    {
        Sheet* sheet = new Sheet();
        sheet->addBar();
        for (int i = 0; i < 8; i++) {
            sheet->addPart(QString("part %1").arg(i));
        }
        
        PartGroup* pg = sheet->addPartGroup(0, 1);
        pg->setName("group 1");
        
        pg = sheet->addPartGroup(2, 3);
        pg->setSymbol(PartGroup::Brace);
        
        pg = sheet->addPartGroup(4, 5);
        pg->setSymbol(PartGroup::Line);
        
        pg = sheet->addPartGroup(6, 7);
        pg->setSymbol(PartGroup::Bracket);
        pg->setCommonBarLines(false);
        
        validateOutput(sheet, "partgroups.xml");
        delete sheet;
    }

    void testNestedPartGroups()
    {
        Sheet* sheet = new Sheet();
        sheet->addBar();
        for (int i = 0; i < 7; i++) {
            sheet->addPart(QString("part %1").arg(i));
        }
        
        sheet->addPartGroup(0, 1)->setName("group 1");
        sheet->addPartGroup(1, 2)->setName("group 2");
        sheet->addPartGroup(2, 3)->setName("group 3");
        
        sheet->addPartGroup(0, 6)->setName("group 4");
        sheet->addPartGroup(4, 5)->setName("group 5");
        sheet->addPartGroup(4, 5)->setName("group 6");
        
        validateOutput(sheet, "nestedpartgroups.xml");
        delete sheet;
    }
    
    void testNoteDurations()
    {
        Sheet* sheet = new Sheet();
        Bar* bar = sheet->addBar();
        Part* part = sheet->addPart("part");
        Voice* voice = part->addVoice();
        Staff* staff = part->addStaff();
        VoiceBar* vb = bar->voice(voice);
        
        for (Duration d = HundredTwentyEighthNote; d <= BreveNote; d = (Duration)(d + 1)) {
            Chord* c = new Chord(d);
            c->addNote(staff, 0);
            vb->addElement(c);
        }
        for (int i = 1; i < 4; i++) {
            Chord* c = new Chord(QuarterNote, i);
            c->addNote(staff, 0);
            vb->addElement(c);
        }
        
        validateOutput(sheet, "notedurations.xml");
        delete sheet;
    }

    void testNotePitch()
    {
        Sheet* sheet = new Sheet();
        Bar* bar = sheet->addBar();
        Part* part = sheet->addPart("part");
        Voice* voice = part->addVoice();
        Staff* staff = part->addStaff();
        VoiceBar* vb = bar->voice(voice);
        
        for (int p = -20; p <= 20; p++) {
            Chord* c = new Chord(QuarterNote);
            c->addNote(staff, p);
            vb->addElement(c);
        }
        
        validateOutput(sheet, "notepitch.xml");
        delete sheet;
    }
        
    void testNoteAccidentals()
    {
        Sheet* sheet = new Sheet();
        Bar* bar = sheet->addBar();
        Part* part = sheet->addPart("part");
        Voice* voice = part->addVoice();
        Staff* staff = part->addStaff();
        VoiceBar* vb = bar->voice(voice);
        
        for (int a = -2; a <= 2; a++) {
            Chord* c = new Chord(QuarterNote);
            c->addNote(staff, 0, a);
            vb->addElement(c);
        }
        
        validateOutput(sheet, "noteaccidentals.xml");
        delete sheet;
    }
};


#define FAIL(message) do { QTest::qFail(message, __FILE__, __LINE__); return false; } while (0)
bool compareNodes(KoXmlNode& valid, KoXmlNode& result, QString path)
{
    path += '/' + valid.nodeName();
    
    if (result.localName() != valid.localName()) {
        FAIL(QString("nodeName does not match at %1; expected %2, received %3").arg(path, valid.nodeName(), result.nodeName()).toLocal8Bit().constData());
    }

    if (result.namespaceURI() != valid.namespaceURI()) {
        FAIL(QString("namespace does not match at %1; expected %2, received %3").arg(path, valid.namespaceURI(), result.namespaceURI()).toLocal8Bit().constData());
    }

    if (result.isCDATASection()) {
        if (!valid.isCDATASection()) {
            FAIL(QString("node value types differ").toLocal8Bit().constData());
        } else {
            if (result.toCDATASection().data() != valid.toCDATASection().data()) {
                FAIL(QString("node value does not match at %1; expected %2, received %3").arg(path, valid.toCDATASection().data(), result.toCDATASection().data()).toLocal8Bit().constData());
            }
        }
    } else if (result.isText()) {
        if (!valid.isText()) {
            FAIL(QString("node value types differ").toLocal8Bit().constData());
        } else {
            if (result.toText().data() != valid.toText().data()) {
                FAIL(QString("node value does not match at %1; expected %2, received %3").arg(path, valid.toText().data(), result.toText().data()).toLocal8Bit().constData());
            }
        }
    }

    // if comparing identification nodes, simply return as the contents is not really relevant
    if (result.nodeName() == "identification") return true;

    // compare attributes
    KoXmlElement r = result.toElement();
    KoXmlElement v = valid.toElement();
    if (!r.isNull() && !v.isNull()) {
        foreach (QString attr, KoXml::attributeNames(v)) {
            if (r.attribute(attr) != v.attribute(attr)) {
                FAIL(QString("incorrect attribute %1 for %2; expected %3, received %4").arg(attr, path, v.attribute(attr), r.attribute(attr)).toLocal8Bit().constData());
            }
        }
        
        foreach (QString attr, KoXml::attributeNames(r)) {
            if (!v.hasAttribute(attr)) {
                FAIL(QString("incorrect attribute %1 for %2; expected %3, received %4").arg(attr, path, v.attribute(attr), r.attribute(attr)).toLocal8Bit().constData());
            }
        }
    }

    // compare child nodes
    if (KoXml::childNodesCount(result) != KoXml::childNodesCount(valid)) {
        FAIL(QString("childNodesCount does not match at %1; expected %2, received %3")
            .arg(path).arg(KoXml::childNodesCount(valid)).arg(KoXml::childNodesCount(result)).toLocal8Bit().constData());
    }

    int idx = 0;
    for (KoXmlNode rChild = result.firstChild(), vChild = valid.firstChild(); !rChild.isNull() || !vChild.isNull(); rChild = rChild.nextSibling(), vChild = vChild.nextSibling()) {
        if (!compareNodes(vChild, rChild, (path + "[%1]").arg(idx++))) return false;
    }

    return true;
}

bool validateOutput(Sheet* sheet, const char* fname)
{
    MusicCore::MusicXmlWriter writer;
    QIODevice* dev = new QBuffer();
    dev->open(QIODevice::ReadWrite);
    KoXmlWriter xmlWriter(dev);

    xmlWriter.startDocument("score-partwise", "-//Recordare//DTD MusicXML 1.1 Partwise//EN",
                              "http://www.musicxml.org/dtds/partwise.dtd");
                              
    writer.writeSheet(xmlWriter, sheet);
    xmlWriter.endDocument();
    
    
    QFile validFile(QString(KDESRCDIR "/files/%1").arg(fname));
    validFile.open(QIODevice::ReadOnly);
    KoXmlDocument valid;
    KoXml::setDocument(valid, &validFile, true);

    KoXmlDocument result;
    KoXml::setDocument(result, dev, true);

    bool res = compareNodes(valid, result);
    if (!res) {
        QFile f(QString(KDESRCDIR "/files/out_%1").arg(fname));
        f.open(QIODevice::WriteOnly);
        f.write(((QBuffer*)dev)->data());
        f.close();
    }

    delete dev;

    return res;
}

QTEST_MAIN(MusicXmlWriterTest)

#include <MusicXmlWriterTest.moc>
