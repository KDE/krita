/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006-2007 Thomas Zander <zander@kde.org>
   Copyright (C)  2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
   Copyright (C)  2011 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>

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

CharacterHighlighting::CharacterHighlighting(bool uniqueFormat,QWidget* parent)
        : QWidget(parent),
        m_uniqueFormat(uniqueFormat)
{
    widget.setupUi(this);

    widget.underlineStyle->addItems(KoText::underlineTypeList());
    widget.underlineLineStyle->addItems(KoText::underlineStyleList());

    widget.strikethroughStyle->addItems(KoText::underlineTypeList()); //TODO make KoText consistent: either add strikethroughTypeList, or change from underlineTypeList to lineTypeList
    widget.strikethroughLineStyle->addItems(KoText::underlineStyleList()); //TODO idem

    connect(widget.underlineStyle, SIGNAL(activated(int)), this, SLOT(underlineTypeChanged(int)));
    connect(widget.underlineLineStyle, SIGNAL(activated(int)), this, SLOT(underlineStyleChanged(int)));
    connect(widget.underlineColor, SIGNAL(changed(QColor)), this, SLOT(underlineColorChanged(QColor)));

    connect(widget.strikethroughStyle, SIGNAL(activated(int)), this, SLOT(strikethroughTypeChanged(int)));
    connect(widget.strikethroughLineStyle, SIGNAL(activated(int)), this, SLOT(strikethroughStyleChanged(int)));
    connect(widget.strikethroughColor, SIGNAL(changed(QColor)), this, SLOT(strikethroughColorChanged(QColor)));

    connect(widget.normal, SIGNAL(toggled(bool)), this, SLOT(capitalisationChanged()));
    connect(widget.smallcaps, SIGNAL(toggled(bool)), this, SLOT(capitalisationChanged()));
    connect(widget.uppercase, SIGNAL(toggled(bool)), this, SLOT(capitalisationChanged()));
    connect(widget.lowercase, SIGNAL(toggled(bool)), this, SLOT(capitalisationChanged()));
    connect(widget.capitalize, SIGNAL(toggled(bool)), this, SLOT(capitalisationChanged()));
}

KoCharacterStyle::LineType CharacterHighlighting::indexToLineType(int index)
{
    KoCharacterStyle::LineType lineType;
    switch (index) {
        case 1: lineType = KoCharacterStyle::SingleLine; break;
        case 2: lineType = KoCharacterStyle::DoubleLine; break;
        case 0:
        default:
            lineType = KoCharacterStyle::NoLineType; break;
    }
    return lineType;
}

KoCharacterStyle::LineStyle CharacterHighlighting::indexToLineStyle(int index)
{
    KoCharacterStyle::LineStyle lineStyle;
    switch (index) {
        case 1: lineStyle = KoCharacterStyle::DashLine; break;
        case 2: lineStyle = KoCharacterStyle::DottedLine; break;
        case 3: lineStyle = KoCharacterStyle::DotDashLine; break;
        case 4: lineStyle = KoCharacterStyle::DotDotDashLine; break;
        case 5: lineStyle = KoCharacterStyle::WaveLine; break;
        case 0:
        default:
            lineStyle = KoCharacterStyle::SolidLine; break;
    }
    return lineStyle;
}

int CharacterHighlighting::lineTypeToIndex(KoCharacterStyle::LineType type)
{
    int index;
    switch (type) {
    case KoCharacterStyle::NoLineType: index = 0; break;
    case KoCharacterStyle::SingleLine: index = 1; break;
    case KoCharacterStyle::DoubleLine: index = 2; break;
    default: index = 0; break;
    }
    return index;
}

int CharacterHighlighting::lineStyleToIndex(KoCharacterStyle::LineStyle type)
{
    int index;
    switch (type) {
    case KoCharacterStyle::SolidLine: index = 0; break;
    case KoCharacterStyle::DashLine: index = 1; break;
    case KoCharacterStyle::DottedLine: index = 2; break;
    case KoCharacterStyle::DotDashLine: index = 3; break;
    case KoCharacterStyle::DotDotDashLine: index = 4; break;
    case KoCharacterStyle::WaveLine: index = 5; break;
    default: index = 0; break;
    }
    return index;
}

