/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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
#include "CharacterHighlighting.h"
#include "LanguageTab.h"
#include "FontDecorations.h"
#include "FormattingPreview.h"

#include <KoStyleManager.h>
#include <KoCharacterStyle.h>

#include "kdebug.h"

CharacterGeneral::CharacterGeneral(QWidget *parent)
        : QWidget(parent),
        m_blockSignals(false),
        m_style(0)
{
    widget.setupUi(this);

    m_characterHighlighting = new CharacterHighlighting(true, this);
    connect(m_characterHighlighting, SIGNAL(charStyleChanged()), this, SIGNAL(styleChanged()));
    connect(m_characterHighlighting, SIGNAL(charStyleChanged()), this, SLOT(setPreviewCharacterStyle()));

    m_languageTab = new LanguageTab(true, this);

    widget.tabs->addTab(m_characterHighlighting, i18n("Font"));

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

    m_characterHighlighting->setDisplay(style);
    //m_languageTab->setDisplay(style);

    widget.preview->setCharacterStyle(style);

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

    m_characterHighlighting->save(savingStyle);
    //m_languageTab->save(savingStyle);

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

void CharacterGeneral::setPreviewCharacterStyle()
{
    KoCharacterStyle *charStyle=new KoCharacterStyle();
    save(charStyle);
    if (charStyle) {
        widget.preview->setCharacterStyle(charStyle);
    }

    delete charStyle;
}

#include <CharacterGeneral.moc>
