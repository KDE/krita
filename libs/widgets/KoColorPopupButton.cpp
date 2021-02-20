/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
