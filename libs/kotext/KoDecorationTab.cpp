/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006 Thomas Zander <zander@kde.org>

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

#include "KoDecorationTab.h"

KoDecorationTab::KoDecorationTab( QWidget* parent )
    : QWidget( parent )
{
    widget.setupUi(this);

    connect(widget.textColor, SIGNAL(changed(const QColor&)), this, SLOT(textColorChanged()));
    connect(widget.backgroundColor, SIGNAL(changed(const QColor&)), this, SLOT( backgroundColorChanged()));

    connect(widget.resetTextColor, SIGNAL(clicked()), this, SLOT(clearTextColor()));
    connect(widget.resetBackground, SIGNAL(clicked()), this, SLOT(clearBackgroundColor()));

    widget.shadowGroupBox->setVisible(false);
}

void KoDecorationTab::open(const QTextCharFormat &format) {
    m_textColorChanged = false;
    m_backgroundColorChanged = false;
    m_textColorReset = ! format.hasProperty(QTextFormat::ForegroundBrush);
    if(!m_textColorReset)
        widget.textColor->setColor(format.foreground().color());
    m_backgroundColorReset = ! format.hasProperty(QTextFormat::BackgroundBrush);
    if(!m_backgroundColorReset)
        widget.backgroundColor->setColor(format.background().color());
}

void KoDecorationTab::save(QTextCharFormat &format) const {
    if(m_backgroundColorReset)
        format.clearBackground();
    else if(m_backgroundColorChanged)
        format.setBackground(QBrush(widget.backgroundColor->color()));
    if(m_textColorReset)
        format.clearForeground();
    else if(m_textColorChanged)
        format.setForeground(QBrush(widget.textColor->color()));
}

void KoDecorationTab::clearTextColor() {
    widget.textColor->setColor(widget.textColor->defaultColor());
    m_textColorReset = true;
}

void KoDecorationTab::clearBackgroundColor() {
    widget.backgroundColor->setColor(widget.backgroundColor->defaultColor());
    m_backgroundColorReset = true;
}

#include "KoDecorationTab.moc"
