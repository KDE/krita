/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KISPALETTELISTWIDGET_P_H
#define KISPALETTELISTWIDGET_P_H

#include <QAbstractItemDelegate>
#include <QListView>
#include <QAbstractListModel>
#include <QPointer>
#include <QCheckBox>
#include <QAction>
#include <QPainter>

#include <KisPaletteListWidget.h>
#include <KisResourceItemView.h>
#include <KisResourceItemChooser.h>
#include <KoColorSet.h>

struct KisPaletteListWidgetPrivate
{
    class View;
    class Delegate;
    class Model;
    KisPaletteListWidgetPrivate(KisPaletteListWidget *);
    virtual ~KisPaletteListWidgetPrivate();

    bool allowModification;

    QPointer<KisPaletteListWidget> c;

    QSharedPointer<KisResourceItemChooser> itemChooser;

    QScopedPointer<Delegate> delegate;

    QScopedPointer<QAction> actAdd;
    QScopedPointer<QAction> actImport;
    QScopedPointer<QAction> actExport;
    QScopedPointer<QAction> actModify;
    QScopedPointer<QAction> actRemove;
};

class KisPaletteListWidgetPrivate::Delegate : public QAbstractItemDelegate
{
public:
    Delegate(QObject *);
    virtual ~Delegate();
    void paint(QPainter * painter,
               const QStyleOptionViewItem & option,
               const QModelIndex & index) const override;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override;
};

#endif // KISPALETTELISTWIDGET_P_H
