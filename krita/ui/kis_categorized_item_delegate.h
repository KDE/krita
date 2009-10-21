/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_CATEGORIZED_ITEM_DELEGATE_H_
#define _KIS_CATEGORIZED_ITEM_DELEGATE_H_

#include <QAbstractItemDelegate>

/**
 * This delegate draw categories using information from a \ref KCategorizedSortFilterProxyModel .
 */
class KisCategorizedItemDelegate : public QAbstractItemDelegate
{
public:
    /**
     * The @p _fallback delegate is used to take care of drawing/editing of the items.
     */
    KisCategorizedItemDelegate(QAbstractItemDelegate* _fallback, QObject* parent = 0);
    ~KisCategorizedItemDelegate();
    virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index);
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const;
    virtual void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;
private:
    struct Private;
    Private* const d;
};

#endif
