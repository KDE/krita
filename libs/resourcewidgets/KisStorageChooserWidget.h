/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISSTORAGECHOOSERWIDGET_H
#define KISSTORAGECHOOSERWIDGET_H

#include <QWidget>
#include <QAbstractItemDelegate>
#include <KisPopupButton.h>

#include "kritaresourcewidgets_export.h"

class KRITARESOURCEWIDGETS_EXPORT KisStorageChooserDelegate  : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit KisStorageChooserDelegate(QObject *parent = 0);
    ~KisStorageChooserDelegate() override {}

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    QSize sizeHint ( const QStyleOptionViewItem &, const QModelIndex & ) const override;

};


class KRITARESOURCEWIDGETS_EXPORT KisStorageChooserWidget : public KisPopupButton
{
    Q_OBJECT
public:
    KisStorageChooserWidget(QWidget *parent = 0);

    ~KisStorageChooserWidget();

private Q_SLOTS:
    void activated(const QModelIndex &index);

};


#endif // KISSTORAGECHOOSERWIDGET_H
