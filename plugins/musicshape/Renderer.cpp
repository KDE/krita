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
#include "Renderer.h"
#include "MusicStyle.h"

#include "core/Sheet.h"
#include "core/Part.h"
#include "core/Voice.h"
#include "core/Staff.h"
#include "core/VoiceBar.h"
#include "core/Chord.h"
#include "core/Note.h"
#include "core/Clef.h"
#include "core/Bar.h"
#include "core/KeySignature.h"
#include "core/TimeSignature.h"
#include "core/StaffSystem.h"

#include <QMultiMap>

#include <climits>

using namespace MusicCore;

MusicRenderer::MusicRenderer(MusicStyle* style) : m_style(style), m_debug(false)
{
}

void MusicRenderer::renderSheet(QPainter& painter, Sheet* sheet, int firstSystem, int lastSystem)
{
    int firstBar = sheet->staffSystem(firstSystem)->firstBar();
    int lastBar = INT_MAX;
    if (lastSystem < sheet->staffSystemCount()-1) {
        lastBar = sheet->staffSystem(lastSystem+1)->firstBar()-1;
    }

    for (int i = 0; i < sheet->partCount(); i++) {
        renderPart(painter, sheet->part(i), firstBar, lastBar);
    }
    for (int i = firstSystem; i <= lastSystem && i < sheet->staffSystemCount(); i++) {
        StaffSystem* ss = sheet->staffSystem(i);
        if (ss->indent() == 0) continue;
        int b = ss->firstBar();
        Bar* bar = sheet->bar(b);
        qreal by = bar->position().y();
        qreal ind = ss->indent();

        for (int p = 0; p < sheet->partCount(); p++) {
            Part* part = sheet->part(p);
            for (int s = 0; s < part->staffCount(); s++) {
                Staff* staff = part->staff(s);
                qreal y = staff->top();
                qreal dy = staff->lineSpacing();

                painter.setPen(m_style->staffLinePen());
                for (int l = 0; l < staff->lineCount(); l++) {
                    painter.drawLine(QPointF(0, by + y + l * dy), QPointF(ind, by + y + l * dy));
                }

                Clef* clef = ss->clef(staff);
                RenderState foo;
                qreal x = 15;
                if (clef) {
                    renderClef(painter, clef, QPointF(x, by), foo, Qt::black, true);
                    x += clef->width() + 15;
                }
                KeySignature* ks = staff->lastKeySignatureChange(b);
                if (ks) {
                    renderKeySignature(painter, ks, QPointF(x, by), foo, Qt::black, true);
                }
            }
        }
    }
}

void MusicRenderer::renderPart(QPainter& painter, Part* part, int firstBar, int lastBar, const QColor& color)
{
    if (lastBar < firstBar) return;

    for (int i = 0; i < part->staffCount(); i++) {
        renderStaff(painter, part->staff(i), firstBar, lastBar, color);
    }
    qreal firstStaff = part->staff(0)->top();
    int c = part->staffCount()-1;
    qreal lastStaff = part->staff(c)->bottom();
    for (int b = firstBar; b <= lastBar && b < part->sheet()->barCount(); b++) {
        Bar* bar = part->sheet()->bar(b);
        QPointF p = bar->position();
        painter.drawLine(QPointF(p.x() + bar->size(), p.y() + firstStaff), QPointF(p.x() + bar->size(), p.y() + lastStaff));
        if (m_debug) {
            painter.setPen(QPen(Qt::green));
            painter.drawLine(QPointF(p.x(), p.y() + firstStaff - 3), QPointF(p.x(), p.y() + lastStaff + 3));
            painter.drawLine(QPointF(p.x() - bar->prefix(), p.y() + firstStaff - 3), QPointF(p.x() - bar->prefix(), p.y() + lastStaff + 3));
        }

        // check if the bar contains any elements, if not render a rest
        bool hasContents = false;
        for (int v = 0; v < part->voiceCount(); v++) {
            if (part->voice(v)->bar(bar)->elementCount() > 0) {
                hasContents = true;
                break;
            }
        }

        if (!hasContents) {
            QPointF pos = bar->position();
            qreal w = bar->size();
            for (int sid = 0; sid < part->staffCount(); sid++) {
                Staff* s = part->staff(sid);
                renderRest(painter, WholeNote, pos + QPointF(w/2, s->top() + s->lineSpacing()), color);
            }
        }
    }
    for (int i = 0; i < part->voiceCount(); i++) {
        renderVoice(painter, part->voice(i), firstBar, lastBar, color);
    }
}

