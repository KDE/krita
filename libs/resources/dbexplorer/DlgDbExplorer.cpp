/*
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#include "DlgDbExplorer.h"

#include <klocalizedstring.h>
#include <kis_debug.h>

#include <QDataWidgetMapper>
#include <QTableView>
#include <QtSql>

class StorageDelegate : public QSqlRelationalDelegate
{
    Q_OBJECT
public:
    StorageDelegate(QObject *parent)
        : QSqlRelationalDelegate(parent)
    {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem viewItemOption(option);
        // Only do this if we are accessing the column with boolean variables.
        if (index.column() == 4 || index.column() == 5) {
            // This basically changes the rectangle in which the check box is drawn.
            const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
            QRect newRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                                QSize(option.decorationSize.width() +
                                                      5,option.decorationSize.height()),
                                                QRect(option.rect.x() + textMargin, option.rect.y(),
                                                      option.rect.width() -
                                                      (2 * textMargin), option.rect.height()));
            viewItemOption.rect = newRect;
        }
        // Draw the check box using the new rectangle.
        QSqlRelationalDelegate::paint(painter, viewItemOption, index);
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        return QSqlRelationalDelegate::sizeHint(option, index);
    }

    bool editorEvent(QEvent */*event*/, QAbstractItemModel */*model*/,
                     const QStyleOptionViewItem &/*option*/,
                     const QModelIndex &/*index*/) override
    {
        return false;
    }

    QWidget *createEditor(QWidget */*parent*/, const QStyleOptionViewItem &/*option*/,
                          const QModelIndex &/*index*/) const override
    {
        return nullptr;
    }
};

class StorageModel : public QSqlRelationalTableModel
{
public:
    StorageModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase())
        : QSqlRelationalTableModel(parent, db)
    {}

    ~StorageModel() override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

StorageModel::~StorageModel()
{
}

Qt::ItemFlags StorageModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QSqlRelationalTableModel::flags((index));
    if (index.column() == 4 || index.column() == 5) {
        f |= Qt::ItemIsUserCheckable;
    }
    return f;
}

QVariant StorageModel::data(const QModelIndex &index, int role) const
{
    QVariant d = QSqlRelationalTableModel::data(index, Qt::DisplayRole);
    if (role == Qt::DisplayRole) {
        if (index.column() == 3) {
            d = QVariant::fromValue<QString>(QDateTime::fromSecsSinceEpoch(d.toInt()).toString());
        }
        if (index.column() == 4 || index.column() == 5) {
            return QVariant();
        }
    }
    else if (role == Qt::CheckStateRole) {
        if (index.column() == 4 || index.column() == 5) {
            if (d.toInt() == 0) {
                return Qt::Unchecked;
            }
            else {
                return Qt::Checked;
            }
        }
        return QVariant();
    }
    return d;
}

class TagDelegate : public QSqlRelationalDelegate
{
    Q_OBJECT
public:
    TagDelegate(QObject *parent)
        : QSqlRelationalDelegate(parent)
    {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem viewItemOption(option);
        // Only do this if we are accessing the column with boolean variables.
        if (index.column() == 5) {
            // This basically changes the rectangle in which the check box is drawn.
            const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
            QRect newRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                                QSize(option.decorationSize.width() +
                                                      5,option.decorationSize.height()),
                                                QRect(option.rect.x() + textMargin, option.rect.y(),
                                                      option.rect.width() -
                                                      (2 * textMargin), option.rect.height()));
            viewItemOption.rect = newRect;
        }
        // Draw the check box using the new rectangle.
        QSqlRelationalDelegate::paint(painter, viewItemOption, index);
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        return QSqlRelationalDelegate::sizeHint(option, index);
    }

    bool editorEvent(QEvent */*event*/, QAbstractItemModel */*model*/,
                     const QStyleOptionViewItem &/*option*/,
                     const QModelIndex &/*index*/) override
    {
        return false;
    }

    QWidget *createEditor(QWidget */*parent*/, const QStyleOptionViewItem &/*option*/,
                          const QModelIndex &/*index*/) const override
    {
        return nullptr;
    }
};


class TagModel : public QSqlRelationalTableModel
{
public:
    TagModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase())
        : QSqlRelationalTableModel(parent, db)
    {}

    ~TagModel() override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

TagModel::~TagModel()
{
}