void CharacterHighlighting::capitalisationChanged()
{
    if (m_uniqueFormat || widget.groupBox->isChecked()) {
        if (widget.normal->isChecked()) {
            emit capitalizationChanged(QFont::MixedCase);
            m_mixedCaseInherited = false;
        }
        else if (widget.smallcaps->isChecked()) {
            emit capitalizationChanged(QFont::SmallCaps);
            m_smallCapsInherited = false;
        }
        else if (widget.uppercase->isChecked()) {
            emit capitalizationChanged(QFont::AllUppercase);
            m_allUpperCaseInherited = false;
        }
        else if (widget.lowercase->isChecked()) {
            emit capitalizationChanged(QFont::AllLowercase);
            m_allLowerCaseInherited = false;
        }
        else if (widget.capitalize->isChecked()) {
            emit capitalizationChanged(QFont::Capitalize);
            m_capitalizInherited = false;
        }
    }
}

void CharacterHighlighting::underlineTypeChanged(int item)
{
    widget.underlineLineStyle->setEnabled(item > 0);
    widget.underlineColor->setEnabled(item > 0);
    m_underlineTypeInherited = false;
    emit underlineChanged(indexToLineType(item), indexToLineStyle(widget.underlineLineStyle->currentIndex()), widget.underlineColor->color());
}

void CharacterHighlighting::underlineStyleChanged(int item)
{
    if (widget.underlineStyle->currentIndex())
        emit underlineChanged(indexToLineType(widget.underlineStyle->currentIndex()), indexToLineStyle(item), widget.underlineColor->color());
    m_underlineStyleInherited = false;
}

void CharacterHighlighting::underlineColorChanged(QColor color)
{
    if (widget.underlineStyle->currentIndex())
        emit underlineChanged(indexToLineType(widget.underlineStyle->currentIndex()), indexToLineStyle(widget.underlineLineStyle->currentIndex()), color);
}

void CharacterHighlighting::strikethroughTypeChanged(int item)
{
    widget.strikethroughLineStyle->setEnabled(item > 0);
    widget.strikethroughColor->setEnabled(item > 0);
    m_strikeoutTypeInherited = false;
    emit strikethroughChanged(indexToLineType(item), indexToLineStyle(widget.strikethroughLineStyle->currentIndex()), widget.strikethroughColor->color());
}

void CharacterHighlighting::strikethroughStyleChanged(int item)
{
    if (widget.strikethroughStyle->currentIndex())
        emit strikethroughChanged(indexToLineType(widget.strikethroughStyle->currentIndex()), indexToLineStyle(item), widget.strikethroughColor->color());
    m_strikeoutStyleInherited = false;
}

void CharacterHighlighting::strikethroughColorChanged(QColor color)
{
    if (widget.strikethroughStyle->currentIndex())
        emit strikethroughChanged(indexToLineType(widget.strikethroughStyle->currentIndex()), indexToLineStyle(widget.strikethroughLineStyle->currentIndex()), color);
    m_strikeoutcolorInherited = false;
}

