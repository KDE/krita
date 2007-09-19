/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_tree_view_popup.h"

#include <QHeaderView>
#include <QTreeView>

#include <kdebug.h>

struct KisTreeViewPopup::Private {
    QTreeView* view;
};


KisTreeViewPopup::KisTreeViewPopup(QWidget* parent) : KisPopupButton(parent), d(new Private)
{
    d->view = new QTreeView;
    d->view->header()->hide();
    setPopupWidget(d->view);
    connect(d->view, SIGNAL(entered(const QModelIndex&)), SLOT(setCurrentIndex(const QModelIndex &)));
    connect(d->view, SIGNAL(clicked(const QModelIndex&)), SLOT(setCurrentIndex(const QModelIndex &)));
    connect(d->view, SIGNAL(activated(const QModelIndex&)), SLOT(setCurrentIndex(const QModelIndex &)));
}

void KisTreeViewPopup::setModel(QAbstractItemModel* model)
{
    d->view->setModel(model);
}

void KisTreeViewPopup::setCurrentIndex(const QModelIndex& idx)
{
    setText( idx.data(Qt::DisplayRole).toString() );
    hidePopupWidget();
}

#include "kis_tree_view_popup.moc"
