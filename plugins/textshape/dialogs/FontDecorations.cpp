/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006-2007 Thomas Zander <zander@kde.org>
   Copyright (C)  2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "FontDecorations.h"

FontDecorations::FontDecorations(bool uniqueFormat, QWidget *parent)
    : QWidget(parent)
    ,  m_uniqueFormat(uniqueFormat)
{
    widget.setupUi(this);

    connect(widget.hyphenate, SIGNAL(stateChanged(int)), this, SLOT(hyphenateStateChanged()));

    widget.shadowGroupBox->setVisible(false);
    widget.positionGroupBox->setVisible(false);
}

void FontDecorations::hyphenateStateChanged()
{
    m_hyphenateInherited = false;
}

void FontDecorations::setDisplay(KoCharacterStyle *style)
{
    if (!style) {
        return;
    }

    m_hyphenateInherited = !style->hasProperty(KoCharacterStyle::HasHyphenation);
    if (!m_uniqueFormat) {
        widget.hyphenate->setTristate(true);
        widget.hyphenate->setCheckState(Qt::PartiallyChecked);
    } else {
        widget.hyphenate->setChecked(style->hasHyphenation());
    }
}

void FontDecorations::save(KoCharacterStyle *style) const
{
    if (!style) {
        return;
    }

    if (!m_hyphenateInherited) {
        if (widget.hyphenate->checkState() == Qt::Checked) {
            style->setHasHyphenation(true);
        } else if (widget.hyphenate->checkState() == Qt::Unchecked) {
            style->setHasHyphenation(false);
        }
    }

}
