/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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
#include "dlg_create_bundle.h"

#include "ui_wdgdlgcreatebundle.h"

#include <QProcessEnvironment>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>
#include <QGridLayout>
#include <QTableWidget>
#include <QPainter>

#include <KisImportExportManager.h>
#include <KoDocumentInfo.h>
#include <KoFileDialog.h>
#include <kis_icon.h>
#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <KisResourceServerProvider.h>
#include <kis_workspace_resource.h>
#include <brushengine/kis_paintop_preset.h>
#include <KisBrushServerProvider.h>

#include <kis_config.h>

#include "KisResourceBundle.h"

#define ICON_SIZE 48

DlgCreateBundle::DlgCreateBundle(KisResourceBundleSP bundle, QWidget *parent)
    : KoDialog(parent)
    , m_ui(new Ui::WdgDlgCreateBundle)
    , m_bundle(bundle)
{
    m_page = new QWidget();
    m_ui->setupUi(m_page);
    setMainWidget(m_page);
    setFixedSize(m_page->sizeHint());
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setButtonText(Ok, i18n("Save"));

    connect(m_ui->bnSelectSaveLocation, SIGNAL(clicked()), SLOT(selectSaveLocation()));

    KoDocumentInfo info;
    info.updateParameters();

    if (bundle) {

        setCaption(i18n("Edit Resource Bundle"));

        m_ui->lblSaveLocation->setText(QFileInfo(bundle->filename()).absolutePath());
        m_ui->editBundleName->setText(bundle->name());
        m_ui->editAuthor->setText(bundle->getMeta("author"));
        m_ui->editEmail->setText(bundle->getMeta("email"));
        m_ui->editLicense->setText(bundle->getMeta("license"));
        m_ui->editWebsite->setText(bundle->getMeta("website"));
        m_ui->editDescription->document()->setPlainText(bundle->getMeta("description"));
        m_ui->lblPreview->setPixmap(QPixmap::fromImage(bundle->image().scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

        Q_FOREACH (const QString & resType, bundle->resourceTypes()) {
            if (resType == ResourceType::Gradients) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedGradients << res->shortFilename();
                    }
                }

            }
            else if (resType  == ResourceType::Patterns) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedPatterns << res->shortFilename();
                    }
                }

            }
            else if (resType  == ResourceType::Brushes) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedBrushes << res->shortFilename();
                    }
                }

            }
            else if (resType  == ResourceType::Palettes) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedPalettes << res->shortFilename();
                    }
                }

            }
            else if (resType  == ResourceType::Workspaces) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedWorkspaces << res->shortFilename();
                    }
                }

            }
            else if (resType  == ResourceType::PaintOpPresets) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedPresets << res->shortFilename();
                    }
                }
            }
            else if (resType  == ResourceType::GamutMasks) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedGamutMasks << res->shortFilename();
                    }
                }
            }
        }
    }
    else {

        setCaption(i18n("Create Resource Bundle"));

        KisConfig cfg(true);

        m_ui->editAuthor->setText(cfg.readEntry<QString>("BundleAuthorName", info.authorInfo("creator")));
        m_ui->editEmail->setText(cfg.readEntry<QString>("BundleAuthorEmail", info.authorInfo("email")));
        m_ui->editWebsite->setText(cfg.readEntry<QString>("BundleWebsite", "http://"));
        m_ui->editLicense->setText(cfg.readEntry<QString>("BundleLicense", "CC-BY-SA"));
        m_ui->lblSaveLocation->setText(cfg.readEntry<QString>("BundleExportLocation", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    }

    m_ui->bnAdd->setIcon(KisIconUtils::loadIcon("arrow-right"));
    connect(m_ui->bnAdd, SIGNAL(clicked()), SLOT(addSelected()));

    m_ui->bnRemove->setIcon(KisIconUtils::loadIcon("arrow-left"));
    connect(m_ui->bnRemove, SIGNAL(clicked()), SLOT(removeSelected()));

    m_ui->cmbResourceTypes->addItem(i18n("Brushes"), ResourceType::Brushes);
    m_ui->cmbResourceTypes->addItem(i18n("Brush Presets"), ResourceType::PaintOpPresets);
    m_ui->cmbResourceTypes->addItem(i18n("Gradients"), ResourceType::Gradients);
    m_ui->cmbResourceTypes->addItem(i18n("Gamut Masks"), ResourceType::GamutMasks);
    m_ui->cmbResourceTypes->addItem(i18n("Patterns"), ResourceType::Patterns);
    m_ui->cmbResourceTypes->addItem(i18n("Palettes"), ResourceType::Palettes);
    m_ui->cmbResourceTypes->addItem(i18n("Workspaces"), ResourceType::Workspaces);
    connect(m_ui->cmbResourceTypes, SIGNAL(activated(int)), SLOT(resourceTypeSelected(int)));

    m_ui->tableAvailable->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_ui->tableAvailable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_ui->tableSelected->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_ui->tableSelected->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(m_ui->bnGetPreview, SIGNAL(clicked()), SLOT(getPreviewImage()));

    resourceTypeSelected(0);
}

DlgCreateBundle::~DlgCreateBundle()
{
    delete m_ui;
}

QString DlgCreateBundle::bundleName() const
{
    return m_ui->editBundleName->text().replace(" ", "_");
}

QString DlgCreateBundle::authorName() const
{
    return m_ui->editAuthor->text();
}

QString DlgCreateBundle::email() const
{
    return m_ui->editEmail->text();
}

QString DlgCreateBundle::website() const
{
    return m_ui->editWebsite->text();
}

QString DlgCreateBundle::license() const
{
    return m_ui->editLicense->text();
}

QString DlgCreateBundle::description() const
{
    return m_ui->editDescription->document()->toPlainText();
}

QString DlgCreateBundle::saveLocation() const
{
    return m_ui->lblSaveLocation->text();
}

QString DlgCreateBundle::previewImage() const
{
    return m_previewImage;
}

void DlgCreateBundle::accept()
{
    QString name = m_ui->editBundleName->text().remove(" ");

    if (name.isEmpty()) {
        m_ui->editBundleName->setStyleSheet(QString(" border: 1px solid red"));
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The resource bundle name cannot be empty."));
        return;
    }
    else {
        QFileInfo fileInfo(m_ui->lblSaveLocation->text() + "/" + name + ".bundle");

        if (fileInfo.exists() && !m_bundle) {
            m_ui->editBundleName->setStyleSheet("border: 1px solid red");
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("A bundle with this name already exists."));
            return;
        }
        else {
            if (!m_bundle) {
                KisConfig cfg(false);
                cfg.writeEntry<QString>("BunleExportLocation", m_ui->lblSaveLocation->text());
                cfg.writeEntry<QString>("BundleAuthorName", m_ui->editAuthor->text());
                cfg.writeEntry<QString>("BundleAuthorEmail", m_ui->editEmail->text());
                cfg.writeEntry<QString>("BundleWebsite", m_ui->editWebsite->text());
                cfg.writeEntry<QString>("BundleLicense", m_ui->editLicense->text());
            }
            KoDialog::accept();
        }
    }
}

