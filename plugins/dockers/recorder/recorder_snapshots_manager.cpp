/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */


#include "recorder_snapshots_manager.h"
#include "ui_recorder_snapshots_manager.h"
#include "recorder_directory_cleaner.h"
#include "recorder_const.h"

#include <klocalizedstring.h>

#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QLocale>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDialogButtonBox>

namespace
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
// FIXME This is to support Ubuntu 16.04. Remove this after dropping support of Qt < 5.10.
QString qlocale_formattedDataSize(qint64 bytes)
{
    static QStringList suffixes = {"B", "KiB", "MiB", "GiB", "TiB"};
    constexpr const qreal divider = 1024;

    int index = 0;
    qreal size = bytes;

    while (size > divider && index < suffixes.length()) {
        size /= divider;
        ++index;
    }

    return QString("%1 %2").arg(size, 0, 'f', 1).arg(suffixes[index]);
}
#endif


constexpr int valueRole = Qt::UserRole + 1;
constexpr int defaultColumnMargin = 16;

enum Page
{
    PageProgress = 0,
    PageSelection = 1
};

enum Column
{
    ColumnCheck = 0,
    ColumnName = 1,
    ColumnSize = 2,
    ColumnDate = 3,
    ColumnCount = ColumnDate + 1
};

class DataSortedItem : public QStandardItem
{
public:
    DataSortedItem(const QString &text, qulonglong sortValue)
        : QStandardItem(text)
    {
        setData(sortValue, valueRole);
    }

    bool operator<(const QStandardItem &other) const
    {
        return data(valueRole).toULongLong() < other.data(valueRole).toULongLong();
    }
};

class CheckedIconItem : public QStandardItem
{
public:
    CheckedIconItem(const QString &imagePath, const QSize &iconSize)
    {
        setData(Qt::Unchecked, Qt::CheckStateRole);

        QPixmap pixmap(imagePath);
        const int minSide = qMin(pixmap.width(), pixmap.height());
        setIcon(pixmap.copy((pixmap.width() - minSide) / 2, (pixmap.height() - minSide) / 2, minSide, minSide)
                .scaled(iconSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
};

}

RecorderSnapshotsManager::RecorderSnapshotsManager(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RecorderSnapshotsManager)
    , scanner(new RecorderSnapshotsScanner)
    , cleaner(nullptr)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(PageProgress);

    connect(scanner, SIGNAL(scanningFinished(SnapshotDirInfoList)),
            this, SLOT(onScanningFinished(SnapshotDirInfoList)));
    connect(ui->buttonSelectAll, SIGNAL(clicked()), this, SLOT(onButtonSelectAllClicked()));
    connect(ui->buttonBox->button(QDialogButtonBox::Discard), SIGNAL(clicked()), this, SLOT(onButtonCleanUpClicked()));
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

RecorderSnapshotsManager::~RecorderSnapshotsManager()
{
    delete scanner;
    delete ui;
}

void RecorderSnapshotsManager::execFor(const QString &snapshotsDirectory)
{
    scanner->setup(snapshotsDirectory);
    startScanning();
    exec();
}

void RecorderSnapshotsManager::closeEvent(QCloseEvent *event)
{
    abortCleanUp();
    QDialog::closeEvent(event);
}

void RecorderSnapshotsManager::reject()
{
    abortCleanUp();
    QDialog::reject();
}

void RecorderSnapshotsManager::onScanningFinished(SnapshotDirInfoList snapshots)
{
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(ColumnCount);

    QLocale locale = this->locale();
    const QString &dateFormat = locale.dateTimeFormat(QLocale::ShortFormat);
    for (const SnapshotDirInfo &info : snapshots) {
        QStandardItem *nameCol = new QStandardItem(info.name);
        nameCol->setData(info.path, valueRole);
        model->appendRow({
            new CheckedIconItem(info.thumbnail, ui->treeDirectories->iconSize()),
            nameCol,
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
            new DataSortedItem(locale.formattedDataSize(info.size), info.size),
#else
            new DataSortedItem(qlocale_formattedDataSize(info.size), info.size),
#endif
            new DataSortedItem(info.dateTime.toString(dateFormat), info.dateTime.toMSecsSinceEpoch())
        });
    }

    model->setHorizontalHeaderItem(ColumnCheck,
        new QStandardItem(i18nc("Header title for preview thumbnail", "Preview")));
    model->setHorizontalHeaderItem(ColumnName,
        new QStandardItem(i18nc("Header title for directory name column", "Name")));
    model->setHorizontalHeaderItem(ColumnSize,
        new QStandardItem(i18nc("Header title for size of directory column", "Size")));
    model->setHorizontalHeaderItem(ColumnDate,
        new QStandardItem(i18nc("Header title for last modified date/time column", "Last Modified")));

    QAbstractItemModel *oldModel = ui->treeDirectories->model();
    QItemSelectionModel *oldSelectionModel = ui->treeDirectories->selectionModel();
    ui->treeDirectories->setModel(model);
    if (oldModel != nullptr)
        oldModel->deleteLater();
    if (oldSelectionModel != nullptr)
        oldSelectionModel->deleteLater();

    ui->buttonBox->button(QDialogButtonBox::Discard)->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(PageSelection);

     for(int col = 0; col < (ColumnCount - 1); ++col) {
        ui->treeDirectories->resizeColumnToContents(col);
        const int colWidth = ui->treeDirectories->columnWidth(col);
        ui->treeDirectories->setColumnWidth(col, colWidth + defaultColumnMargin);
     }

    connect(ui->treeDirectories->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)), Qt::UniqueConnection);
}

