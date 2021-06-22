/*
 *  SPDX-FileCopyrightText: 2014 Victor Lafon metabolic.ewilan @hotmail.fr
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "dlg_bundle_manager.h"

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

#include <KisMainWindow.h>
#include <KisPart.h>

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

    QRect paintRect = kisGrowRect(option.rect, -minMargin);
    int height = paintRect.height();

    // make border around active ones
    bool active = KisStorageModel::instance()->data(sourceIndex, Qt::UserRole + KisStorageModel::Active).toBool();

    if (active) {
        QColor borderColor = option.state & QStyle::State_Selected ?
                    qApp->palette().color(QPalette::HighlightedText) :
                    qApp->palette().color(QPalette::Text);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(borderColor));

        QRect borderRect = kisGrowRect(paintRect, -painter->pen().widthF());

        painter->drawRect(borderRect);

        painter->setBrush(oldBrush);
        painter->setPen(oldPen);

    }

    // paint the image
    QImage thumbnail = KisStorageModel::instance()->data(sourceIndex, Qt::UserRole + KisStorageModel::Thumbnail).value<QImage>();

    QRect iconRect = paintRect;
    iconRect.setWidth(height);
    painter->drawImage(iconRect, thumbnail);

    QRect nameRect = paintRect;
    nameRect.setX(paintRect.x() + height + textMargin);
    nameRect.setWidth(paintRect.width() - height - 1.5*textMargin); // should be 2, but with 1.5 it's more clear that it's cropped when it is

    QColor textColor = option.state & QStyle::State_Selected ?
                qApp->palette().color(QPalette::HighlightedText) :
                qApp->palette().color(QPalette::Text);
    painter->setPen(QPen(textColor));

    QTextOption textCenterOption;
    textCenterOption.setAlignment(Qt::AlignVCenter);
    textCenterOption.setWrapMode(QTextOption::NoWrap);
    QString name = KisStorageModel::instance()->data(sourceIndex, Qt::UserRole + KisStorageModel::DisplayName).toString();
    painter->drawText(nameRect, name, textCenterOption);

    painter->restore();

}


DlgBundleManager::DlgBundleManager(QWidget *parent)
    : KoDialog(parent)
{
    setCaption(i18n("Manage Resource Libraries"));

    m_ui = new WdgDlgBundleManager(this);
    Q_CHECK_PTR(m_ui);
    setMainWidget(m_ui);

    m_ui->bnAdd->setIcon(KisIconUtils::loadIcon("list-add"));
    m_ui->bnAdd->setText(i18nc("In bundle manager; press button to import a resource library", "Import"));
    connect(m_ui->bnAdd, SIGNAL(clicked(bool)), SLOT(addBundle()));

    m_ui->bnToggle->setIcon(KisIconUtils::loadIcon("edit-delete"));
    m_ui->bnToggle->setText(i18nc("In bundle manager; press button to deactivate the resource library"
                                  "(remove resources from the resource library from the available resources)", "Deactivate"));
    connect(m_ui->bnToggle, SIGNAL(clicked(bool)), SLOT(toggleBundle()));

    m_ui->bnNew->setIcon(KisIconUtils::loadIcon("document-new"));
    m_ui->bnNew->setText(i18nc("In bundle manager; press button to create a new bundle", "Create Bundle"));
    connect(m_ui->bnNew, SIGNAL(clicked(bool)), SLOT(createBundle()));


    setButtons(Close);

    m_proxyModel = new KisStorageFilterProxyModel(this);
    m_proxyModel->setSourceModel(KisStorageModel::instance());
    m_proxyModel->setFilter(KisStorageFilterProxyModel::ByStorageType,
                            QStringList()
                            << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::Bundle)
                            << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::AdobeBrushLibrary)
                            << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::AdobeStyleLibrary));

    m_ui->listView->setModel(m_proxyModel);
    m_ui->listView->setItemDelegate(new ItemDelegate(this, m_proxyModel));

    QItemSelectionModel* selectionModel = m_ui->listView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &DlgBundleManager::currentCellSelectedChanged);
    //connect(m_ui->listView, &QItemSelectionModel::currentChanged, this, &DlgBundleManager::currentCellSelectedChanged);

    connect(KisStorageModel::instance(), &KisStorageModel::modelAboutToBeReset, this, &DlgBundleManager::slotModelAboutToBeReset);
    connect(KisStorageModel::instance(), &KisStorageModel::modelReset, this, &DlgBundleManager::slotModelReset);

    updateToggleButton(m_proxyModel->data(m_ui->listView->currentIndex(), Qt::UserRole + KisStorageModel::Active).toBool());
}

void DlgBundleManager::done(int res)
{
    KisMainWindow *mw = KisPart::instance()->currentMainwindow();
    if (mw) {
        QString warning;
        if (!mw->checkActiveBundlesAvailable()) {
            warning = i18n("You don't have any resource bundles enabled.");
        }

        if (!mw->checkPaintOpAvailable()) {
            warning += i18n("\nThere are no brush presets available. Please enable a bundle that has presets before continuing.\nIf there are no bundles, please import a bundle before continuing.");
            QMessageBox::critical(this, i18nc("@title:window", "Krita"), warning);
            return;
        }

        if (!mw->checkActiveBundlesAvailable()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), warning + i18n("\nOnly your local resources are available."));
        }
    }
    KoDialog::done(res);
}

void DlgBundleManager::addBundle()
{
    KoFileDialog dlg(this, KoFileDialog::OpenFile, i18n("Choose the resource library to import"));
    dlg.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    dlg.setMimeTypeFilters(QStringList() << "application/x-krita-bundle"
                                         << "application/x-krita-bundle"
                                         << "image/x-adobe-brushlibrary"
                                         << "application/x-photoshop-style-library");
    dlg.setCaption(i18n("Select the bundle"));
    QString filename = dlg.filename();
    if (!filename.isEmpty()) {
        // 1. Copy the bundle to the resource folder
        QFileInfo oldFileInfo(filename);

        KisConfig cfg(true);
        QString newDir = cfg.readEntry<QString>(KisResourceLocator::resourceLocationKey,
                                                QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        QString newName = oldFileInfo.fileName();
        QString newLocation = newDir + '/' + newName;

        QFileInfo newFileInfo(newLocation);
        if (newFileInfo.exists()) {
            if (QMessageBox::warning(this, i18nc("@ttile:window", "Warning"), i18n("There is already a bundle with this name installed. Do you want to overwrite it?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel) {
                return;
            }
            else {
                QFile::remove(newLocation);
            }
        }
        QFile::copy(filename, newLocation);

        // 2. Add the bundle as a storage/update database
        KisResourceStorageSP storage = QSharedPointer<KisResourceStorage>::create(newLocation);
        KIS_ASSERT(!storage.isNull());
        if (!KisResourceLocator::instance()->addStorage(newLocation, storage)) {
            qWarning() << "Could not add bundle to the storages" << newLocation;
        }
    }
}

void DlgBundleManager::createBundle()
{
    DlgCreateBundle* dlg = new DlgCreateBundle(0, this);
    dlg->exec();
}

void DlgBundleManager::toggleBundle()
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

    currentCellSelectedChanged(idx, idx);

    KisMainWindow *mw = KisPart::instance()->currentMainwindow();
    if (mw) {
        QString warning;
        if (!mw->checkActiveBundlesAvailable()) {
            warning = i18n("You don't have any resource bundles enabled.");
        }

        if (!mw->checkPaintOpAvailable()) {
            button(KoDialog::Close)->setEnabled(false);

            warning += i18n("\nThere are no brush presets available. Please enable a bundle that has presets before continuing.\nIf there are no bundles, please import a bundle before continuing.");
            QMessageBox::critical(this, i18nc("@title:window", "Krita"), warning);
            return;
        }

        if (!mw->checkActiveBundlesAvailable()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), warning + i18n("\nOnly your local resources are available."));
        }
    }
    button(KoDialog::Close)->setEnabled(true);
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
    KIS_ASSERT(m_proxyModel);
    if (!current.isValid()) {
        ENTER_FUNCTION() << "Index is invalid\n";
        return;
    }
    bool active = m_proxyModel->data(current, Qt::UserRole + KisStorageModel::Active).toBool();
    updateToggleButton(active);
    updateBundleInformation(current);
}

void DlgBundleManager::updateToggleButton(bool active)
{
    if (active) {
        m_ui->bnToggle->setIcon(KisIconUtils::loadIcon("edit-delete"));
        m_ui->bnToggle->setText(i18nc("In bundle manager; press button to deactivate the bundle "
                                      "(remove resources from the bundle from the available resources)","Deactivate"));
    } else {
        m_ui->bnToggle->setIcon(QIcon());
        m_ui->bnToggle->setText(i18nc("In bundle manager; press button to activate the bundle "
                                      "(add resources from the bundle to the available resources)","Activate"));
    }
}

void DlgBundleManager::updateBundleInformation(QModelIndex idx)
{
    KisResourceStorageSP storage = m_proxyModel->storageForIndex(idx);
    KIS_SAFE_ASSERT_RECOVER_RETURN(storage);

    m_ui->detailsPanel->hide();
    m_ui->lblDescription->hide();

    m_ui->BundleSelectedGroupBox->setTitle(storage->name());

    if (storage->type() == KisResourceStorage::StorageType::Bundle) {

        m_ui->detailsPanel->show();
        m_ui->lblDescription->show();

        m_ui->lblAuthor->setText(storage->metaData(KisResourceStorage::s_meta_author).toString());
        QString date = storage->metaData(KisResourceStorage::s_meta_creation_date).toString();
        m_ui->lblCreated->setText(date);
        QString date2 = storage->metaData(KisResourceStorage::s_meta_dc_date).toString();
        m_ui->lblUpdated->setText(date2);

        m_ui->lblDescription->setPlainText(storage->metaData(KisResourceStorage::s_meta_description).toString());
        m_ui->lblEmail->setText(storage->metaData(KisResourceStorage::s_meta_email).toString());
        m_ui->lblLicense->setText(storage->metaData(KisResourceStorage::s_meta_license).toString());
        m_ui->lblWebsite->setText(storage->metaData(KisResourceStorage::s_meta_website).toString());


    }

    QImage thumbnail = KisStorageModel::instance()->data(m_proxyModel->mapToSource(idx), Qt::UserRole + KisStorageModel::Thumbnail).value<QImage>();
    m_ui->lblPreview->setPixmap(QPixmap::fromImage(thumbnail));
    m_ui->lblType->setText(KisResourceStorage::storageTypeToString(storage->type()));
}