void DlgCreateBundle::selectSaveLocation()
{
    KoFileDialog dialog(this, KoFileDialog::OpenDirectory, "resourcebundlesavelocation");
    dialog.setDefaultDir(m_ui->lblSaveLocation->text());
    dialog.setCaption(i18n("Select a directory to save the bundle"));
    QString location = dialog.filename();
    m_ui->lblSaveLocation->setText(location);
}

void DlgCreateBundle::addSelected()
{
    int row = m_ui->tableAvailable->currentRow();

    Q_FOREACH (QListWidgetItem *item, m_ui->tableAvailable->selectedItems()) {
        m_ui->tableSelected->addItem(m_ui->tableAvailable->takeItem(m_ui->tableAvailable->row(item)));
        QString resourceType = m_ui->cmbResourceTypes->itemData(m_ui->cmbResourceTypes->currentIndex()).toString();
        if (resourceType == ResourceType::Brushes) {
            m_selectedBrushes.append(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == "presets") {
            m_selectedPresets.append(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == ResourceType::Gradients) {
            m_selectedGradients.append(item->data(Qt::UserRole).toString());

        }
        else if (resourceType == ResourceType::Patterns) {
            m_selectedPatterns.append(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == ResourceType::Palettes) {
            m_selectedPalettes.append(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == ResourceType::Workspaces) {
            m_selectedWorkspaces.append(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == ResourceType::GamutMasks) {
            m_selectedGamutMasks.append(item->data(Qt::UserRole).toString());
        }
    }

    m_ui->tableAvailable->setCurrentRow(row);
}

void DlgCreateBundle::removeSelected()
{
    int row = m_ui->tableSelected->currentRow();

    Q_FOREACH (QListWidgetItem *item, m_ui->tableSelected->selectedItems()) {
        m_ui->tableAvailable->addItem(m_ui->tableSelected->takeItem(m_ui->tableSelected->row(item)));
        QString resourceType = m_ui->cmbResourceTypes->itemData(m_ui->cmbResourceTypes->currentIndex()).toString();
        if (resourceType == ResourceType::Brushes) {
            m_selectedBrushes.removeAll(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == "presets") {
            m_selectedPresets.removeAll(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == ResourceType::Gradients) {
            m_selectedGradients.removeAll(item->data(Qt::UserRole).toString());

        }
        else if (resourceType == ResourceType::Patterns) {
            m_selectedPatterns.removeAll(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == ResourceType::Palettes) {
            m_selectedPalettes.removeAll(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == ResourceType::Workspaces) {
            m_selectedWorkspaces.removeAll(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == ResourceType::GamutMasks) {
            m_selectedGamutMasks.removeAll(item->data(Qt::UserRole).toString());
        }
    }

    m_ui->tableSelected->setCurrentRow(row);
}

QPixmap imageToIcon(const QImage &img) {
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    pixmap.fill();
    QImage scaled = img.scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    int x = (ICON_SIZE - scaled.width()) / 2;
    int y = (ICON_SIZE - scaled.height()) / 2;
    QPainter gc(&pixmap);
    gc.drawImage(x, y, scaled);
    gc.end();
    return pixmap;
}

void DlgCreateBundle::resourceTypeSelected(int idx)
{
    QString resourceType = m_ui->cmbResourceTypes->itemData(idx).toString();

    m_ui->tableAvailable->clear();
    m_ui->tableSelected->clear();

    if (resourceType == ResourceType::Brushes) {
        KoResourceServer<KisBrush> *server = KisBrushServerProvider::instance()->brushServer();
        Q_FOREACH (KisBrushSP res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedBrushes.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
    else if (resourceType == "presets") {
        KisPaintOpPresetResourceServer* server = KisResourceServerProvider::instance()->paintOpPresetServer();
        Q_FOREACH (KisPaintOpPresetSP res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedPresets.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
    else if (resourceType == ResourceType::Gradients) {
        KoResourceServer<KoAbstractGradient>* server = KoResourceServerProvider::instance()->gradientServer();
        Q_FOREACH (KoResourceSP res, server->resources()) {
            if (res->filename()!="Foreground to Transparent" && res->filename()!="Foreground to Background") {
            //technically we should read from the file-name whether or not the file can be opened, but this works for now. The problem is making sure that bundle-resource know where they are stored.//
            //dbgKrita<<res->filename();
                QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
                item->setData(Qt::UserRole, res->shortFilename());

                if (m_selectedGradients.contains(res->shortFilename())) {
                    m_ui->tableSelected->addItem(item);
                }
                else {
                    m_ui->tableAvailable->addItem(item);
                }
            }
        }
    }
    else if (resourceType == ResourceType::Patterns) {
        KoResourceServer<KoPattern>* server = KoResourceServerProvider::instance()->patternServer();
        Q_FOREACH (KoResourceSP res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedPatterns.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
    else if (resourceType == ResourceType::Palettes) {
        KoResourceServer<KoColorSet>* server = KoResourceServerProvider::instance()->paletteServer();
        Q_FOREACH (KoResourceSP res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedPalettes.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
    else if (resourceType == ResourceType::Workspaces) {
        KoResourceServer<KisWorkspaceResource>* server = KisResourceServerProvider::instance()->workspaceServer();
        Q_FOREACH (KoResourceSP res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedWorkspaces.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
    else if (resourceType == ResourceType::GamutMasks) {
        KoResourceServer<KoGamutMask>* server = KoResourceServerProvider::instance()->gamutMaskServer();
        Q_FOREACH (KoResourceSP res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedGamutMasks.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }

}

void DlgCreateBundle::getPreviewImage()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, "BundlePreviewImage");
    dialog.setCaption(i18n("Select file to use as bundle icon"));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
    m_previewImage = dialog.filename();
    QImage img(m_previewImage);
    img = img.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_ui->lblPreview->setPixmap(QPixmap::fromImage(img));
}



