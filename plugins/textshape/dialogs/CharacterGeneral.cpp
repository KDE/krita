/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#include "CharacterGeneral.h"
#include "FontLayoutTab.h"
#include "FontTab.h"
#include "CharacterHighlighting.h"
#include "LanguageTab.h"
#include "FontDecorations.h"
#include "FormattingPreview.h"

#include <KoStyleManager.h>
#include <KoCharacterStyle.h>

#include "kdebug.h"

CharacterGeneral::CharacterGeneral(QWidget *parent, bool uniqueFormat)
        : QWidget(parent),
        m_blockSignals(false),
        m_style(0)
{
    widget.setupUi(this);

    m_layoutTab = new FontLayoutTab(true, uniqueFormat, this);

    m_characterDecorations = new FontDecorations(uniqueFormat, this);
    connect(m_characterDecorations, SIGNAL(backgroundColorChanged(QColor)), this, SLOT(slotBackgroundColorChanged(QColor)));
    connect(m_characterDecorations, SIGNAL(textColorChanged(QColor)), this, SLOT(slotTextColorChanged(QColor)));

    m_characterHighlighting = new CharacterHighlighting(uniqueFormat, this);
    connect(m_characterHighlighting, SIGNAL(underlineChanged(KoCharacterStyle::LineType, KoCharacterStyle::LineStyle, QColor)), this, SLOT(slotUnderlineChanged(KoCharacterStyle::LineType, KoCharacterStyle::LineStyle, QColor)));
    connect(m_characterHighlighting, SIGNAL(strikethroughChanged(KoCharacterStyle::LineType, KoCharacterStyle::LineStyle, QColor)), this, SLOT(slotStrikethroughChanged(KoCharacterStyle::LineType, KoCharacterStyle::LineStyle, QColor)));
    connect(m_characterHighlighting, SIGNAL(capitalizationChanged(QFont::Capitalization)), this, SLOT(slotCapitalizationChanged(QFont::Capitalization)));

    m_fontTab = new FontTab(uniqueFormat, this);
    connect(m_fontTab, SIGNAL(fontChanged(const QFont &)), this, SLOT(slotFontSelected(const QFont &)));

    m_languageTab = new LanguageTab(uniqueFormat, this);

    widget.tabs->addTab(m_fontTab, i18n("Font"));
    widget.tabs->addTab(m_characterDecorations, i18n("Decorations"));
    widget.tabs->addTab(m_characterHighlighting, i18n("Highlighting"));
    widget.tabs->addTab(m_layoutTab, i18n("Layout"));
    //widget.tabs->addTab(m_languageTab, i18n("Language"));
    m_languageTab->setVisible(false);

    connect(widget.name, SIGNAL(textChanged(const QString &)), this, SIGNAL(nameChanged(const QString&)));
    connect(widget.name, SIGNAL(textChanged(const QString &)), this, SLOT(setName(const QString&)));
}

void CharacterGeneral::hideStyleName(bool hide)
{
    if (hide) {
        disconnect(widget.name, SIGNAL(textChanged(const QString &)), this, SIGNAL(nameChanged(const QString&)));
        disconnect(widget.name, SIGNAL(textChanged(const QString &)), this, SLOT(setName(const QString&)));
        widget.tabs->removeTab(0);
        m_nameHidden = true;
    }
}

void CharacterGeneral::setStyle(KoCharacterStyle *style)
{
    m_style = style;
    if (m_style == 0)
        return;
    m_blockSignals = true;

    if (!m_nameHidden)
        widget.name->setText(style->name());
    m_fontTab->setDisplay(style);
    m_layoutTab->setDisplay(style);
    m_characterDecorations->setDisplay(style);
    m_characterHighlighting->setDisplay(style);
    m_languageTab->setDisplay(style);

    m_blockSignals = false;
}

void CharacterGeneral::save(KoCharacterStyle *style)
{
    KoCharacterStyle *savingStyle;
    if (style == 0) {
        if (m_style == 0)
            return;
        else
            savingStyle = m_style;
    }
    else
        savingStyle = style;

    m_fontTab->save(savingStyle);
    m_characterDecorations->save(savingStyle);
    m_characterHighlighting->save(savingStyle);
    m_layoutTab->save(savingStyle);
    m_languageTab->save(savingStyle);

    emit styleAltered(savingStyle);
}

void CharacterGeneral::switchToGeneralTab()
{
    widget.tabs->setCurrentIndex(0);
}

void CharacterGeneral::setName(const QString &name)
{
    m_style->setName(name);
}

void CharacterGeneral::slotCapitalizationChanged(QFont::Capitalization capitalisation)
{
    widget.preview->setFontCapitalisation(capitalisation);
}

void CharacterGeneral::slotFontSelected(const QFont &font)
{
    widget.preview->setFont(font);
}

void CharacterGeneral::slotBackgroundColorChanged(QColor color)
{
    widget.preview->setBackgroundColor(color);
}

void CharacterGeneral::slotTextColorChanged(QColor color)
{
    widget.preview->setTextColor(color);
}

void CharacterGeneral::slotUnderlineChanged(KoCharacterStyle::LineType lineType, KoCharacterStyle::LineStyle lineStyle, QColor lineColor)
{
    widget.preview->setUnderline(lineType, lineStyle, lineColor);
}

void CharacterGeneral::slotStrikethroughChanged(KoCharacterStyle::LineType lineType, KoCharacterStyle::LineStyle lineStyle, QColor lineColor)
{
    widget.preview->setStrikethrough(lineType, lineStyle, lineColor);
}

#include <CharacterGeneral.moc>
