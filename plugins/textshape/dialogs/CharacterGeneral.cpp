/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "CharacterStyleOptions.h"
#include "CharacterDecorations.h"
#include "CharacterHighlighting.h"

#include <KoStyleManager.h>
#include <KoCharacterStyle.h>
#include <kfontdialog.h>

CharacterGeneral::CharacterGeneral(QWidget *parent)
    :QWidget(parent),
    m_blockSignals(false),
    m_style(0)
{
    widget.setupUi(this);

    m_styleOptions = new CharacterStyleOptions (true, this);
    m_characterDecorations = new CharacterDecorations (this);
    m_characterHighlighting = new CharacterHighlighting (this);

    QWidget *fonts = new QWidget(this);
    QLayout *layout = new QVBoxLayout(fonts);
    fonts->setLayout(layout);

    QStringList list;
    KFontChooser::getFontList(list, KFontChooser::SmoothScalableFonts);
    m_fontChooser = new KFontChooser(this, false, list, false);
    m_fontChooser->setSampleBoxVisible( false );
    layout->addWidget(m_fontChooser);

    widget.tabs->addTab(fonts, i18n("Font"));
    widget.tabs->addTab(m_characterDecorations, i18n("Decorations"));
    widget.tabs->addTab(m_characterHighlighting, i18n("Highlighting"));
    widget.tabs->addTab(m_styleOptions, i18n("Layout"));
    // TODO language

    connect(widget.name, SIGNAL(textChanged (const QString &)), this, SIGNAL(nameChanged(const QString&)));
    connect(widget.name, SIGNAL(textChanged (const QString &)), this, SLOT(setName(const QString&)));
    //connect( m_fontChooser, SIGNAL( fontSelected( const QFont & ) ), this, SIGNAL( fontChanged( const QFont & ) ) );
}

void CharacterGeneral::setStyle(KoCharacterStyle *style) {
    m_style = style;
    if(m_style == 0)
        return;
    m_blockSignals = true;

    widget.name->setText(style->name());
    m_fontChooser->setFont(style->font());
    m_styleOptions->open(style);
    m_characterDecorations->open(style);
    m_characterHighlighting->open(style);

    m_blockSignals = false;
}

void CharacterGeneral::save() {
    if(m_style == 0) return;
    m_characterDecorations->save();
    m_characterHighlighting->save();
    m_styleOptions->save();
    QFont font = m_fontChooser->font();
    m_style->setFontFamily(font.family());
    m_style->setFontPointSize(font.pointSizeF());
    m_style->setFontWeight(font.weight());
    m_style->setFontItalic(font.italic());
}

void CharacterGeneral::switchToGeneralTab() {
    widget.tabs->setCurrentIndex(0);
}

void CharacterGeneral::setName(const QString &name) {
    m_style->setName(name);
}

#include <CharacterGeneral.moc>
