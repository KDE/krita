/*
 *  SPDX-FileCopyrightText: 2014 Victor Lafon metabolic.ewilan @hotmail.fr
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "dlg_bundle_manager.h"
#include "ui_wdgdlgbundlemanager.h"

#include "resourcemanager.h"
#include "dlg_create_bundle.h"

#include <QListWidget>
#include <QTreeWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>
#include <QInputDialog>
#include <QItemSelectionModel>


#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <KoIcon.h>
#include <KoFileDialog.h>

#include <kis_icon.h>
#include "kis_action.h"
#include <KisResourceStorage.h>
#include <KisResourceServerProvider.h>
#include <KisStorageModel.h>
#include <KisStorageFilterProxyModel.h>
#include <kis_config.h>
#include <KisResourceLocator.h>



DlgBundleManager::ItemDelegate::ItemDelegate(QObject *parent, KisStorageFilterProxyModel* proxy)
    : QStyledItemDelegate(parent)
    , m_bundleManagerProxyModel(proxy)
{

}

QSize DlgBundleManager::ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(100, 30);
}

void DlgBundleManager::ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    QModelIndex sourceIndex = m_bundleManagerProxyModel->mapToSource(index);

    int minMargin = 3;
    int textMargin = 10;


    painter->save();

    // paint background
    QColor bgColor = option.state & QStyle::State_Selected ?
        qApp->palette().color(QPalette::Highlight) :
        qApp->palette().color(QPalette::Base);
    QBrush oldBrush(painter->brush());
    QPen oldPen = painter->pen();
    painter->setBrush(QBrush(bgColor));
    painter->setPen(Qt::NoPen);
    painter->drawRect(option.rect);
    painter->setBrush(oldBrush);
    painter->setPen(oldPen);


    QRect paintRect = kisGrowRect(option.rect, -minMargin);
    int height = paintRect.height();


    // make border around active ones
    bool active = KisStorageModel::instance()->data(sourceIndex, Qt::UserRole + KisStorageModel::Active).toBool();

    QColor borderColor = qApp->palette().color(QPalette::Text);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(borderColor));

    QRect borderRect = kisGrowRect(paintRect, -painter->pen().widthF());
    if (active) {
        painter->drawRect(borderRect);
    }
    painter->setBrush(oldBrush);
    painter->setPen(oldPen);


    // paint the image
    QImage thumbnail = KisStorageModel::instance()->data(sourceIndex, Qt::UserRole + KisStorageModel::Thumbnail).value<QImage>();


    QRect iconRect = paintRect;
    iconRect.setWidth(height);
    painter->drawImage(iconRect, thumbnail);

    QRect nameRect = paintRect;
    nameRect.setX(paintRect.x() + height + textMargin);
    nameRect.setWidth(paintRect.width() - height - textMargin);

    QTextOption textCenterOption;
    textCenterOption.setAlignment(Qt::AlignVCenter);
    QString name = KisStorageModel::instance()->data(sourceIndex, Qt::UserRole + KisStorageModel::DisplayName).toString();
    painter->drawText(nameRect, name, textCenterOption);

    painter->restore();

}


DlgBundleManager::DlgBundleManager(QWidget *parent)
    : KoDialog(parent)
    , m_page(new QWidget())
    , m_ui(new Ui::WdgDlgBundleManager)
{
    setCaption(i18n("Manage Resource Libraries"));
    m_ui->setupUi(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_ui->bnAdd->setIcon(KisIconUtils::loadIcon("list-add"));
    m_ui->bnAdd->setText(i18nc("In bundle manager; press button to import a bundle", "Import"));
    connect(m_ui->bnAdd, SIGNAL(clicked(bool)), SLOT(addBundle()));

    m_ui->bnNew->setIcon(KisIconUtils::loadIcon("document-new"));
    m_ui->bnNew->setText(i18nc("In bundle manager; press button to create a new bundle", "Create"));
    connect(m_ui->bnNew, SIGNAL(clicked(bool)), SLOT(createBundle()));

    m_ui->bnDelete->setIcon(KisIconUtils::loadIcon("edit-delete"));
    m_ui->bnDelete->setText(i18nc("In bundle manager; press button to deactivate the bundle "
                                  "(remove resources from the bundle from the available resources)","Deactivate"));
    connect(m_ui->bnDelete, SIGNAL(clicked(bool)), SLOT(deleteBundle()));

    setButtons(Close);

    m_proxyModel = new KisStorageFilterProxyModel(this);
    m_proxyModel->setSourceModel(KisStorageModel::instance());
    m_proxyModel->setFilter(KisStorageFilterProxyModel::ByStorageType,
                          QStringList()
                          << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::Bundle)
                          << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::Folder));

    m_ui->listView->setModel(m_proxyModel);
    m_ui->listView->setItemDelegate(new ItemDelegate(this, m_proxyModel));

    QItemSelectionModel* selectionModel = m_ui->listView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &DlgBundleManager::currentCellSelectedChanged);
    //connect(m_ui->listView, &QItemSelectionModel::currentChanged, this, &DlgBundleManager::currentCellSelectedChanged);


    connect(KisStorageModel::instance(), &KisStorageModel::modelAboutToBeReset, this, &DlgBundleManager::slotModelAboutToBeReset);
    connect(KisStorageModel::instance(), &KisStorageModel::modelReset, this, &DlgBundleManager::slotModelReset);



}

void DlgBundleManager::addBundle()
{
    KoFileDialog dlg(this, KoFileDialog::OpenFile, i18n("Choose the bundle to import"));
    dlg.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    dlg.setCaption(i18n("Select the bundle"));
    QString filename = dlg.filename();
    if (!filename.isEmpty()) {
        addBundleToActiveResources(filename);
    }
}

void DlgBundleManager::createBundle()
{
    DlgCreateBundle* dlg = new DlgCreateBundle(0, this);
    dlg->exec();
}

void DlgBundleManager::deleteBundle()
{
    QModelIndex idx = m_ui->listView->currentIndex();
    KIS_ASSERT(m_proxyModel);
    if (!idx.isValid()) {
        ENTER_FUNCTION() << "Index is invalid\n";
        return;
    }
    bool active = m_proxyModel->data(idx, Qt::UserRole + KisStorageModel::Active).toBool();
    idx = m_proxyModel->index(idx.row(), 0);
    m_proxyModel->setData(idx, QVariant(!active), Qt::CheckStateRole);
}

void DlgBundleManager::slotModelAboutToBeReset()
{
    ENTER_FUNCTION();
    lastIndex = QPersistentModelIndex(m_proxyModel->mapToSource(m_ui->listView->currentIndex()));
    ENTER_FUNCTION() << ppVar(lastIndex) << ppVar(lastIndex.isValid());
}

void DlgBundleManager::slotModelReset()
{
    ENTER_FUNCTION();
    ENTER_FUNCTION() << ppVar(lastIndex) << ppVar(lastIndex.isValid());
    if (lastIndex.isValid()) {
        ENTER_FUNCTION() << "last index valid!";
        m_ui->listView->setCurrentIndex(m_proxyModel->mapToSource(lastIndex));
    }
    lastIndex = QModelIndex();
}

void DlgBundleManager::currentCellSelectedChanged(QModelIndex current, QModelIndex previous)
{
    Q_UNUSED(previous);

    ENTER_FUNCTION() << "Current cell changed!";
    QModelIndex idx = m_ui->listView->currentIndex();
    KIS_ASSERT(m_proxyModel);
    if (!idx.isValid()) {
        ENTER_FUNCTION() << "Index is invalid\n";
        return;
    }
    bool active = m_proxyModel->data(idx, Qt::UserRole + KisStorageModel::Active).toBool();

    if (active) {
        m_ui->bnDelete->setIcon(KisIconUtils::loadIcon("edit-delete"));
        m_ui->bnDelete->setText(i18nc("In bundle manager; press button to deactivate the bundle "
                                      "(remove resources from the bundle from the available resources)","Deactivate"));
    } else {
        m_ui->bnDelete->setIcon(QIcon());
        m_ui->bnDelete->setText(i18nc("In bundle manager; press button to activate the bundle "
                                      "(add resources from the bundle to the available resources)","Activate"));
    }
    updateBundleInformation(current);
}

void DlgBundleManager::updateBundleInformation(QModelIndex currentInProxy)
{
    QModelIndex idx = m_proxyModel->mapToSource(currentInProxy);

    KisResourceStorageSP storage = KisStorageModel::instance()->storageForIndex(idx);
    KIS_SAFE_ASSERT_RECOVER_RETURN(storage);

    m_ui->lblAuthor->setText(storage->metaData(KisResourceStorage::s_meta_author).toString());
    QString date = storage->metaData(KisResourceStorage::s_meta_creation_date).toString();
    date = QDateTime::fromSecsSinceEpoch(date.toInt()).toString();
    m_ui->lblCreated->setText(date);
    m_ui->lblDescription->setPlainText(storage->metaData(KisResourceStorage::s_meta_description).toString());
    m_ui->lblName->setText(storage->name());
    m_ui->lblType->setText(KisResourceStorage::storageTypeToString(storage->type()));
    m_ui->lblEmail->setText(storage->metaData(KisResourceStorage::s_meta_email).toString());
    m_ui->lblLicense->setText(storage->metaData(KisResourceStorage::s_meta_license).toString());

    QImage thumbnail = KisStorageModel::instance()->data(idx, Qt::UserRole + KisStorageModel::Thumbnail).value<QImage>();
    m_ui->lblPreview->setPixmap(QPixmap::fromImage(thumbnail));

}

QString createNewBundlePath(QString resourceFolder, QString filename)
{
    return resourceFolder + '/' + filename;
}

void DlgBundleManager::addBundleToActiveResources(QString filename)
{
    Q_UNUSED(filename);
    // 1. Copy the bundle to the resource folder
    QFileInfo oldFileInfo(filename);

    KisConfig cfg(true);
    QString newDir = cfg.readEntry<QString>(KisResourceLocator::resourceLocationKey,
                                            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QString newName = oldFileInfo.fileName();
    QString newLocation = createNewBundlePath(newDir, newName);

    QFileInfo newFileInfo(newLocation);
    if (newFileInfo.exists()) {
        bool done = false;
        int i = 0;
        do {
            // ask for new filename
            bool ok;
            newName = QInputDialog::getText(this, i18n("New name for the bundle"), i18n("The old filename %1 is taken.\nNew name:", newName),
                                                    QLineEdit::Normal, newName, &ok);
            newLocation = createNewBundlePath(newDir, newName);
            newFileInfo.setFile(newLocation);
            done = !newFileInfo.exists();
            i++;
        } while (!done);
    }

    QFile::copy(filename, newLocation);

    // 2. Add the bundle as a storage/update database
    KisResourceStorageSP storage = QSharedPointer<KisResourceStorage>::create(QFileInfo(newLocation).fileName());
    KIS_ASSERT(!storage.isNull());
    KisResourceLocator::instance()->addStorage(QFileInfo(newLocation).fileName(), storage);
}

