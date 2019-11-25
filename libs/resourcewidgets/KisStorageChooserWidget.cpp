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
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QListView>
#include <QDateTime>
#include <QSizePolicy>

#include "KisStorageChooserWidget.h"
#include "KisStorageModel.h"
#include <KoIcon.h>


KisStorageChooserDelegate::KisStorageChooserDelegate(QAbstractItemView *itemView, QObject *parent)
    : KWidgetItemDelegate(itemView, parent)
{
}

QList<QWidget *> KisStorageChooserDelegate::createItemWidgets(const QModelIndex &index) const
{
    QList<QWidget *> widgetList;
    QWidget *page = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(page);

    QCheckBox *checkBox = new QCheckBox;
    checkBox->setProperty("storageItem", index);
    QLabel *thumbnail = new QLabel;
    QLabel *filename = new QLabel;
    filename->setWordWrap(true);

    layout->addWidget(checkBox);
    layout->addWidget(thumbnail);
    layout->addWidget(filename);

    page->setFixedSize(400, 100);

    return widgetList << page;
}

void KisStorageChooserDelegate::updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const
{
    if (option.rect == QRect()) return;

    QWidget* page= widgets[0];
    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(page->layout());
    QCheckBox *checkBox = qobject_cast<QCheckBox*>(layout->itemAt(0)->widget());
    QLabel *thumbnail = qobject_cast<QLabel*>(layout->itemAt(1)->widget());
    QLabel *filename = qobject_cast<QLabel*>(layout->itemAt(2)->widget());

    checkBox->setChecked(index.data(Qt::UserRole + KisStorageModel::Active).value<bool>());
    checkBox->setFixedWidth(checkBox->height());
    connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(toggleStorage(bool)), Qt::UniqueConnection);

    QString storageType = index.data(Qt::UserRole + KisStorageModel::StorageType).value<QString>();
    if (storageType == "Folder") {
        thumbnail->setPixmap(koIcon("document-open").pixmap(option.decorationSize));
    }
    else if (storageType == "Memory") {
        thumbnail->setPixmap(koIcon("document-new").pixmap(option.decorationSize));
    }
    else {
        thumbnail->setPixmap(koIcon("bundle_archive").pixmap(option.decorationSize));
    }
    thumbnail->setFixedSize(option.decorationSize);
    filename->setText(index.data(Qt::UserRole + KisStorageModel::Location).value<QString>());

    // move the page _up_ otherwise it will draw relative to the actual position
    page->setGeometry(option.rect.translated(0, -option.rect.y()));
}

void KisStorageChooserDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    itemView()->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, 0);

    /**
    QString location = i;
    if (location.isEmpty()) {
        location = QString::number(index.row());
    }
    QColor penColor(option.palette.text().color());
    //penColor.setAlphaF(0.6);
    painter->setPen(penColor);
    painter->drawText(option.rect, 0, location);
    QApplication::style()->drawControl(QStyle::CE_PushButtonBevel, &option, painter, 0);
    **/
}

QSize KisStorageChooserDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(400, 100);
}

void KisStorageChooserDelegate::toggleStorage(bool toggle)
{
    QModelIndex index = sender()->property("storageItem").value<QModelIndex>();
    if (index.isValid()) {
        KisStorageModel::instance()->setData(index, QVariant(toggle), Qt::CheckStateRole);
    }
}

KisStorageChooserWidget::KisStorageChooserWidget(QWidget *parent) : KisPopupButton(parent)
{
    QListView *view = new QListView(this);
//    view->setModel(KisStorageModel::instance());
//    view->setIconSize(QSize(80, 80));
//    view->setItemDelegate(new KisStorageChooserDelegate(view, this));
    this->setPopupWidget(view);
}

KisStorageChooserWidget::~KisStorageChooserWidget()
{

}