void MusicRenderer::renderStaff(QPainter& painter, Staff *staff, int firstBar, int lastBar, const QColor& color)
{
    qreal dy = staff->lineSpacing();
    qreal y = staff->top();
    for (int b = firstBar; b <= lastBar && b < staff->part()->sheet()->barCount(); b++) {
        Bar* bar = staff->part()->sheet()->bar(b);
        QPointF p = bar->position();
        QPointF prep = bar->prefixPosition() + QPointF(bar->prefix(), 0);
        painter.setPen(m_style->staffLinePen(color));
        for (int i = 0; i < staff->lineCount(); i++) {
            painter.drawLine(QPointF(p.x(), p.y() + y + i * dy), QPointF(p.x() + bar->size(), p.y() + y + i * dy));
        }
        if (bar->prefix() > 0) {
            QPointF q = bar->prefixPosition();
            for (int i = 0; i < staff->lineCount(); i++) {
                painter.drawLine(QPointF(q.x(), q.y() + y + i * dy), QPointF(q.x() + bar->prefix(), q.y() + y + i * dy));
            }
        }
        RenderState state;
        for (int e = 0; e < bar->staffElementCount(staff); e++) {
            StaffElement* se = bar->staffElement(staff, e);
            if (se->startTime() == 0) {
                renderStaffElement(painter, bar->staffElement(staff, e), prep, state, color);
            } else {
                renderStaffElement(painter, bar->staffElement(staff, e), p, state, color);
            }
        }
    }
}

void MusicRenderer::renderVoice(QPainter& painter, Voice *voice, int firstBar, int lastBar, const QColor& color)
{
    RenderState state;
    state.clef = 0;
    for (int b = firstBar; b <= lastBar && b < voice->part()->sheet()->barCount(); b++) {
        Bar* bar = voice->part()->sheet()->bar(b);
        QPointF p = bar->position();
        VoiceBar* vb = voice->bar(bar);
        for (int e = 0; e < vb->elementCount(); e++) {
            if (vb->element(e)->staff()) {
                state.clef = vb->element(e)->staff()->lastClefChange(b, 0);
            }
            renderElement(painter, vb->element(e), voice, p, state, color);
        }
    }
}

void MusicRenderer::renderElement(QPainter& painter, VoiceElement* me, Voice* voice, const QPointF& pos, RenderState& state, const QColor& color)
{
    Q_UNUSED( state ); // unused for now, but will probably be used again in the future

    qreal top = 0;
    if (me->staff()) top += me->staff()->top();
    if (m_debug) {
        painter.setPen(QPen(Qt::blue));
        painter.drawLine(pos + QPointF(me->x(), top + me->y() - 4), pos + QPointF(me->x(), top + me->y() + me->height() + 4));
        painter.drawLine(pos + QPointF(me->x() + me->width(), top + me->y() - 4), pos + QPointF(me->x() + me->width(), top + me->y() + me->height() + 4));

        painter.drawLine(pos + QPointF(me->x() - 4, top + me->y()), pos + QPointF(me->x() + me->width() + 4, top + me->y()));
        painter.drawLine(pos + QPointF(me->x() - 4, top + me->y() + me->height()), pos + QPointF(me->x() + me->width() + 4, top + me->y() + me->height()));

        painter.setPen(QPen(Qt::red));
        painter.drawLine(pos + QPointF(me->x() + me->beatline(), top + me->y() - 10), pos + QPointF(me->x() + me->beatline(), top + me->y() + me->height() + 10));
    }

    // TODO: make this less hacky
    Chord *c = dynamic_cast<Chord*>(me);
    if (c) renderChord(painter, c, voice, pos, color);
}

