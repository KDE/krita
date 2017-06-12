/*
 *  Copyright (C) 2008 Jeremy Whiting <jpwhiting@kde.org>
 *  Copyright (C) 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *  Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>
 *  Copyright (c) 2017 Aniketh Girish anikethgireesh@gmail.com
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

#ifndef ITEMSGRIDVIEWDELEGATE_P_H
#define ITEMSGRIDVIEWDELEGATE_P_H

#include "itemsviewbasedelegate_p.h"

class QToolButton;

static const int ItemGridHeight = 202;
static const int ItemGridWidth = 158;

static const int FrameThickness = 5;
static const int ItemMargin = 2;
class ItemsGridViewDelegate: public ItemsViewBaseDelegate
{
    Q_OBJECT
public:
    explicit ItemsGridViewDelegate(QAbstractItemView *itemView, KNSCore::Engine *engine, QObject *parent = nullptr);
    ~ItemsGridViewDelegate();

    // paint the item at index with all its attributes shown
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    // get the list of widgets
    QList<QWidget *> createItemWidgets(const QModelIndex &index) const Q_DECL_OVERRIDE;

    // update the widgets
    virtual void updateItemWidgets(const QList<QWidget *> widgets,
                                   const QStyleOptionViewItem &option,
                                   const QPersistentModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    void createOperationBar();
    void displayOperationBar(const QRect &rect, const QModelIndex &index);

private:
    QWidget *m_operationBar;
    QToolButton *m_detailsButton;
    QToolButton *m_installButton;

    QModelIndex m_oldIndex;
    mutable int m_elementYPos;
};

#endif // ITEMSGRIDVIEWDELEGATE_P_H
