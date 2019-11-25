/* This file is part of the KDE project
 * Copyright (C) 2019 Wolthera van Hövell tot Westerflier<griffinvalley@gmail.com>
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

#include <QAbstractItemView>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QDebug>

#include <QListView>

#include "KisStorageChooserWidget.h"
#include "KisStorageModel.h"
#include <KoIcon.h>


KisStorageChooserDelegate::KisStorageChooserDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

void KisStorageChooserDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) return;

    painter->save();

    QString location = index.data(Qt::UserRole + KisStorageModel::Location).value<QString>();
    location = location.split("/").last();
    location = location.split(".").first();
    location = location.split("_").join(" ");
    bool active = index.data(Qt::UserRole + KisStorageModel::Active).value<bool>();
    QString storageType = index.data(Qt::UserRole + KisStorageModel::StorageType).value<QString>();
    QPixmap picture = QPixmap(option.decorationSize);
    if (storageType == "Folder") {
        picture = koIcon("document-open").pixmap(option.decorationSize);
    }
    else if (storageType == "Memory") {
        picture = koIcon("document-new").pixmap(option.decorationSize);
    }
    else {
        picture = koIcon("bundle_archive").pixmap(option.decorationSize);
    }

    if (location.isEmpty()) {
        location = QString::number(index.row());
    }

    QColor penColor(option.palette.text().color());

    QStyleOptionViewItem opt = option;

    if (active) {
        opt.state = QStyle::State_Sunken;
    }

    QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, painter);

    painter->setPen(penColor);
    painter->drawImage(option.rect.topLeft(), picture.toImage(), picture.rect());
    QRect text = option.rect;
    text.setLeft(text.left()+option.decorationSize.width()+2);
    painter->drawText(text, 0, location);

    painter->restore();
}

QSize KisStorageChooserDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int w = 400;
    int h = option.decorationSize.height();
    return QSize(w, h);
}

KisStorageChooserWidget::KisStorageChooserWidget(QWidget *parent) : KisPopupButton(parent)
{
    QListView *view = new QListView(this);
    view->setModel(KisStorageModel::instance());
    view->setIconSize(QSize(64, 64));
    view->setItemDelegate(new KisStorageChooserDelegate(this));
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(view, SIGNAL(clicked(QModelIndex)), this, SLOT(activated(QModelIndex)));
    this->setPopupWidget(view);
}

void KisStorageChooserWidget::activated(const QModelIndex &index)
{
    if (!index.isValid()) return;

    bool active = index.data(Qt::UserRole + KisStorageModel::Active).value<bool>();
    KisStorageModel::instance()->setData(index, !active, Qt::CheckStateRole);
}

KisStorageChooserWidget::~KisStorageChooserWidget()
{

}