Qt::ItemFlags TagModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QSqlRelationalTableModel::flags((index));
    if (index.column() == 5) {
        f |= Qt::ItemIsUserCheckable;
    }
    return f;
}

QVariant TagModel::data(const QModelIndex &index, int role) const
{
    QVariant d = QSqlRelationalTableModel::data(index, Qt::DisplayRole);
    if (role == Qt::DisplayRole) {
        if (index.column() == 5) {
            return QVariant();
        }
    }
    else if (role == Qt::CheckStateRole) {
        if (index.column() == 5) {
            if (d.toInt() == 0) {
                return Qt::Unchecked;
            }
            else {
                return Qt::Checked;
            }
        }
        return QVariant();
    }
    return d;
}


DlgDbExplorer::DlgDbExplorer(QWidget *parent)
    : KoDialog(parent)
{
    setCaption(i18n("Resources Cache Database Explorer"));

    setButtons(Ok);

    m_page = new WdgDbExplorer(this);
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);

    QSqlRelationalTableModel *storagesModel = new StorageModel(this, QSqlDatabase::database());
    storagesModel->setTable("storages");
    storagesModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
    storagesModel->setHeaderData(1, Qt::Horizontal, i18n("Type"));
    storagesModel->setRelation(1, QSqlRelation("storage_types", "id", "name"));
    storagesModel->setHeaderData(2, Qt::Horizontal, i18n("Location"));
    storagesModel->setHeaderData(3, Qt::Horizontal, i18n("Creation Date"));
    storagesModel->setHeaderData(4, Qt::Horizontal, i18n("Preinstalled"));
    storagesModel->setHeaderData(5, Qt::Horizontal, i18n("Active"));
    storagesModel->select();
    m_page->tableStorages->setModel(storagesModel);
    m_page->tableStorages->hideColumn(0);
    m_page->tableStorages->setItemDelegate(new StorageDelegate(m_page->tableStorages));
    m_page->tableStorages->setSelectionMode(QAbstractItemView::SingleSelection);;
    m_page->tableStorages->resizeColumnsToContents();

    QSqlTableModel *resourcesModel = new QSqlTableModel(this, QSqlDatabase::database());
    resourcesModel->setTable("resources");
    resourcesModel->select();
    m_page->tableResources->setModel(resourcesModel);
    m_page->tableResources->hideColumn(0);
    m_page->tableResources->setSelectionMode(QAbstractItemView::SingleSelection);;

    QSqlRelationalTableModel *tagsModel = new TagModel(this, QSqlDatabase::database());
    tagsModel->setTable("tags");
    tagsModel->select();
    tagsModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
    tagsModel->setHeaderData(1, Qt::Horizontal, i18n("Type"));
    tagsModel->setRelation(1, QSqlRelation("resource_types", "id", "name"));
    tagsModel->setHeaderData(2, Qt::Horizontal, i18n("Tag"));
    tagsModel->setHeaderData(3, Qt::Horizontal, i18n("Name"));
    tagsModel->setHeaderData(4, Qt::Horizontal, i18n("Comment"));
    tagsModel->setHeaderData(5, Qt::Horizontal, i18n("Active"));

    m_page->tableTags->setModel(tagsModel);
    m_page->tableTags->hideColumn(0);
    m_page->tableTags->setItemDelegate(new TagDelegate(m_page->tableTags));
    m_page->tableTags->setSelectionMode(QAbstractItemView::SingleSelection);
    m_page->tableTags->resizeColumnsToContents();

    {
        QSqlTableModel *versionModel = new QSqlTableModel(this, QSqlDatabase::database());
        versionModel->setTable("version_information");
        versionModel->setHeaderData(0, Qt::Horizontal, "id");
        versionModel->setHeaderData(1, Qt::Horizontal, "database_version");
        versionModel->setHeaderData(2, Qt::Horizontal, "krita_version");
        versionModel->setHeaderData(3, Qt::Horizontal, "creation_date");
        versionModel->select();
        QSqlRecord r = versionModel->record(0);

        m_page->lblDatabaseVersion->setText(r.value("database_version").toString());
        m_page->lblKritaVersion->setText(r.value("krita_version").toString());
        m_page->lblCreationDate->setText(r.value("creation_date").toString());
    }


}

DlgDbExplorer::~DlgDbExplorer()
{
}

#include "DlgDbExplorer.moc"