void CharacterHighlighting::setDisplay(KoCharacterStyle *style)
{
    if (style == 0)
        return;
    m_underlineStyleInherited = !style->hasProperty(KoCharacterStyle::UnderlineStyle);
    m_underlineTypeInherited = !style->hasProperty(KoCharacterStyle::UnderlineType);
    m_strikeoutStyleInherited = !style->hasProperty(KoCharacterStyle::StrikeOutStyle);
    m_strikeoutTypeInherited = !style->hasProperty(KoCharacterStyle::StrikeOutType);
    m_strikeoutcolorInherited = !style->hasProperty(KoCharacterStyle::StrikeOutColor);
    m_mixedCaseInherited = !style->hasProperty(QFont::MixedCase);
    m_smallCapsInherited = !style->hasProperty(QFont::SmallCaps);
    m_allUpperCaseInherited = !style->hasProperty(QFont::AllUppercase);
    m_allLowerCaseInherited = !style->hasProperty(QFont::AllLowercase);
    m_capitalizInherited = !style->hasProperty(QFont::Capitalize);

//set the underline up
    widget.underlineStyle->setCurrentIndex(1);
    widget.underlineLineStyle->setCurrentIndex(lineStyleToIndex(style->underlineStyle()));
    if (m_uniqueFormat)
        widget.underlineStyle->setCurrentIndex(lineTypeToIndex(style->underlineType()));
    else
        widget.underlineStyle->setCurrentIndex(-1);

    underlineTypeChanged(widget.underlineStyle->currentIndex());
    widget.underlineColor->setColor(style->underlineColor());

//set the strikethrough up
    widget.strikethroughStyle->setCurrentIndex(1);
    widget.strikethroughLineStyle->setCurrentIndex(lineStyleToIndex(style->strikeOutStyle()));
    if (m_uniqueFormat)
        widget.strikethroughStyle->setCurrentIndex(lineTypeToIndex(style->strikeOutType()));
    else
        widget.strikethroughStyle->setCurrentIndex(-1);
    strikethroughTypeChanged(widget.strikethroughStyle->currentIndex());
    widget.strikethroughColor->setColor(style->strikeOutColor());

//Now set the capitalisation
    switch (style->fontCapitalization()) {
    case QFont::MixedCase: widget.normal->setChecked(true); break;
    case QFont::SmallCaps: widget.smallcaps->setChecked(true); break;
    case QFont::AllUppercase: widget.uppercase->setChecked(true); break;
    case QFont::AllLowercase: widget.lowercase->setChecked(true); break;
    case QFont::Capitalize: widget.capitalize->setChecked(true); break;
    }

    widget.groupBox->setCheckable(!m_uniqueFormat);
    widget.groupBox->setChecked(m_uniqueFormat);

    capitalisationChanged();
}

void CharacterHighlighting::save(KoCharacterStyle *style)
{
    if (style == 0)
        return;

    if (widget.underlineStyle->currentIndex() == 0) {
        style->setUnderlineType(KoCharacterStyle::NoLineType);
        style->setUnderlineStyle(KoCharacterStyle::NoLineStyle);
    } else if (widget.underlineStyle->currentIndex() > 0) {
        if (!m_underlineTypeInherited) {
            style->setUnderlineType(indexToLineType(widget.underlineStyle->currentIndex()));
        }
        if (!m_underlineStyleInherited) {
            style->setUnderlineStyle(indexToLineStyle(widget.underlineLineStyle->currentIndex()));
        }
        style->setUnderlineColor(widget.underlineColor->color());
    }

    if (widget.strikethroughStyle->currentIndex() == 0) {
        style->setStrikeOutType(KoCharacterStyle::NoLineType);
        style->setStrikeOutStyle(KoCharacterStyle::NoLineStyle);
    } else if (widget.strikethroughStyle->currentIndex() > 0) {
        if (!m_strikeoutTypeInherited) {
            style->setStrikeOutType(indexToLineType(widget.strikethroughStyle->currentIndex()));
        }
        if (!m_strikeoutStyleInherited) {
            style->setStrikeOutStyle(indexToLineStyle(widget.strikethroughLineStyle->currentIndex()));
        }
        if (!m_strikeoutcolorInherited) {
            style->setStrikeOutColor(widget.strikethroughColor->color());
        }
    }
    if (m_uniqueFormat || widget.groupBox->isChecked()) {
        if (widget.normal->isChecked() && !m_mixedCaseInherited)
            style->setFontCapitalization(QFont::MixedCase);
        else if (widget.smallcaps->isChecked() && !m_smallCapsInherited)
            style->setFontCapitalization(QFont::SmallCaps);
        else if (widget.uppercase->isChecked() && !m_allUpperCaseInherited)
            style->setFontCapitalization(QFont::AllUppercase);
        else if (widget.lowercase->isChecked() && !m_allLowerCaseInherited)
            style->setFontCapitalization(QFont::AllLowercase);
        else if (widget.capitalize->isChecked() && !m_capitalizInherited)
            style->setFontCapitalization(QFont::Capitalize);
    }
}

#include <CharacterHighlighting.moc>
