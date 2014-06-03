/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#include "MusicXmlReader.h"
#include "Sheet.h"
#include "Part.h"
#include "Chord.h"
#include "Voice.h"
#include "VoiceBar.h"
#include "KeySignature.h"
#include "Bar.h"
#include "Clef.h"
#include "TimeSignature.h"
#include "Note.h"

#include <KoXmlReader.h>

#include <kdebug.h>

#include <QHash>

#include <math.h>

namespace MusicCore {

MusicXmlReader::MusicXmlReader(const char* musicNamespace)
    : m_namespace(musicNamespace)
{
}

static Duration parseDuration(const QString& type, int length, int div)
{
    if (type == "128th")        return HundredTwentyEighthNote;
    else if (type == "64th")    return SixtyFourthNote;
    else if (type == "32nd")    return ThirtySecondNote;
    else if (type == "16th")    return SixteenthNote;
    else if (type == "eighth")  return EighthNote;
    else if (type == "quarter") return QuarterNote;
    else if (type == "half")    return HalfNote;
    else if (type == "whole")   return WholeNote;
    else if (type == "breve")   return BreveNote;
    
    // else try to parse it from length
    qreal fact = 26880.0 / div;
    int ticks = (int) round(length * fact);
    // TODO: take number of dots into account
    if (ticks <= Note128Length)       return HundredTwentyEighthNote;
    else if (ticks <= Note64Length)   return SixtyFourthNote;
    else if (ticks <= Note32Length)   return ThirtySecondNote;
    else if (ticks <= Note16Length)   return SixteenthNote;
    else if (ticks <= Note8Length)    return EighthNote;
    else if (ticks <= QuarterLength)  return QuarterNote;
    else if (ticks <= HalfLength)     return HalfNote;
    else if (ticks <= WholeLength)    return WholeNote;
    else                                            return BreveNote;
}

Sheet* MusicXmlReader::loadSheet(const KoXmlElement& scoreElement)
{
    Sheet* sheet = new Sheet();
    
    QHash<QString, Part*> parts;
    
    KoXmlElement partList = namedItem(scoreElement, "part-list");
    if (partList.isNull()) {
        //kDebug() << "no part list found";
        return 0;
    }
    KoXmlElement elem;
    forEachElement(elem, partList) {
        if (checkNamespace(elem) && elem.localName() == "score-part") {
            QString id = elem.attribute("id");
            QString name = getProperty(elem, "part-name");
            QString abbr = getProperty(elem, "part-abbreviation");
            Part* p = sheet->addPart(name);
            p->setShortName(abbr);
            // always add one voice and one staff
            p->addVoice();
            p->addStaff();
            parts[id] = p;
        }
    }
    
    forEachElement(elem, scoreElement) {
        if (checkNamespace(elem) && elem.localName() == "part") {
            QString id = elem.attribute("id");
            loadPart(elem, parts[id]);
        }
    }
    
    return sheet;
}


QString MusicXmlReader::getProperty(const KoXmlElement& elem, const char *propName)
{
    KoXmlElement propElem = namedItem(elem, propName);
    return propElem.text();
}

KoXmlElement MusicXmlReader::namedItem(const KoXmlNode& node, const char* localName)
{
    if (m_namespace) {
        return KoXml::namedItemNS(node, m_namespace, localName);
    } else {
        return node.namedItem(localName).toElement();
    }
}

bool MusicXmlReader::checkNamespace(const KoXmlNode& node)
{
    return !m_namespace || node.namespaceURI() == m_namespace;
}

Clef* MusicXmlReader::loadClef(const KoXmlElement& element, Staff* staff)
{
    QString shapeStr = getProperty(element, "sign");
    Clef::ClefShape shape = Clef::GClef;
    int line = 2;
    if (shapeStr == "G") {
        line = 2;
        shape = Clef::GClef;
    } else if (shapeStr == "F") {
        line = 4;
        shape = Clef::FClef;
    } else if (shapeStr == "C") {
        line = 3;
        shape = Clef::CClef;
    }
    
    QString lineStr = getProperty(element, "line");
    if (!lineStr.isNull()) line = lineStr.toInt();
    
    int octave = 0;
    QString octaveStr = getProperty(element, "clef-octave-change");
    if (!octaveStr.isNull()) octave = octaveStr.toInt();
    
    return new Clef(staff, 0, shape, line, octave);
}

TimeSignature* MusicXmlReader::loadTimeSignature(const KoXmlElement& element, Staff* staff)
{
    int beats = getProperty(element, "beats").toInt();
    int beat = getProperty(element, "beat-type").toInt();
    
    return new TimeSignature(staff, 0, beats, beat);
}

void MusicXmlReader::loadPart(const KoXmlElement& partElement, Part* part)
{
    Sheet* sheet = part->sheet();

    KoXmlElement barElem;

    int curBar = 0;
    int curDivisions = 26880;
    Chord* lastNote = NULL;

    forEachElement(barElem, partElement) {
        if (!checkNamespace(barElem) || barElem.localName() != "measure") continue;

        Bar* bar = NULL;
        if (curBar >= sheet->barCount()) {
            bar = sheet->addBar();
        } else {
            bar = sheet->bar(curBar);
        }

        QList<QList<Chord*> > beams;
        for (int i = 0; i < 6; i++) beams.append(QList<Chord*>());
        
        KoXmlElement e;
        forEachElement(e, barElem) {
            if (!checkNamespace(e)) continue;

            if (e.localName() == "attributes") {
                KoXmlElement attr;

                QString staffCountStr = getProperty(e, "staves");
                if (!staffCountStr.isNull()) {
                    int staffCount = staffCountStr.toInt();
                    while (staffCount > part->staffCount()) {
                        part->addStaff();
                    }
                }
                
                forEachElement(attr, e) {
                    if (!checkNamespace(attr)) continue;
                    
                    if (attr.localName() == "divisions") {
                        curDivisions = attr.text().toInt();
                    } else if (attr.localName() == "key") {
                        QString number = attr.attribute("number");
                        int firstStaffId = 0;
                        int lastStaffId = part->staffCount()-1;
                        if (!number.isNull()) firstStaffId = lastStaffId = number.toInt() - 1;
                        
                        for (int staffId = firstStaffId; staffId <= lastStaffId; staffId++) {
                            KeySignature* ks = new KeySignature(part->staff(staffId), 0, getProperty(attr, "fifths").toInt());
                            bar->addStaffElement(ks);
                        }
                    } else if (attr.localName() == "clef") {
                        QString number = attr.attribute("number");
                        int staffId = 0;
                        if (!number.isNull()) staffId = number.toInt() - 1;
                        
                        Clef* clef = loadClef(attr, part->staff(staffId));
                        bar->addStaffElement(clef);
                    } else if (attr.localName() == "time") {
                        QString number = attr.attribute("number");
                        int firstStaffId = 0;
                        int lastStaffId = part->staffCount()-1;
                        if (!number.isNull()) firstStaffId = lastStaffId = number.toInt() - 1;
 
                        for (int staffId = firstStaffId; staffId <= lastStaffId; staffId++) {                       
                            TimeSignature* ts = loadTimeSignature(attr, part->staff(staffId));
                            bar->addStaffElement(ts);
                        }
                    }
                }
                
            } else if (e.localName() == "note") {
                QString staffStr = getProperty(e, "staff");
                int staffId = 0;
                if (!staffStr.isNull()) staffId = staffStr.toInt() - 1;
                                
                if (namedItem(e, "chord").isNull()) {
                    // no chord element, so this is the start of a new chord
                    int length = getProperty(e, "duration").toInt();
                    QString type = getProperty(e, "type");
                    Duration duration = parseDuration(type, length, curDivisions);
                    
                    QString voiceStr = getProperty(e, "voice");
                    int voiceId = 0;
                    if (!voiceStr.isNull()) voiceId = voiceStr.toInt() - 1;
                    while (voiceId >= part->voiceCount()) {
                        part->addVoice();
                    }
                    
                    int nDots = 0;
                    KoXmlElement dot;
                    forEachElement(dot, e) {
                        if (checkNamespace(dot) && dot.localName() == "dot") nDots++;
                    }
                    
                    Staff* staff = part->staff(staffId);
                    lastNote = new Chord(staff, duration, nDots);
                    Voice* voice = part->voice(voiceId);
                    voice->bar(bar)->addElement(lastNote);
                    
                    KoXmlElement beam;
                    forEachElement(beam, e) if (checkNamespace(beam) && beam.localName() == "beam") {
                        int number = beam.attribute("number").toInt() - 1;
                        if (number < 0 || number > 5) continue;
                        QString type = beam.text();
                        if (type == "begin") {
                            beams[number].clear();
                            beams[number].append(lastNote);
                        } else if (type == "continue") {
                            beams[number].append(lastNote);
                        } else if (type == "end") {
                            beams[number].append(lastNote);
                            Chord* startNote = beams[number][0];
                            foreach (Chord* c, beams[number]) {
                                c->setBeam(number, startNote, lastNote);
                            }
                            beams[number].clear();
                        } else if (type == "forward hook") {
                            lastNote->setBeam(number, lastNote, lastNote, BeamForwardHook);
                            beams[number].clear();
                        } else if (type == "backward hook") {
                            lastNote->setBeam(number, lastNote, lastNote, BeamBackwardHook);
                            beams[number].clear();
                        }
                    }
                    
                }

                KoXmlElement pitch = namedItem(e, "pitch");
                if (!pitch.isNull()) {
                    QString step = getProperty(pitch, "step");
                    int octave = getProperty(pitch, "octave").toInt();
                    int note = step[0].toLatin1() - 'A';
                    note -= 2;
                    if (note < 0) note += 7;
                    note += (octave - 4) * 7;
                    
                    int alter = 0;
                    QString alterStr = getProperty(pitch, "alter");
                    if (!alterStr.isNull()) alter = alterStr.toInt();
                    
                    QString accidental = getProperty(e, "accidental");
                    if (accidental == "double-sharp" || accidental == "sharp-sharp") {
                        alter = 2;
                    } else if (accidental == "sharp" || accidental == "natural-sharp") {
                        alter = 1;
                    } else if (accidental == "natural") {
                        alter = 0;
                    } else if (accidental == "flat" || accidental == "natural-flat") {
                        alter = -1;
                    } else if (accidental == "double-flat" || accidental == "flat-flat") {
                        alter = -2;
                    }
                    
                    Note* theNote = lastNote->addNote(part->staff(staffId), note, alter);
                    
                    KoXmlElement tie = namedItem(e, "tie");
                    if (!tie.isNull()) {
                        if (tie.attribute("type") == "start") {
                            theNote->setStartTie(true);
                        }
                    }                    
                }
            }
        }

        curBar++;
    }
}


} // namespace MusicCore
