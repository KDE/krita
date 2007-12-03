/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006-2007 Thomas Zander <zander@kde.org>

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

#include "CharacterHighlighting.h"

#include <KoText.h>
#include <KoCharacterStyle.h>

CharacterHighlighting::CharacterHighlighting( QWidget* parent)
    : QWidget ( parent)
{
    widget.setupUi(this);

    widget.underlineStyle->addItems( KoText::underlineTypeList() );
    widget.underlineLineStyle->addItems( KoText::underlineStyleList() );

    connect( widget.underlineStyle, SIGNAL( activated( int ) ), this, SLOT( underlineChanged( int ) ) );
}

void CharacterHighlighting::underlineChanged(int item) {
    widget.underlineLineStyle->setEnabled(item != 0);
    widget.underlineColor->setEnabled(item != 0);
}

void CharacterHighlighting::open(KoCharacterStyle *style) {
    m_style = style;
    if(m_style == 0)
        return;

    widget.underlineStyle->setCurrentIndex(1);
    switch(style->underlineStyle()) {
        case KoCharacterStyle::DashLine:
            widget.underlineLineStyle->setCurrentIndex(1);
            break;
        case KoCharacterStyle::DottedLine:
            widget.underlineLineStyle->setCurrentIndex(2);
            break;
        case KoCharacterStyle::DotDashLine:
            widget.underlineLineStyle->setCurrentIndex(3);
            break;
        case KoCharacterStyle::DotDotDashLine:
            widget.underlineLineStyle->setCurrentIndex(4);
            break;
        case KoCharacterStyle::WaveLine:
            widget.underlineLineStyle->setCurrentIndex(5);
            break;
        case KoCharacterStyle::SolidLine:
        default:
            widget.underlineStyle->setCurrentIndex(0);
            break;
    }
    widget.underlineStyle->setCurrentIndex( style->underlineType() );

    switch(style->transform()) {
        case KoCharacterStyle::MixedCase: widget.normal->setChecked(true); break;
        case KoCharacterStyle::SmallCaps: widget.smallcaps->setChecked(true); break;
        case KoCharacterStyle::AllUppercase: widget.uppercase->setChecked(true); break;
        case KoCharacterStyle::AllLowercase: widget.lowercase->setChecked(true); break;
        case KoCharacterStyle::Capitalize: widget.capitalize->setChecked(true); break;
    }

    underlineChanged(widget.underlineStyle->currentIndex());
    widget.underlineColor->setColor(style->underlineColor());

    widget.strikethrough->setChecked(style->strikeOutStyle() != KoCharacterStyle::NoLineStyle);
}

void CharacterHighlighting::save() {
    if(m_style == 0)
        return;
    if (widget.underlineStyle->currentIndex() == 0) {
        m_style->setUnderlineType(KoCharacterStyle::NoLineType);
        m_style->setUnderlineStyle(KoCharacterStyle::NoLineStyle);
    }
    else {
        m_style->setUnderlineType( static_cast<KoCharacterStyle::LineType>(widget.underlineStyle->currentIndex()));
        KoCharacterStyle::LineStyle style;
        switch(widget.underlineLineStyle->currentIndex()) {
            case 1: style = KoCharacterStyle::DashLine; break;
            case 2: style = KoCharacterStyle::DottedLine; break;
            case 3: style = KoCharacterStyle::DotDashLine; break;
            case 4: style = KoCharacterStyle::DotDotDashLine; break;
            case 5: style = KoCharacterStyle::WaveLine; break;
            case 0:
            default:
                style = KoCharacterStyle::SolidLine; break;
        }
        m_style->setUnderlineStyle( style);
        m_style->setUnderlineColor(widget.underlineColor->color());
    }

    if (widget.strikethrough->isChecked())
        m_style->setStrikeOutStyle(KoCharacterStyle::SolidLine);
    else
        m_style->setStrikeOutStyle(KoCharacterStyle::NoLineStyle);

    if (widget.normal->isChecked())
        m_style->setTransform(KoCharacterStyle::MixedCase);
    else if (widget.smallcaps->isChecked())
        m_style->setTransform(KoCharacterStyle::SmallCaps);
    else if (widget.uppercase->isChecked())
        m_style->setTransform(KoCharacterStyle::AllUppercase);
    else if (widget.lowercase->isChecked())
        m_style->setTransform(KoCharacterStyle::AllLowercase);
    else if (widget.capitalize->isChecked())
        m_style->setTransform(KoCharacterStyle::Capitalize);
}

#include "CharacterHighlighting.moc"

