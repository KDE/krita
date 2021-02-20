/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

#include <KisPaletteChooser.h>
#include <KisResourceItemView.h>
#include <KisResourceItemChooser.h>
#include <KoColorSet.h>

struct KisPaletteChooserPrivate
{
    class View;
    class Delegate;
    class Model;
    KisPaletteChooserPrivate(KisPaletteChooser *);
    virtual ~KisPaletteChooserPrivate();

    bool allowModification;

    QPointer<KisPaletteChooser> c;

    QSharedPointer<KisResourceItemChooser> itemChooser;

    QScopedPointer<Delegate> delegate;

    QScopedPointer<QAction> actAdd;
    QScopedPointer<QAction> actImport;
    QScopedPointer<QAction> actExport;
    QScopedPointer<QAction> actModify;
    QScopedPointer<QAction> actRemove;
};

class KisPaletteChooserPrivate::Delegate : public QAbstractItemDelegate
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
