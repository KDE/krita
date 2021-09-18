/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QAbstractItemView>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QDebug>
#include <QMessageBox>

#include <QListView>

#include "KisResourceTypes.h"
#include "KisResourceModel.h"
#include "KisStorageChooserWidget.h"
#include "KisStorageModel.h"
#include "KisStorageFilterProxyModel.h"
#include <KoIcon.h>

KisStorageChooserDelegate::KisStorageChooserDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

void KisStorageChooserDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) return;

    painter->save();

    QString name = index.sibling(index.row(), KisStorageModel::DisplayName).data(Qt::DisplayRole).value<QString>();
    QString location = index.sibling(index.row(), KisStorageModel::Location).data(Qt::DisplayRole).value<QString>();
    bool active = index.data(Qt::UserRole + KisStorageModel::Active).value<bool>();
    QString storageType = index.data(Qt::UserRole + KisStorageModel::StorageType).value<QString>();

    QImage thumbnail = index.data(Qt::UserRole +  + KisStorageModel::Thumbnail).value<QImage>();

    qreal devicePixelRatioF = painter->device()->devicePixelRatioF();

    if (thumbnail.isNull()) {
        //fallback on cute icons.
        thumbnail = koIcon("warning").pixmap(option.decorationSize).toImage();
        if (storageType == "Folder") {
            thumbnail = koIcon("document-open").pixmap(option.decorationSize).toImage();
        }
        else if (storageType == "Adobe Style Library") {
            thumbnail = koIcon("layer-style-enabled").pixmap(option.decorationSize).toImage();
            if (!thumbnail.isNull()) {
                thumbnail = thumbnail.scaled(option.decorationSize, Qt::KeepAspectRatio, Qt::FastTransformation);
            }
        }
        else if (storageType == "Adobe Brush Library") {
            thumbnail = koIcon("select-all").pixmap(option.decorationSize).toImage();
        }
        else if (storageType == "Memory") {
            if (location != "memory") {
                thumbnail = koIcon("document-new").pixmap(option.decorationSize).toImage();
            } else {
                thumbnail = koIcon("drive-harddisk").pixmap(option.decorationSize).toImage();
            }

        }
        else if (storageType == "Bundle") {
            thumbnail = koIcon("bundle_archive").pixmap(option.decorationSize).toImage();
        }

    } else {
        if (!thumbnail.isNull()) {
            thumbnail = thumbnail.scaled(option.decorationSize*devicePixelRatioF, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        thumbnail.setDevicePixelRatio(devicePixelRatioF);
    }

    QColor penColor(option.palette.text().color());

    QStyleOptionViewItem opt = option;

    if (active) {
        opt.state = QStyle::State_Sunken;
    }

    QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, painter);

    painter->setPen(penColor);
    painter->drawImage(option.rect.topLeft()+QPoint(4, 4), thumbnail, thumbnail.rect());
    QRect text = option.rect;
    text.setLeft(text.left()+option.decorationSize.width()+8);
    text.setTop(text.top()+4);
    painter->drawText(text, Qt::TextWordWrap, name.split("_").join(" "));

    painter->restore();
}

QSize KisStorageChooserDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    int w = 200;
    int h = option.decorationSize.height()+8;
    return QSize(w, h);
}

KisStorageChooserWidget::KisStorageChooserWidget(QWidget *parent) : KisPopupButton(parent)
{
    QListView *view = new QListView(this);

    KisStorageFilterProxyModel *proxyModel = new KisStorageFilterProxyModel(this);
    proxyModel->setSourceModel(KisStorageModel::instance());
    proxyModel->setFilter(KisStorageFilterProxyModel::ByStorageType,
                          QStringList()
                          << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::Bundle));
    view->setModel(proxyModel);
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

    KisStorageFilterProxyModel proxy;
    proxy.setSourceModel(KisStorageModel::instance());
    proxy.setFilter(KisStorageFilterProxyModel::ByStorageType,
                    QStringList()
                    << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::Bundle));

    QString warning;
    if (!proxy.rowCount()) {
        warning = i18n("All bundles have been deactivated.");
    }

    KisResourceModel resourceModel(ResourceType::PaintOpPresets);
    resourceModel.setResourceFilter(KisResourceModel::ShowActiveResources);
    if (!resourceModel.rowCount()) {
        warning += i18n("\nThere are no brush presets available. Please re-enable a bundle that provides brush presets.");
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), warning);
    }
}

KisStorageChooserWidget::~KisStorageChooserWidget()
{

}


