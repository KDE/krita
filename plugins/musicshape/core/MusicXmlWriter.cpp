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
#include "MusicXmlWriter.h"
#include "Global.h"
#include "Sheet.h"
#include "Part.h"
#include "PartGroup.h"
#include "VoiceElement.h"
#include "Chord.h"
#include "VoiceBar.h"
#include "Bar.h"
#include "Note.h"
#include "Clef.h"
#include "KeySignature.h"
#include "TimeSignature.h"
#include "Staff.h"

#include <KoXmlWriter.h>

using namespace MusicCore;

MusicXmlWriter::MusicXmlWriter()
{
}

MusicXmlWriter::~MusicXmlWriter()
{
}

static void writePartGroup(KoXmlWriter& w, int id, PartGroup* group)
{
    w.startElement("music:part-group");
    w.addAttribute("type", "start");
    w.addAttribute("number", id);

    if (!group->name().isNull()) {
        w.startElement("music:group-name");
        w.addTextNode(group->name());
        w.endElement(); // music:group-name
    }
    if (!group->shortName(false).isNull()) {
        w.startElement("music:group-abbreviation");
        w.addTextNode(group->shortName());
        w.endElement(); // music:group-abbreviation
    }

    if (group->symbol() != PartGroup::None) {
        w.startElement("music:group-symbol");
        switch (group->symbol()) {
            case PartGroup::None:       w.addTextNode("none");   break;
            case PartGroup::Brace:      w.addTextNode("brace");  break;
            case PartGroup::Line:       w.addTextNode("line");   break;
            case PartGroup::Bracket:    w.addTextNode("bracket"); break;
        }
        w.endElement(); // music:group-symbol
    }

    w.startElement("music:group-barline");
    w.addTextNode(group->commonBarLines() ? "yes" : "no");
    w.endElement(); // music:group-barline

    w.endElement(); // music:part-group
}

static void writePartDesc(KoXmlWriter& w, int id, Part* part)
{
    w.startElement("music:score-part");
    w.addAttribute("id", QString("P%1").arg(id));

    w.startElement("music:part-name");
    w.addTextNode(part->name());
    w.endElement(); // music:part-name

    QString abbr = part->shortName(false);
    if (!abbr.isNull()) {
        w.startElement("music:part-abbreviation");
        w.addTextNode(abbr);
        w.endElement(); // music:part-abbreviation
    }

    w.endElement(); // music:score-part
}

