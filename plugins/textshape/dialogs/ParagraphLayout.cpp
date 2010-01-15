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

#include "ParagraphLayout.h"

#include <KoParagraphStyle.h>

ParagraphLayout::ParagraphLayout(QWidget *parent)
        : QWidget(parent)
{
    widget.setupUi(this);

    connect(widget.right, SIGNAL(toggled(bool)), this, SLOT(slotAlignChanged()));
    connect(widget.center, SIGNAL(toggled(bool)), this, SLOT(slotAlignChanged()));
    connect(widget.justify, SIGNAL(toggled(bool)), this, SLOT(slotAlignChanged()));
    connect(widget.left, SIGNAL(toggled(bool)), this, SLOT(slotAlignChanged()));
}

void ParagraphLayout::slotAlignChanged()
{
    Qt::Alignment align;
    if (widget.right->isChecked())
        align = Qt::AlignRight;
    else if (widget.center->isChecked())
        align = Qt::AlignHCenter;
    else if (widget.justify->isChecked())
        align = Qt::AlignJustify;
    else
        align = Qt::AlignLeft;

    emit horizontalAlignmentChanged(align);
}


void ParagraphLayout::setDisplay(KoParagraphStyle *style)
{
    switch (style->alignment()) {
    case Qt::AlignRight: widget.right->setChecked(true); break;
    case Qt::AlignHCenter: widget.center->setChecked(true); break;
    case Qt::AlignJustify: widget.justify->setChecked(true); break;
    case Qt::AlignLeft:
    default:
        widget.left->setChecked(true); break;
    }

    widget.keepTogether->setChecked(style->nonBreakableLines());
    widget.breakBefore->setChecked(style->breakBefore());
    widget.breakAfter->setChecked(style->breakAfter());
}

void ParagraphLayout::save(KoParagraphStyle *style)
{
    Qt::Alignment align;
    if (widget.right->isChecked())
        align = Qt::AlignRight;
    else if (widget.center->isChecked())
        align = Qt::AlignHCenter;
    else if (widget.justify->isChecked())
        align = Qt::AlignJustify;
    else
        align = Qt::AlignLeft;
    style->setAlignment(align);
    style->setNonBreakableLines(widget.keepTogether->isChecked());
    style->setBreakBefore(widget.breakBefore->isChecked());
    style->setBreakAfter(widget.breakAfter->isChecked());
}

#include <ParagraphLayout.moc>
