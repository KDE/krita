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

#include "CharacterDecorations.h"

#include <KoCharacterStyle.h>

CharacterDecorations::CharacterDecorations( QWidget* parent )
    : QWidget( parent ),
    m_style(0)
{
    widget.setupUi(this);

    connect(widget.textColor, SIGNAL(changed(const QColor&)), this, SLOT(textColorChanged()));
    connect(widget.backgroundColor, SIGNAL(changed(const QColor&)), this, SLOT( backgroundColorChanged()));

    connect(widget.resetTextColor, SIGNAL(clicked()), this, SLOT(clearTextColor()));
    connect(widget.resetBackground, SIGNAL(clicked()), this, SLOT(clearBackgroundColor()));

    widget.shadowGroupBox->setVisible(false);
}

void CharacterDecorations::open(KoCharacterStyle *style) {
    m_style = style;
    if(m_style == 0)
        return;
    m_textColorChanged = false;
    m_backgroundColorChanged = false;
    m_textColorReset = ! style->hasProperty(QTextFormat::ForegroundBrush);
    if(m_textColorReset)
        clearTextColor();
    else
        widget.textColor->setColor(style->foreground().color());
    m_backgroundColorReset = !style->hasProperty(QTextFormat::BackgroundBrush);
    if(m_backgroundColorReset)
        clearBackgroundColor();
    else
        widget.backgroundColor->setColor(style->background().color());
}

void CharacterDecorations::save() {
    if(m_style == 0)
        return;
    if(m_backgroundColorReset)
        m_style->clearBackground();
    else if(m_backgroundColorChanged)
        m_style->setBackground(QBrush(widget.backgroundColor->color()));
    if(m_textColorReset)
        m_style->clearForeground();
    else if(m_textColorChanged)
        m_style->setForeground(QBrush(widget.textColor->color()));
}

void CharacterDecorations::clearTextColor() {
    widget.textColor->setColor(widget.textColor->defaultColor());
    m_textColorReset = true;
}

void CharacterDecorations::clearBackgroundColor() {
    widget.backgroundColor->setColor(widget.backgroundColor->defaultColor());
    m_backgroundColorReset = true;
}

#include "CharacterDecorations.moc"