static void writeChord(KoXmlWriter& w, Chord* chord, Voice* voice, Part* part, int bar)
{
    if (!chord->noteCount()) {
        w.startElement("music:note");

        w.startElement("music:rest");
        w.endElement();  // music:rest

        w.startElement("music:duration");
        w.addTextNode(QString::number(chord->length()));
        w.endElement(); // music:duration
        
        w.startElement("music:voice");
        w.addTextNode(QString::number(part->indexOfVoice(voice) + 1));
        w.endElement(); // music:voice
        
        w.startElement("music:type");
        w.addTextNode(durationToString(chord->duration()));
        w.endElement(); // music:type
        
        for (int i = 0; i < chord->dots(); i++) {
            w.startElement("music:dot");
            w.endElement(); // music:dot
        }
        
        if (part->staffCount() > 1) {
            // only write staff info when more than one staff exists
            Staff* s = chord->staff();
            w.startElement("music:staff");
            w.addTextNode(QString::number(part->indexOfStaff(s) + 1));
            w.endElement();  //music:staff
        }
        w.endElement(); // music:note
    } else for (int n = 0; n < chord->noteCount(); n++) {
        Staff* staff = chord->note(n)->staff();
        w.startElement("music:note");
        
        if (n > 0) {
            w.startElement("music:chord");
            w.endElement(); // music:chord
        }

        w.startElement("music:pitch");
        w.startElement("music:step");
        int pitch = chord->note(n)->pitch();
        char note = 'A' + ((((pitch + 2) % 7) + 7) % 7);
        w.addTextNode(QString(note));
        w.endElement(); // music:step

        if (chord->note(n)->accidentals()) {
            w.startElement("music:alter");
            w.addTextNode(QString::number(chord->note(n)->accidentals()));
            w.endElement(); // music:alter
        }
        
        w.startElement("music:octave");
        w.addTextNode(QString::number((pitch + 4*7) / 7)); // first add, than divide to get proper rounding
        w.endElement(); // music:octave
        w.endElement(); // music:pitch
        w.startElement("music:duration");
        w.addTextNode(QString::number(chord->length()));
        w.endElement(); // music:duration
        
        w.startElement("music:voice");
        w.addTextNode(QString::number(part->indexOfVoice(voice) + 1));
        w.endElement(); // music:voice
        
        w.startElement("music:type");
        w.addTextNode(durationToString(chord->duration()));
        w.endElement(); // music:type
        
        for (int i = 0; i < chord->dots(); i++) {
            w.startElement("music:dot");
            w.endElement(); // music:dot
        }
        
        int activeAccidentals = 0;
        KeySignature* ks = staff->lastKeySignatureChange(bar);
        if (ks) activeAccidentals = ks->accidentals(chord->note(n)->pitch());
        VoiceBar* vb = chord->voiceBar();
        // next check the bar for the last previous note in the same voice with the same pitch
        for (int e = 0; e < vb->elementCount(); e++) {
            Chord* c = dynamic_cast<Chord*>(vb->element(e));
            if (!c) continue;
            if (c == chord) break;
            for (int nid = 0; nid < c->noteCount(); nid++) {
                Note* note = c->note(nid);
                if (note->staff() != staff) continue;
                if (note->pitch() == chord->note(n)->pitch()) {
                    activeAccidentals = note->accidentals();
                }
            }
        }
        
        if (chord->note(n)->accidentals() != activeAccidentals) {
            w.startElement("music:accidental");
            switch (chord->note(n)->accidentals()) {
                case -2: w.addTextNode("flat-flat"); break;
                case -1: w.addTextNode("flat"); break;
                case  0: w.addTextNode("natural"); break;
                case  1: w.addTextNode("sharp"); break;
                case  2: w.addTextNode("double-sharp"); break;
            }
            w.endElement(); // music:accidental
        }
        
        if (part->staffCount() > 1) {
            // only write staff info when more than one staff exists
            Staff* s = chord->note(n)->staff();
            w.startElement("music:staff");
            w.addTextNode(QString::number(part->indexOfStaff(s) + 1));
            w.endElement();  //music:staff
        }
        w.endElement(); // music:note
    }
}

static void writeClef(KoXmlWriter& w, Clef* clef, Part* part)
{
    w.startElement("music:clef");
  
      if (part->staffCount() > 1) {
        // only write staff info when more than one staff exists
        Staff* s = clef->staff();
        w.addAttribute("number", QString::number(part->indexOfStaff(s) + 1));
    }
    
    w.startElement("music:sign");
    switch (clef->shape()) {
        case Clef::GClef: w.addTextNode("G"); break;
        case Clef::FClef: w.addTextNode("F"); break;
        case Clef::CClef: w.addTextNode("C"); break;
    }
    w.endElement(); // music:sign
    
    w.endElement(); // music:clef
}

static void writeKeySignature(KoXmlWriter& w, KeySignature* ks, Part* part)
{
    w.startElement("music:key");
    
    if (part->staffCount() > 1) {
        // only write staff info when more than one staff exists
        Staff* s = ks->staff();
        w.addAttribute("number", QString::number(part->indexOfStaff(s) + 1));
    }
    
    w.startElement("music:fifths");
    w.addTextNode(QString::number(ks->accidentals()));
    w.endElement(); // music:fifths
    
    w.endElement(); // music:key
}

static void writeTimeSignature(KoXmlWriter& w, TimeSignature* ts, Part* part)
{
    w.startElement("music:time");
    
    if (part->staffCount() > 1) {
        // only write staff info when more than one staff exists
        Staff* s = ts->staff();
        w.addAttribute("number", QString::number(part->indexOfStaff(s) + 1));
    }
    
    w.startElement("music:beats");
    w.addTextNode(QString::number(ts->beats()));
    w.endElement(); // music:beats
    
    w.startElement("music:beat-type");
    w.addTextNode(QString::number(ts->beat()));
    w.endElement(); // music:beat-type
    
    w.endElement(); // music:time
}

