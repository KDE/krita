/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#include <krita_export.h>
#include <QIcon>
#include <QStyledItemDelegate>

/**
 * This delegate draw categories using information from a \ref KCategorizedSortFilterProxyModel .
 */
class KRITAUI_EXPORT KisCategorizedItemDelegate: public QStyledItemDelegate
{
public:
    KisCategorizedItemDelegate(bool indicateError);
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    
private:
    void paintTriangle(QPainter* painter, qint32 x, qint32 y, qint32 size, bool rotate) const;
    
private:
    bool           m_indicateError;
    mutable qint32 m_minimumItemHeight;
};

#endif // _KIS_CATEGORIZED_ITEM_DELEGATE_H_