void MusicRenderer::renderStaffElement(QPainter& painter, MusicCore::StaffElement* se, const QPointF& pos, RenderState& state, const QColor& color)
{
    qreal top = 0;
    top += se->staff()->top();
    if (m_debug) {
        painter.setPen(QPen(Qt::blue));
        painter.drawLine(pos + QPointF(se->x(), top + se->y() - 20), pos + QPointF(se->x(), top + se->y() + 20));
        painter.drawLine(pos + QPointF(se->x() + se->width(), top + se->y() - 20), pos + QPointF(se->x() + se->width(), top + se->y() + 20));

        painter.drawLine(pos + QPointF(se->x() - 10, top + se->y()), pos + QPointF(se->x() + se->width() + 10, top + se->y()));
        painter.drawLine(pos + QPointF(se->x() - 10, top + se->y() + se->height()), pos + QPointF(se->x() + se->width() + 10, top + se->y() + se->height()));
    }

    Clef *cl = dynamic_cast<Clef*>(se);
    if (cl) renderClef(painter, cl, pos, state, color);
    KeySignature *ks = dynamic_cast<KeySignature*>(se);
    if (ks) renderKeySignature(painter, ks, pos, state, color);
    TimeSignature* ts = dynamic_cast<TimeSignature*>(se);
    if (ts) renderTimeSignature(painter, ts, pos, color);
}


void MusicRenderer::renderClef(QPainter& painter, Clef *c, const QPointF& pos, RenderState& state, const QColor& color, bool ignoreOwnPos)
{
    Q_UNUSED(color);
    state.clef = c;
    Staff* s = c->staff();
    m_style->renderClef(painter, pos.x() + (ignoreOwnPos ? 0 : c->x()), pos.y() + s->top() + (s->lineCount() - c->line()) * s->lineSpacing(), c->shape());
}

void MusicRenderer::renderKeySignature(QPainter& painter, KeySignature* ks, const QPointF& pos, RenderState& state, const QColor& color, bool ignoreOwnPos)
{
    Q_UNUSED(color);
    Staff * s = ks->staff();
    qreal curx = pos.x() + (ignoreOwnPos ? 0 : ks->x());
    // draw naturals for sharps
    int idx = 3;
    for (int i = 0; i < 7; i++) {
        if (ks->cancel(idx) > 0) {
            int line = 10;
            if (state.clef) line = state.clef->pitchToLine(idx);

            while (line < 0) line += 7;
            while (line >= 6) line -= 7;
            m_style->renderAccidental( painter, curx, pos.y() + s->top() + line * s->lineSpacing() / 2, 0 );

            curx += 6;
        }
        idx = (idx + 4) % 7;
    }

    // draw naturals for flats
    idx = 6;
    for (int i = 0; i < 7; i++) {
        if (ks->cancel(idx) < 0) {
            int line = 10;
            if (state.clef) line = state.clef->pitchToLine(idx);

            while (line < 0) line += 7;
            while (line >= 6) line -= 7;

            m_style->renderAccidental( painter, curx, pos.y() + s->top() + line * s->lineSpacing() / 2, 0 );

            curx += 6;
        }
        idx = (idx + 3) % 7;
    }

    // draw sharps
    idx = 3;
    for (int i = 0; i < 7; i++) {
        if (ks->accidentals(idx) > 0) {
            int line = 10;
            if (state.clef) line = state.clef->pitchToLine(idx);

            while (line < 0) line += 7;
            while (line >= 6) line -= 7;
            m_style->renderAccidental( painter, curx, pos.y() + s->top() + line * s->lineSpacing() / 2, 1 );

            curx += 6;
        }
        idx = (idx + 4) % 7;
    }

    // draw flats
    idx = 6;
    for (int i = 0; i < 7; i++) {
        if (ks->accidentals(idx) < 0) {
            int line = 10;
            if (state.clef) line = state.clef->pitchToLine(idx);

            while (line < 0) line += 7;
            while (line >= 6) line -= 7;

            m_style->renderAccidental( painter, curx, pos.y() + s->top() + line * s->lineSpacing() / 2, -1 );

            curx += 6;
        }
        idx = (idx + 3) % 7;
    }
}

