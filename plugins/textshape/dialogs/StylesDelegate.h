/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
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
#ifndef STYLESDELEGATE_H
#define STYLESDELEGATE_H

#include <QRect>
#include <QStyledItemDelegate>

class StylesDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    StylesDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model,
                            const QStyleOptionViewItem &option, const QModelIndex &index);

signals:
    void styleManagerButtonClicked(QModelIndex index);
    void deleteStyleButtonClicked(QModelIndex index);
    void needsUpdate(QModelIndex index);
    void clickedInItem(QModelIndex index);

private:
    bool m_editButtonPressed;
    bool m_deleteButtonPressed;

    int m_buttonSize;
    int m_buttonDistance;

    QRect m_delRect;
    QRect m_editRect;
};

#endif
