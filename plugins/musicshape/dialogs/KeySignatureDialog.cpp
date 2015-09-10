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
#include "KeySignatureDialog.h"

#include "../core/KeySignature.h"

using namespace MusicCore;

KeySignatureDialog::KeySignatureDialog(QWidget* parent)
    : KoDialog(parent)
{
    setCaption(i18n("Set key signature"));
    QWidget* w = new QWidget(this);
    widget.setupUi(w);
    setMainWidget(w);
    m_ks = new KeySignature(widget.preview->staff(), 0, 0);
    widget.preview->setStaffElement(m_ks);
    connect(widget.accidentals, SIGNAL(valueChanged(int)), this, SLOT(accidentalsChanged(int)));
}

void KeySignatureDialog::setBar(int bar)
{
    widget.startBar1->setValue(bar+1);
    widget.startBar2->setValue(bar+1);
    widget.startBar3->setValue(bar+1);
    widget.endBar->setValue(bar+1);
}

int KeySignatureDialog::accidentals()
{
    return -widget.accidentals->value();
}

void KeySignatureDialog::setAccidentals(int accidentals)
{
    widget.accidentals->setValue(accidentals);
    accidentalsChanged(-accidentals);
}

void KeySignatureDialog::accidentalsChanged(int accidentals)
{
    m_ks->setAccidentals(-accidentals);
    widget.preview->update();
}

void KeySignatureDialog::setMusicStyle(MusicStyle* style)
{
    widget.preview->setMusicStyle(style);
}

bool KeySignatureDialog::updateAllStaves()
{
    return widget.allStaves->isChecked();
}

bool KeySignatureDialog::updateToNextChange()
{
    return widget.toNextChange->isChecked();
}

bool KeySignatureDialog::updateTillEndOfPiece()
{
    return widget.throughEndOfPiece->isChecked();
}

int KeySignatureDialog::startBar()
{
    if (updateToNextChange()) {
        return widget.startBar3->value() - 1;
    } else if (updateTillEndOfPiece()) {
        return widget.startBar2->value() - 1;
    } else {
        return widget.startBar1->value() - 1;
    }
}

int KeySignatureDialog::endBar()
{
    return widget.endBar->value() - 1;
}

