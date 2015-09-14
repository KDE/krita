/* This file is part of the KDE project
 * Copyright (c) 2013 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#include "KoColorPopupButton.h"

#include <QResizeEvent>
#include <QStyle>
#include <QStyleOptionToolButton>

#include "WidgetsDebug.h"

KoColorPopupButton::KoColorPopupButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
}

KoColorPopupButton::~KoColorPopupButton()
{
}

QSize KoColorPopupButton::sizeHint() const
{
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    return style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(16,16), this);
}

void KoColorPopupButton::resizeEvent(QResizeEvent *e)
{
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    QSize size = iconSize();

    QSize rect = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, size, this);
    int iconWidth = size.width() - rect.width() + e->size().width();

    if (iconWidth != size.width()) {
        size.setWidth(iconWidth);
        setIconSize(size);
    }
    QToolButton::resizeEvent(e);

    emit iconSizeChanged();
}