static void writePart(KoXmlWriter& w, int id, Part* part)
{
    w.startElement("music:part");
    w.addAttribute("id", QString("P%1").arg(id));

    for (int i = 0; i < part->sheet()->barCount(); i++) {
        Bar* bar = part->sheet()->bar(i);
        
        w.startElement("music:measure");
        w.addAttribute("number", i+1);

        bool inAttributes = false;
        if (i == 0) {
            w.startElement("music:attributes");
            w.startElement("music:divisions");
            w.addTextNode(QString::number(QuarterLength));
            w.endElement(); // music:divisions
            inAttributes = true;
        }

        for (int st = 0; st < part->staffCount(); st++) {
            Staff* staff = part->staff(st);
            for (int e = 0; e < bar->staffElementCount(staff); e++) {
                StaffElement* se = bar->staffElement(staff, e);
                
                KeySignature* ks = dynamic_cast<KeySignature*>(se);
                if (ks) {
                    if (!inAttributes) {
                        w.startElement("music:attributes");
                        inAttributes = true;
                    }
                    writeKeySignature(w, ks, part);
                }
            }
        }
        for (int st = 0; st < part->staffCount(); st++) {
            Staff* staff = part->staff(st);
            for (int e = 0; e < bar->staffElementCount(staff); e++) {
                StaffElement* se = bar->staffElement(staff, e);
                
                TimeSignature* ts = dynamic_cast<TimeSignature*>(se);
                if (ts) {
                    if (!inAttributes) {
                        w.startElement("music:attributes");
                        inAttributes = true;
                    }
                    writeTimeSignature(w, ts, part);
                }
            }
        }

        if (i == 0 && part->staffCount() != 1) {
            w.startElement("music:staves");
            w.addTextNode(QString::number(part->staffCount()));
            w.endElement(); // music:staves
        }            
            
        for (int st = 0; st < part->staffCount(); st++) {
            Staff* staff = part->staff(st);
            for (int e = 0; e < bar->staffElementCount(staff); e++) {
                StaffElement* se = bar->staffElement(staff, e);
                
                Clef* c = dynamic_cast<Clef*>(se);
                if (c) {
                    if (!inAttributes) {
                        w.startElement("music:attributes");
                        inAttributes = true;
                    }
                    writeClef(w, c, part);
                }
            }
        }

        if (inAttributes) {
            w.endElement(); // music:attributes
        }

        int curTime = 0;
        for (int voice = 0; voice < part->voiceCount(); voice++) {
            if (curTime != 0) {
                w.startElement("music:backup");
                w.startElement("music:duration");
                w.addTextNode(QString::number(curTime));
                w.endElement(); // music:duration
                w.endElement(); // music:backup
            }

            Voice* v = part->voice(voice);
            VoiceBar* vb = part->sheet()->bar(i)->voice(v);
            for (int e = 0; e < vb->elementCount(); e++) {
                VoiceElement* ve = vb->element(e);
                curTime += ve->length();

                Chord* c =  dynamic_cast<Chord*>(ve);
                if(c) writeChord(w, c, v, part, i);
            }
        }
        w.endElement(); // music:measure
    }

    w.endElement(); // music:part
}

void MusicXmlWriter::writeSheet(KoXmlWriter& w, Sheet* sheet, bool writeNamespaceDef)
{
//    w.startDocument("score-partwise", "-//Recordare//DTD MusicXML 1.1 Partwise//EN",
//        "http://www.musicxml.org/dtds/partwise.dtd");
    w.startElement("music:score-partwise");
    if (writeNamespaceDef) {
        w.addAttribute("xmlns:music", "http://www.calligra.org/music");
    }
    w.addAttribute("version", "1.1");

    w.startElement("music:part-list");
    for (int i = 0; i < sheet->partCount(); i++) {
        for (int pg = 0; pg < sheet->partGroupCount(); pg++) {
            if (sheet->partGroup(pg)->firstPart() == i) {
                writePartGroup(w, pg+1, sheet->partGroup(pg));
            }
        }
        writePartDesc(w, i, sheet->part(i));
        for (int pg = 0; pg < sheet->partGroupCount(); pg++) {
            if (sheet->partGroup(pg)->lastPart() == i) {
                w.startElement("music:part-group");
                w.addAttribute("type", "stop");
                w.addAttribute("number", pg+1);
                w.endElement(); // music:part-group
            }
        }
    }
    w.endElement(); // music:part-list

    for (int i = 0; i < sheet->partCount(); i++) {
        writePart(w, i, sheet->part(i));
    }

    w.endElement(); // music:score-partwise
//    w.endDocument();
}

