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
#include "VoiceBar.h"
#include "VoiceElement.h"
#include "Bar.h"
#include "Chord.h"
#include "Note.h"
#include "KeySignature.h"
#include "Staff.h"

#include <QtCore/QList>

namespace MusicCore {

class VoiceBar::Private
{
public:
    QList<VoiceElement*> elements;
};

VoiceBar::VoiceBar(Bar* bar) : QObject(bar), d(new Private)
{
    Q_ASSERT( bar );
}

VoiceBar::~VoiceBar()
{
    Q_FOREACH(VoiceElement* me, d->elements) delete me;
    delete d;
}

Bar* VoiceBar::bar()
{
    return qobject_cast<Bar*>(parent());
}

int VoiceBar::elementCount() const
{
    return d->elements.size();
}

VoiceElement* VoiceBar::element(int index)
{
    Q_ASSERT( index >= 0 && index < elementCount() );
    return d->elements[index];
}

int VoiceBar::indexOfElement(VoiceElement* element)
{
    Q_ASSERT( element );
    return d->elements.indexOf(element);
}

void VoiceBar::addElement(VoiceElement* element)
{
    Q_ASSERT( element );
    d->elements.append(element);
    element->setVoiceBar(this);
    updateAccidentals();
}

void VoiceBar::insertElement(VoiceElement* element, int before)
{
    Q_ASSERT( element );
    Q_ASSERT( before >= 0 && before <= elementCount() );
    d->elements.insert(before, element);
    element->setVoiceBar(this);
    updateAccidentals();
}

void VoiceBar::insertElement(VoiceElement* element, VoiceElement* before)
{
    Q_ASSERT( element );
    Q_ASSERT( before );
    int index = d->elements.indexOf(before);
    Q_ASSERT( index != -1 );
    insertElement(element, index);
}

void VoiceBar::removeElement(int index, bool deleteElement)
{
    Q_ASSERT( index >= 0 && index < elementCount() );
    VoiceElement* e = d->elements.takeAt(index);
    if (deleteElement) {
        delete e;
    }
    updateAccidentals();
}

void VoiceBar::removeElement(VoiceElement* element, bool deleteElement)
{
    Q_ASSERT( element );
    int index = d->elements.indexOf(element);
    Q_ASSERT( index != -1 );
    removeElement(index, deleteElement);
}

void VoiceBar::updateAccidentals()
{
    // this isn't the best way I think, but just implement a O(N^2) algorithm for now :)

    for (int e = 0; e < elementCount(); e++) {
        Chord* c = dynamic_cast<Chord*>(element(e));
        if (!c) continue;
        for (int nid = 0; nid < c->noteCount(); nid++) {
            Note* note = c->note(nid);
            Staff* s = note->staff();
            
            KeySignature* ks = s->lastKeySignatureChange(bar());
            int curAccidentals = 0;
            if (ks) curAccidentals = ks->accidentals(note->pitch());
            // next check the bar for the last previous note in the same voice with the same pitch
            for (int eid = 0; eid < e; eid++) {
                Chord* c2 = dynamic_cast<Chord*>(element(eid));
                if (!c2) continue;
                for (int nid2 = 0; nid2 < c2->noteCount(); nid2++) {
                    Note* n = c2->note(nid2);
                    if (n->staff() != s) continue;
                    if (note->pitch() == n->pitch()) {
                        curAccidentals = n->accidentals();
                    }
                }
            }
            note->setDrawAccidentals(curAccidentals != note->accidentals());
        }
    }
}

} // namespace MusicCore

#include <VoiceBar.moc>
