/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceItemDelegate.h"

#include <QPainter>
#include <QDebug>

#include "KisResourceModel.h"

KisResourceItemDelegate::KisResourceItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
    , m_checkerPainter(4)
{
}

void KisResourceItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    if (!index.isValid()) return;

    painter->save();

    m_thumbnailPainter.paint(painter, index, option.rect, option.palette, option.state & QStyle::State_Selected, true);

    painter->restore();
}

QSize KisResourceItemDelegate::sizeHint(const QStyleOptionViewItem &optionItem, const QModelIndex &) const
{
    return optionItem.decorationSize;
}
