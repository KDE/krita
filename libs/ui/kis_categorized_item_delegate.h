/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_CATEGORIZED_ITEM_DELEGATE_H_
#define _KIS_CATEGORIZED_ITEM_DELEGATE_H_

#include <kritaui_export.h>
#include <QIcon>
#include <QStyledItemDelegate>

/**
 * This delegate draw categories using information from a QSortFilterProxyModel.
 */
class KRITAUI_EXPORT KisCategorizedItemDelegate: public QStyledItemDelegate
{
public:
    KisCategorizedItemDelegate(QObject *parent);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    void paintTriangle(QPainter* painter, qint32 x, qint32 y, qint32 size, bool rotate) const;
    mutable qint32 m_minimumItemHeight;
};

#endif // _KIS_CATEGORIZED_ITEM_DELEGATE_H_
