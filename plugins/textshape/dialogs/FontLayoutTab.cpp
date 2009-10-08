/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#include "FontLayoutTab.h"
#include "styles/KoCharacterStyle.h"

#include <QButtonGroup>

enum Position {
    Normal,
    Superscript,
    Subscript,
    Custom
};

FontLayoutTab::FontLayoutTab(bool withSubSuperScript, bool uniqueFormat, QWidget* parent)
        : QWidget(parent),
        m_uniqueFormat(uniqueFormat)
{
    widget.setupUi(this);

    if (!withSubSuperScript) widget.positionGroup->setVisible(false);

    widget.custom->setVisible(false);
    widget.offset->setVisible(false);
    widget.offsetLabel->setVisible(false);

    widget.hyphenate->setVisible(false); // TODO enable when we add this feature to the layout engine
}

void FontLayoutTab::setDisplay(KoCharacterStyle *style)
{
    if (!style)
        return;

    switch (style->verticalAlignment()) {
    case QTextCharFormat::AlignSuperScript:
        widget.superscript->setChecked(true);
        break;
    case QTextCharFormat::AlignSubScript:
        widget.subscript->setChecked(true);
        break;
    default:
        // TODO check if its custom instead.
        widget.normal->setChecked(true);
    }

    widget.positionGroup->setCheckable(!m_uniqueFormat);
    widget.positionGroup->setChecked(m_uniqueFormat);

    if (!m_uniqueFormat) {
        widget.hyphenate->setTristate(true);
        widget.hyphenate->setCheckState(Qt::PartiallyChecked);
    }
    else
        widget.hyphenate->setChecked(style->hasHyphenation());
}

void FontLayoutTab::save(KoCharacterStyle *style)
{
    Q_ASSERT(style);
    QTextCharFormat::VerticalAlignment va;

    if (m_uniqueFormat || widget.positionGroup->isChecked()) {
        if (widget.normal->isChecked())
            va = QTextCharFormat::AlignNormal;
        else if (widget.subscript->isChecked())
            va = QTextCharFormat::AlignSubScript;
        else if (widget.superscript->isChecked())
            va = QTextCharFormat::AlignSuperScript;
        else
            va = QTextCharFormat::AlignNormal;
        style->setVerticalAlignment(va);
    }

    if (widget.hyphenate->checkState() == Qt::Checked)
        style->setHasHyphenation(true);
    else if (widget.hyphenate->checkState() == Qt::Unchecked)
        style->setHasHyphenation(false);
}

#include "FontLayoutTab.moc"
