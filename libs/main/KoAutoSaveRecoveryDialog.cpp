/* This file is part of the KDE project
   Copyright (C) 2012 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoAutoSaveRecoveryDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QAbstractTableModel>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QImage>
#include <QPixmap>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QCheckBox>
#include <QLabel>
#include <QDebug>

#include <KoStore.h>

#include <kwidgetitemdelegate.h>
#include <klocale.h>


struct FileItem {

    FileItem() : checked(true) {}

    QImage thumbnail;
    QString name;
    QString date;
    bool checked;
};

class FileItemDelegate : public KWidgetItemDelegate
{
public:

    FileItemDelegate(QAbstractItemView *itemView, KoAutoSaveRecoveryDialog *dlg)
        : KWidgetItemDelegate(itemView, dlg)
        , m_parent(dlg)
    {
    }

    QList<QWidget*> createItemWidgets() const
    {
        // a lump of coal and a piece of elastic will get you through the world
        QModelIndex idx = property("goya:creatingWidgetForIndex").value<QModelIndex>();

        QWidget *page = new QWidget;
        QHBoxLayout *layout = new QHBoxLayout(page);

        QCheckBox *checkBox = new QCheckBox;
        checkBox->setProperty("fileitem", idx.data());

        connect(checkBox, SIGNAL(toggled(bool)), m_parent, SLOT(toggleFileItem(bool)));
        QLabel *thumbnail = new QLabel;
        QLabel *filename = new QLabel;
        QLabel *dateModified = new QLabel;

        layout->addWidget(checkBox);
        layout->addWidget(thumbnail);
        layout->addWidget(filename);
        layout->addWidget(dateModified);

        page->setFixedSize(600, 200);

        return QList<QWidget*>() << page;
    }

    void updateItemWidgets(const QList<QWidget*> widgets,
                           const QStyleOptionViewItem &option,
                           const QPersistentModelIndex &index) const
    {
        FileItem *fileItem = (FileItem*)index.data().value<void*>();

        QWidget* page= widgets[0];
        QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(page->layout());
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(layout->itemAt(0)->widget());
        QLabel *thumbnail = qobject_cast<QLabel*>(layout->itemAt(1)->widget());
        QLabel *filename = qobject_cast<QLabel*>(layout->itemAt(2)->widget());
        QLabel *modified = qobject_cast<QLabel*>(layout->itemAt(3)->widget());

        checkBox->setChecked(fileItem->checked);
        thumbnail->setPixmap(QPixmap::fromImage(fileItem->thumbnail));
        filename->setText(fileItem->name);
        modified->setText(fileItem->date);

        // move the page _up_ otherwise it will draw relative to the actual postion
        page->setGeometry(option.rect.translated(0, -option.rect.y()));
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
    {
        //paint background for selected or hovered item
        QStyleOptionViewItemV4 opt = option;
        itemView()->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, 0);
    }

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
    {
        return QSize(600, 200);
    }


    KoAutoSaveRecoveryDialog *m_parent;
};

class KoAutoSaveRecoveryDialog::FileItemModel : public QAbstractListModel
{
public:
    FileItemModel(QList<FileItem*> fileItems, QObject *parent)
        : QAbstractListModel(parent)
        , m_fileItems(fileItems){}

    virtual ~FileItemModel()
    {
        qDeleteAll(m_fileItems);
        m_fileItems.clear();
    }

    int rowCount(const QModelIndex &/*parent*/) const { return m_fileItems.size(); }

    Qt::ItemFlags flags(const QModelIndex& /*index*/) const
    {
        Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
        return flags;
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        if (index.isValid() && index.row() < m_fileItems.size()) {

            FileItem *item = m_fileItems.at(index.row());

            switch (role) {
            case Qt::DisplayRole:
            {
                return QVariant::fromValue<void*>((void*)item);
            }
            case Qt::SizeHintRole:
                return QSize(600, 200);
            }
        }
        return QVariant();
    }

    bool setData(const QModelIndex& index, const QVariant& /*value*/, int role)
    {
        if (index.isValid() && index.row() < m_fileItems.size()) {
            if (role == Qt::CheckStateRole) {
                m_fileItems.at(index.row())->checked = !m_fileItems.at(index.row())->checked;
                return true;
            }
        }
        return false;
    }
    QList<FileItem *> m_fileItems;
};

KoAutoSaveRecoveryDialog::KoAutoSaveRecoveryDialog(const QStringList &filenames, QWidget *parent) :
    KDialog(parent)
{
    setCaption(i18n("Recover Files"));
    setMinimumSize(650, 500);
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    if (filenames.size() == 1) {
        layout->addWidget(new QLabel(i18n("The following autosave file can be recovered:")));
    }
    else {
        layout->addWidget(new QLabel(i18n("The following autosave files can be recovered:")));
    }

    m_listView = new QListView();
    m_listView->setAcceptDrops(false);
    KWidgetItemDelegate *delegate = new FileItemDelegate(m_listView, this);
    m_listView->setItemDelegate(delegate);

    QList<FileItem*> fileItems;
    foreach(const QString &filename, filenames) {

        FileItem *file = new FileItem();
        file->name = filename;

        QString path = QDir::homePath() + "/" + filename;
        // get thumbnail -- all calligra apps save a thumbnail
        KoStore* store = KoStore::createStore(path, KoStore::Read);

        if (store && (    store->open(QString("Thumbnails/thumbnail.png"))
                          || store->open(QString("preview.png")))) {
            // Hooray! No long delay for the user...
            QByteArray bytes = store->read(store->size());
            store->close();
            delete store;
            QImage img;
            img.loadFromData(bytes);
            file->thumbnail = img.scaled(QSize(200,200), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // get the date
        QDateTime date = QFileInfo(path).lastModified();
        file->date = "(" + date.toString(Qt::LocalDate) + ")";

        fileItems.append(file);
    }

    m_model = new FileItemModel(fileItems, m_listView);
    m_listView->setModel(m_model);
    layout->addWidget(m_listView);
    setMainWidget(page);
}


QStringList KoAutoSaveRecoveryDialog::recoverableFiles()
{
    QStringList files;
    foreach(FileItem* fileItem, m_model->m_fileItems) {
        if (fileItem->checked) {
            files << fileItem->name;
        }
    }
    return files;
}

void KoAutoSaveRecoveryDialog::toggleFileItem(bool toggle)
{
    // I've made better man from a piece of putty and matchstick!
    QVariant v = sender()->property("fileitem") ;
    if (v.isValid()) {
        FileItem *fileItem = (FileItem*)v.value<void*>();
        fileItem->checked = toggle;
    }
}
