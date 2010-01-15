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

ParagraphDecorations::ParagraphDecorations(QWidget* parent)
        : QWidget(parent)
{
    widget.setupUi(this);

    connect(widget.backgroundColor, SIGNAL(changed(const QColor&)), this, SLOT(slotBackgroundColorChanged()));
    connect(widget.resetBackgroundColor, SIGNAL(clicked()), this, SLOT(clearBackgroundColor()));
}

void ParagraphDecorations::slotBackgroundColorChanged()
{
    m_backgroundColorReset = false; m_backgroundColorChanged = true;
    emit backgroundColorChanged(widget.backgroundColor->color());
}

void ParagraphDecorations::setDisplay(KoParagraphStyle *style)
{
    m_backgroundColorChanged = false;
    m_backgroundColorReset = style->background().style() == Qt::NoBrush;
    if (m_backgroundColorReset) {
        clearBackgroundColor();
    } else {
        widget.backgroundColor->setColor(style->background().color());
    }
}

void ParagraphDecorations::save(KoParagraphStyle *style) const
{
    Q_ASSERT(style);
    if (m_backgroundColorReset)
        // clearing the property doesn't work since ParagraphSettingsDialog does a mergeBlockFormat
        // so we'll set it to a Qt::NoBrush brush instead
        style->setBackground(QBrush(Qt::NoBrush));
    else if (m_backgroundColorChanged)
        style->setBackground(QBrush(widget.backgroundColor->color()));
}

void ParagraphDecorations::clearBackgroundColor()
{
    widget.backgroundColor->setColor(widget.backgroundColor->defaultColor());
    m_backgroundColorReset = true;
    emit backgroundColorChanged(QColor(Qt::transparent));
}

#include <ParagraphDecorations.moc>
