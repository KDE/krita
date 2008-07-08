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

#include "ParagraphDecorations.h"

#include <KDebug>

ParagraphDecorations::ParagraphDecorations( QWidget* parent )
    : QWidget( parent ),
    m_style(0)
{
    widget.setupUi(this);

    connect(widget.backgroundColor, SIGNAL(changed(const QColor&)), this, SLOT( backgroundColorChanged()));
    connect(widget.resetBackgroundColor, SIGNAL(clicked()), this, SLOT(clearBackgroundColor()));
}

void ParagraphDecorations::open(KoParagraphStyle *style) {
    m_style = style;
    m_backgroundColorChanged = false;
    m_backgroundColorReset = ! m_style->hasProperty(QTextFormat::BackgroundBrush);
    if (m_backgroundColorReset) {
        clearBackgroundColor();
    } else {
        widget.backgroundColor->setColor(m_style->background().color());
    }
}

void ParagraphDecorations::save() const {
    Q_ASSERT(m_style);
    if(m_backgroundColorReset) {
        // clearing the property doesn't work since ParagraphSettingsDialog does a mergeBlockFormat 
        // so we'll set it to an invalid brush instead
        QBrush brush;
        m_style->setBackground(brush);
    } else if(m_backgroundColorChanged) {
        m_style->setBackground(QBrush(widget.backgroundColor->color()));
    }
}

void ParagraphDecorations::clearBackgroundColor() {
    widget.backgroundColor->setColor(widget.backgroundColor->defaultColor());
    m_backgroundColorReset = true;
}

#include "ParagraphDecorations.moc"
