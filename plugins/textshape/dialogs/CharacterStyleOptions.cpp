/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

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

#include "CharacterStyleOptions.h"
#include <KoCharacterStyle.h>
#include <QButtonGroup>

enum Position {
    Normal,
    Superscript,
    Subscript,
    Custom
};

CharacterStyleOptions::CharacterStyleOptions( bool withSubSuperScript, QWidget* parent)
        : QWidget( parent), m_style(0)
{
    widget.setupUi(this);

    if ( !withSubSuperScript ) widget.positionGroup->setVisible(false);

    // sigh, we could do this in designer in Qt3 :(
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(widget.normal, Normal);
    m_buttonGroup->addButton(widget.subscript, Subscript);
    m_buttonGroup->addButton(widget.superscript, Superscript);
    m_buttonGroup->addButton(widget.custom, Custom);

    widget.custom->setVisible(false);
    widget.offset->setVisible(false);
    widget.offsetLabel->setVisible(false);
}

void CharacterStyleOptions::open(KoCharacterStyle *style) {
    m_style = style;
    if(m_style == 0)
        return;
    switch(style->verticalAlignment()) {
        case QTextCharFormat::AlignSuperScript:
            m_buttonGroup->button(Superscript)->setChecked(true);
            break;
        case QTextCharFormat::AlignSubScript:
            m_buttonGroup->button(Subscript)->setChecked(true);
            break;
        default:
            // TODO check if its custom instead.
            m_buttonGroup->button(Normal)->setChecked(true);
    }

    widget.hyphenate->setChecked(style->hasHyphenation());
}

void CharacterStyleOptions::save() {
    if(m_style == 0)
        return;
    QTextCharFormat::VerticalAlignment va;

    switch(m_buttonGroup->checkedId()) {
        case Subscript:
            va = QTextCharFormat::AlignSubScript;
            break;
        case Superscript:
            va = QTextCharFormat::AlignSuperScript;
            break;
        case Custom:
            // fallthrough..
        default:
            va = QTextCharFormat::AlignNormal;
            // TODO also handle custom
    }
    m_style->setVerticalAlignment(va);
    m_style->setHasHyphenation(widget.hyphenate->isChecked());
}

#include "CharacterStyleOptions.moc"