void RecorderSnapshotsManager::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QAbstractItemModel *model = ui->treeDirectories->model();
    for (const QModelIndex &index : selected.indexes())
        model->setData(index.sibling(index.row(), ColumnCheck), Qt::Checked, Qt::CheckStateRole);
    for (const QModelIndex &index : deselected.indexes())
        model->setData(index.sibling(index.row(), ColumnCheck), Qt::Unchecked, Qt::CheckStateRole);

    ui->buttonBox->button(QDialogButtonBox::Discard)->setEnabled(!ui->treeDirectories->selectionModel()->selectedIndexes().isEmpty());

    updateSpaceToBeFreed();
}

void RecorderSnapshotsManager::startScanning()
{
    ui->labelProgress->setText(i18nc("Label title, Scanning for directory, files, etc..", "Scanning..."));
    ui->stackedWidget->setCurrentIndex(PageProgress);
    scanner->start();
}

void RecorderSnapshotsManager::updateSpaceToBeFreed()
{
    const QModelIndexList &indexes = ui->treeDirectories->selectionModel()->selectedRows(ColumnSize);
    qulonglong totalSize = 0;
    QAbstractItemModel *model = ui->treeDirectories->model();
    for (const QModelIndex &index : indexes)
        totalSize += model->data(index, valueRole).toULongLong();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    ui->labelSpace->setText(locale().formattedDataSize(totalSize));
#else
    ui->labelSpace->setText(qlocale_formattedDataSize(totalSize));
#endif
    ui->buttonSelectAll->setText(indexes.size() != model->rowCount() ? i18n("Select All") : i18n("Select None"));
}

void RecorderSnapshotsManager::abortCleanUp()
{
    if (cleaner == nullptr)
        return;

    cleaner->stop();
    cleaner->deleteLater();
    cleaner = nullptr;
}

void RecorderSnapshotsManager::onButtonSelectAllClicked()
{
    const QModelIndexList &selectedIndexes = ui->treeDirectories->selectionModel()->selectedRows(ColumnCheck);
    QAbstractItemModel *model = ui->treeDirectories->model();
    int rowCount = model->rowCount();
    bool selectAll = selectedIndexes.size() != rowCount;
    if (selectAll) {
        ui->treeDirectories->selectAll();
    } else {
        ui->treeDirectories->clearSelection();
    }
}

void RecorderSnapshotsManager::onButtonCleanUpClicked()
{
    const QString confirmation(i18n("The selected recordings will be deleted"
                                    " and you will not be able to export a timelapse for them again"
                                    " (the already exported timelapses will be preserved though)."
                                    "\nDo you wish to continue?"));
    if (QMessageBox::question(this, windowTitle(), confirmation) != QMessageBox::Yes)
        return;

    QStringList directories;
    const QModelIndexList &indexes = ui->treeDirectories->selectionModel()->selectedRows(ColumnName);
    QAbstractItemModel *model = ui->treeDirectories->model();
    for (const QModelIndex &index : indexes)
        directories.append(model->data(index, valueRole).toString());

    ui->labelProgress->setText(i18nc("Label title, Snapshot directory deleting is in progress", "Cleaning up..."));
    ui->stackedWidget->setCurrentIndex(PageProgress);

    Q_ASSERT(cleaner == nullptr);
    cleaner = new RecorderDirectoryCleaner(directories);
    connect(cleaner, SIGNAL(finished()), this, SLOT(onCleanUpFinished()));
    cleaner->start();
}

void RecorderSnapshotsManager::onButtonCancelCleanUpClicked()
{
    abortCleanUp();
    startScanning();
}

void RecorderSnapshotsManager::onCleanUpFinished()
{
    cleaner->deleteLater();
    cleaner = nullptr;

    startScanning();
}