void MusicRenderer::renderTimeSignature(QPainter& painter, TimeSignature* ts, const QPointF& pos, const QColor& color)
{
    Q_UNUSED(color);
    Staff* s = ts->staff();
    qreal hh = 0.5 * (s->lineCount() - 1) * s->lineSpacing();
    m_style->renderTimeSignatureNumber( painter, pos.x() + ts->x(), pos.y() + s->top() + hh, ts->width(), ts->beats());
    m_style->renderTimeSignatureNumber( painter, pos.x() + ts->x(), pos.y() + s->top() + 2*hh, ts->width(), ts->beat());
}

void MusicRenderer::renderRest(QPainter& painter, Duration duration, const QPointF& pos, const QColor& color)
{
    m_style->renderRest(painter, pos.x(), pos.y(), duration, color);
}

void MusicRenderer::renderChord(QPainter& painter, Chord* chord, Voice* voice, const QPointF& ref, const QColor& color)
{
    qreal x = chord->x();
    if (chord->noteCount() == 0) { // a rest
        Staff *s = chord->staff();
        renderRest(painter, chord->duration(), ref + QPointF(x, s->top() + (2 - (chord->duration() == WholeNote)) * s->lineSpacing()), color);
        return;
    }
    int topLine = 0, bottomLine = 0;
    VoiceBar* vb = chord->voiceBar();
    Bar* bar = vb->bar();
    Sheet* sheet = voice->part()->sheet();
    int barIdx = bar->sheet()->indexOfBar(bar);
    qreal topy = 1e9, bottomy = -1e9;
    Staff* topStaff = 0, *bottomStaff = 0;

    qreal mainNoteX = (chord->stemDirection() == StemUp ? chord->stemX() - 6 : chord->stemX());
    qreal alternateNoteX = mainNoteX + (chord->stemDirection() == StemUp ? 6 : -6);
    bool prevAlternate = false;
    qreal maxNoteX = 0;

    QMultiMap<Staff*, int> dots;

    Chord* nextChord = 0;

    for (int i = 0; i < chord->noteCount(); i++) {
        Note *n = chord->note(i);
        Staff * s = n->staff();
        Clef* clef = s->lastClefChange(barIdx);
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());

        qreal noteX = mainNoteX;
        if (i > 0) {
            int prevPitch = chord->note(i-1)->pitch();
            if (abs(prevPitch - n->pitch()) <= 1 && !prevAlternate) {
                noteX = alternateNoteX;
            }
        }
        if (i < chord->noteCount()-1 && chord->stemDirection() == StemDown) {
            int pitch = n->pitch();
            int nPitch = chord->note(i+1)->pitch();
            if (abs(pitch - nPitch) <= 1 && !prevAlternate) {
                noteX = alternateNoteX;
            }
        }
        prevAlternate = noteX != mainNoteX;
        if (noteX > maxNoteX) maxNoteX = noteX;

        if (line > 9) { // lines under the bar
            painter.setPen(m_style->staffLinePen(color));
            for (int i = 10; i <= line; i+= 2) {
                qreal y = s->top() + i * s->lineSpacing() / 2;
                painter.drawLine(ref + QPointF(noteX - 4, y), ref + QPointF(noteX + 10, y));
            }
        } else if (line < -1) { // lines above the bar
            painter.setPen(m_style->staffLinePen(color));
            for (int i = -2; i >= line; i-= 2) {
                qreal y = s->top() + i * s->lineSpacing() / 2;
                painter.drawLine(ref + QPointF(noteX - 4, y), ref + QPointF(noteX + 10, y));
            }
        }

        qreal ypos = s->top() + line * s->lineSpacing() / 2;
        if (ypos < topy) {
            topy = ypos;
            topLine = line;
            topStaff = s;
        }
        if (ypos > bottomy) {
            bottomy = ypos;
            bottomLine = line;
            bottomStaff = s;
        }

        m_style->renderNoteHead( painter, ref.x() + noteX, ref.y() + s->top() + line * s->lineSpacing() / 2, chord->duration(), color );

        // render accidentals
        if (n->drawAccidentals()) {
            m_style->renderAccidental( painter, ref.x() + x, ref.y() + /*chord->y() +*/ s->top() + line * s->lineSpacing() / 2, n->accidentals(), color );
        }

        dots.insert(s, line);

        if (n->isStartTie()) {
            // render tie for this note...
            if (!nextChord) {
                // figure out what the next chord in this voice is
                bool afterCurrent = false;
                for (int e = 0; e < vb->elementCount(); e++) {
                    if (afterCurrent) {
                        nextChord = dynamic_cast<Chord*>(vb->element(e));
                        if (nextChord) break;
                    } else {
                        if (vb->element(e) == chord) {
                            afterCurrent = true;
                        }
                    }
                }
                if (!nextChord) {
                    // check the next bar
                    int nextBar = sheet->indexOfBar(bar)+1;
                    if (nextBar < sheet->barCount()) {
                        VoiceBar* nextVB = voice->bar(nextBar);
                        for (int e = 0; e < nextVB->elementCount(); e++) {
                            nextChord = dynamic_cast<Chord*>(nextVB->element(e));
                            if (nextChord) break;
                        }
                    }
                }
            }

            // okay, now nextChord is the chord to which the tie should go
            if (nextChord) {
                QPointF startPos = bar->position() + QPointF(1 + chord->x() + chord->width(), ypos);
                QPointF endPos = nextChord->voiceBar()->bar()->position() + QPointF(nextChord->x() - 1, ypos);
                if (bar->position().y() < nextChord->voiceBar()->bar()->position().y() - 1e-6) {
                    endPos = bar->position() + QPointF(bar->size(), 0);
                }

                endPos.setY(startPos.y());
                QPointF c1a = startPos + QPointF(2, 4);
                QPointF c2a = endPos + QPointF(-2, 4);
                QPointF c1b = startPos + QPointF(2, 5);
                QPointF c2b = endPos + QPointF(-2, 5);

                QPainterPath p;
                p.moveTo(startPos);
                p.cubicTo(c1a, c2a, endPos);
                p.cubicTo(c2b, c1b, startPos);
                painter.setPen(Qt::NoPen);//m_style->slurPen(color));
                painter.setBrush(QBrush(color));
                painter.drawPath(p);
            }
        }
    }

    // calculate correct positioning of dots
    // render dots of notes
    painter.setPen(m_style->noteDotPen(color));
    foreach (Staff* s, dots.keys()) {
        QList<int> lines = dots.values(s);
        qSort(lines);

        int lastLine = INT_MIN;
        bool moveGroupDown = true;
        for (int i = 0; i < lines.size(); i++) {
            int line = lines[i];
            if (line % 2 == 0) {
                line--;
            }
            if (line == lastLine) {
                if (moveGroupDown) {
                    lines[i-1] += 2;
                    for (int j = i-2; j >= 0; j--) {
                        if (lines[j] == lines[j+1]) {
                            lines[j] += 2;
                        } else {
                            break;
                        }
                    }
                } else {
                    line -= 2;
                }
                moveGroupDown = !moveGroupDown;
            }
            lines[i] = line;
            lastLine = line;
        }

        foreach (int line, lines) {
            qreal dotX = maxNoteX + 11;
            for (int i = 0; i < chord->dots(); i++) {
                painter.drawPoint(ref + QPointF(dotX, s->top() + line * s->lineSpacing() / 2));
                dotX += 3;
            }
        }
    }

    qreal stemLen = chord->stemLength() * 2;
    if (stemLen != 0.0 && stemLen != -0.0) {
        qreal stemX = chord->stemX();
        bool stemsUp = chord->stemDirection() == StemUp;

        painter.setPen(m_style->stemPen(color));
        if (stemsUp) {
            painter.drawLine(ref + QPointF(stemX, chord->stemEndY()),
                             ref + QPointF(stemX, bottomStaff->top() + bottomLine * bottomStaff->lineSpacing() / 2));
            if (chord->beamType(0) == BeamFlag) {
                m_style->renderNoteFlags( painter, ref.x() + stemX, ref.y() + chord->stemEndY(), chord->duration(), stemsUp, color );
            }
        } else {
            painter.drawLine(ref + QPointF(stemX, topStaff->top() + topLine * topStaff->lineSpacing() / 2),
                             ref + QPointF(stemX, chord->stemEndY()));
            if (chord->beamType(0) == BeamFlag) {
                m_style->renderNoteFlags( painter, ref.x() + stemX, ref.y() + chord->stemEndY(), chord->duration(), stemsUp, color );
            }
        }

        painter.setPen(QPen(Qt::NoPen));
        painter.setBrush(QBrush(color));
        for (int i = 0; i < chord->beamCount(); i++) {
            if (chord->beamType(i) == BeamStart) {
                const Chord* endChord = chord->beamEnd(i);

                QPointF beamStart(chord->stemX(), chord->stemEndY());
                QPointF beamEnd(endChord->stemX(), endChord->stemEndY());
                if (stemsUp) {
                    beamStart += QPointF(0, topStaff->lineSpacing() * i);
                    beamEnd += QPointF(0, topStaff->lineSpacing() * i);
                } else {
                    beamStart -= QPointF(0, bottomStaff->lineSpacing() * i);
                    beamEnd -= QPointF(0, bottomStaff->lineSpacing() * i);
                }


                QPointF dir(0, (stemsUp ? 1 : -1) * m_style->beamLineWidth());
                QPointF p[4];
                p[0] = ref + beamStart;
                p[1] = ref + beamEnd;
                p[2] = p[1] + dir;
                p[3] = p[0] + dir;
                painter.drawConvexPolygon(p, 4);
            } else if (chord->beamType(i) == BeamForwardHook || chord->beamType(i) == BeamBackwardHook) {
                QPointF beamStart(chord->stemX(), chord->stemEndY());
                qreal dir = 6;
                if (chord->beamType(i) == BeamBackwardHook) dir = -dir;
                if (stemsUp) {
                    beamStart += QPointF(0, topStaff->lineSpacing() * i);
                } else {
                    beamStart -= QPointF(0, bottomStaff->lineSpacing() * i);
                }

                QPointF beamEnd = beamStart + QPointF(dir, dir * chord->beamDirection());

                QPointF bdir(0, (stemsUp ? 1 : -1) * m_style->beamLineWidth());
                QPointF p[4];
                p[0] = ref + beamStart;
                p[1] = ref + beamEnd;
                p[2] = p[1] + bdir;
                p[3] = p[0] + bdir;
                painter.drawConvexPolygon(p, 4);
            }
        }

    }
}

void MusicRenderer::renderNote(QPainter& painter, Duration duration, const QPointF& pos, qreal stemLength, const QColor& color)
{
    m_style->renderNoteHead(painter, pos.x(), pos.y(), duration, color);

    if (duration <= HalfNote) {
        painter.setPen(m_style->stemPen(color));
        painter.drawLine(pos + QPointF(6, -stemLength), pos + QPointF(6, 0));
    }
    if (duration <= EighthNote) {
        m_style->renderNoteFlags(painter, pos.x()+6, pos.y() - stemLength, duration, true, color);
    }
}

void MusicRenderer::renderAccidental(QPainter& painter, int accidentals, const QPointF& pos, const QColor& color)
{
    m_style->renderAccidental( painter, pos.x(), pos.y(), accidentals, color );
}
