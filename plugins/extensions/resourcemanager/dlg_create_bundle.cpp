/*
 *  SPDX-FileCopyrightText: 2014 Victor Lafon metabolic.ewilan @hotmail.fr
 *  SPDX-FileCopyrightText: 2020 Agata Cacko cacko.azh @gmail.com
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <QStack>

#include <KisImportExportManager.h>
#include <KoDocumentInfo.h>
#include <KoFileDialog.h>
#include <kis_icon.h>
#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoResource.h>

#include <kis_workspace_resource.h>
#include <brushengine/kis_paintop_preset.h>
#include <dlg_embed_tags.h>
#include <KisGlobalResourcesInterface.h>

#include <kis_config.h>

#define ICON_SIZE 48

DlgCreateBundle::DlgCreateBundle(KoResourceBundleSP bundle, QWidget *parent)
    : KoDialog(parent)
    , m_ui(new Ui::WdgDlgCreateBundle)
    , m_bundle(bundle)
{
    m_page = new QWidget();
    m_ui->setupUi(m_page);
    setMainWidget(m_page);
    //setFixedSize(m_page->sizeHint());
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setButtonText(Ok, i18n("Save"));

    connect(m_ui->bnSelectSaveLocation, SIGNAL(clicked()), SLOT(selectSaveLocation()));

    KoDocumentInfo info;
    info.updateParameters();

    if (bundle) {

        setCaption(i18n("Edit Resource Bundle"));
#if 0
        /*
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
                        m_selectedGradients << res->filename();
                    }
                }

            }
            else if (resType  == ResourceType::Patterns) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedPatterns << res->filename();
                    }
                }

            }
            else if (resType  == ResourceType::Brushes) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedBrushes << res->filename();
                    }
                }

            }
            else if (resType  == ResourceType::Palettes) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedPalettes << res->filename();
                    }
                }

            }
            else if (resType  == ResourceType::Workspaces) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedWorkspaces << res->filename();
                    }
                }

            }
            else if (resType  == ResourceType::PaintOpPresets) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedPresets << res->filename();
                    }
                }
            }
            else if (resType  == ResourceType::GamutMasks) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedGamutMasks << res->filename();
                    }
                }
            }
#if defined HAVE_SEEXPR
            else if (resType == ResourceTypes::SeExprScripts) {
                Q_FOREACH (const KoResourceSP res, bundle->resources(resType)) {
                    if (res) {
                        m_selectedSeExprScripts << res->filename();
                    }
                }
            }
#endif
        }
        */
#endif
    }
    else {

        setCaption(i18n("Create Resource Bundle"));

        KisConfig cfg(true);

        m_ui->editAuthor->setText(cfg.readEntry<QString>("BundleAuthorName", info.authorInfo("creator")));
        m_ui->editEmail->setText(cfg.readEntry<QString>("BundleAuthorEmail", info.authorInfo("email")));
        m_ui->editWebsite->setText(cfg.readEntry<QString>("BundleWebsite", "http://"));
        m_ui->editLicense->setText(cfg.readEntry<QString>("BundleLicense", "CC-BY-SA"));
        m_ui->editBundleName->setText(cfg.readEntry<QString>("BundleName", "New Bundle"));
        m_ui->editDescription->document()->setPlainText(cfg.readEntry<QString>("BundleDescription", "New Bundle"));
        m_previewImage = cfg.readEntry<QString>("BundleImage", "");
        if (!m_previewImage.isEmpty()) {
            QImage img(m_previewImage);
            img = img.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_ui->lblPreview->setPixmap(QPixmap::fromImage(img));
        }

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
#if defined HAVE_SEEXPR
    m_ui->cmbResourceTypes->addItem(i18n("SeExpr Scripts"), ResourceType::SeExprScripts);
#endif
    m_ui->cmbResourceTypes->addItem(i18n("Patterns"), ResourceType::Patterns);
    m_ui->cmbResourceTypes->addItem(i18n("Palettes"), ResourceType::Palettes);
    m_ui->cmbResourceTypes->addItem(i18n("Workspaces"), ResourceType::Workspaces);
    connect(m_ui->cmbResourceTypes, SIGNAL(activated(int)), SLOT(resourceTypeSelected(int)));

    m_ui->tableAvailable->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_ui->tableAvailable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_ui->tableSelected->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_ui->tableSelected->setSelectionMode(QAbstractItemView::ExtendedSelection);

    KisTagModel* model = new KisTagModel(ResourceType::PaintOpPresets);
    QList<KisTagSP> tags;
    QList<KisTagSP> tagsSec;
    for(int i = 0; i < model->rowCount(); i++) {
        QModelIndex idx = model->index(i, 0);
        KisTagSP tag = model->tagForIndex(idx);
        if (i%2 && tags.count() < 2) {
            tags << tag;
        } else /* if (tagsSec.count() < 2) */ {
            tagsSec << tag;
        }
    }
    //m_ui->tagsTempCombo->setTagList(true, tagsSec, tags);
    m_tagSelectionWidgetController = new KisWdgTagSelectionControllerBundleTags(m_ui->tagsTempCombo, true);
    m_tagSelectionWidgetController->updateView();

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

QVector<KisTagSP> DlgCreateBundle::getTagsForEmbeddingInResource(QVector<KisTagSP> resourceTags, QList<int> selectedTagIds) const
{
    QVector<KisTagSP> tagsToEmbed;
    Q_FOREACH(KisTagSP tag, resourceTags) {
        if (selectedTagIds.contains(tag->id())) {
            tagsToEmbed << tag;
            ENTER_FUNCTION() << "Adding " << tag->id() << tag->name() << "to embedded tags";
        }
    }
    return tagsToEmbed;
}

void DlgCreateBundle::putResourcesInTheBundle() const
{
    KisResourceModel emptyModel("");
    QStack<int> allResourcesIds;
    Q_FOREACH(int id, m_selectedResourcesIds) {
        allResourcesIds << id;
    }

    QList<int> selectedTagIds = m_tagSelectionWidgetController->getSelectedTagIds();

    // note: if there are repetitions, it's fine; the bundle will filter them out
    while(!allResourcesIds.isEmpty()) {
        int id = allResourcesIds.takeFirst();
        KoResourceSP res = emptyModel.resourceForId(id);
        if (!res) {
            warnKrita << "No resource for id " << id;
            continue;
        }
        KisResourceModel resModel(res->resourceType().first);
        QVector<KisTagSP> tags = getTagsForEmbeddingInResource(resModel.tagsForResource(id), selectedTagIds);
        m_bundle->addResource(res->resourceType().first, res->filename(), tags, res->md5());

        QList<KoResourceSP> linkedResources = res->linkedResources(KisGlobalResourcesInterface::instance());
        if (!linkedResources.isEmpty()) {
            Q_FOREACH(KoResourceSP resource, linkedResources) {
                if (!allResourcesIds.contains(resource->resourceId())) {
                    allResourcesIds.append(resource->resourceId());
                }
            }
        }
    }
}

void DlgCreateBundle::accept()
{
    QString name = bundleName();
    QString filename = m_ui->lblSaveLocation->text() + "/" + name + ".bundle";

    if (name.isEmpty()) {
        m_ui->editBundleName->setStyleSheet(QString(" border: 1px solid red"));
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The resource bundle name cannot be empty."));
        return;
    }
    else {
        QFileInfo fileInfo(filename);

        if (fileInfo.exists() && !m_bundle) {
            m_ui->editBundleName->setStyleSheet("border: 1px solid red");

            QMessageBox msgBox;
            msgBox.setText(i18nc("In a dialog asking whether to overwrite a bundle (resource pack)", "A bundle with this name already exists."));
            msgBox.setInformativeText(i18nc("In a dialog regarding overwriting a bundle (resource pack)", "Do you want to overwrite the existing bundle?"));
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            int ret = msgBox.exec();
            if (ret == QMessageBox::Cancel) {
                return;
            }
        }

        if (!m_bundle) {
            saveToConfiguration(false);

            m_bundle.reset(new KoResourceBundle(filename));
            putResourcesInTheBundle();
            m_bundle->save();

        } else {
            KIS_SAFE_ASSERT_RECOVER(!m_bundle) { warnKrita << "Updating a bundle is not implemented yet"; };
        }
        KoDialog::accept();
    }
}

void DlgCreateBundle::saveToConfiguration(bool full)
{
    KisConfig cfg(false);
    if (full) {
        cfg.writeEntry<QString>("BundleName", bundleName());
        cfg.writeEntry<QString>("BundleDescription", description());
        cfg.writeEntry<QString>("BundleImage", previewImage());
    } else {
        cfg.writeEntry<QString>("BundleName", "");
        cfg.writeEntry<QString>("BundleDescription", "");
        cfg.writeEntry<QString>("BundleImage", "");
    }
    cfg.writeEntry<QString>("BundleExportLocation", saveLocation());
    cfg.writeEntry<QString>("BundleAuthorName", authorName());
    cfg.writeEntry<QString>("BundleAuthorEmail", email());
    cfg.writeEntry<QString>("BundleWebsite", website());
    cfg.writeEntry<QString>("BundleLicense", license());
}

void DlgCreateBundle::reject()
{
    saveToConfiguration(true);
    KoDialog::reject();
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
        m_selectedResourcesIds.append(item->data(Qt::UserRole).toInt());
    }

    m_ui->tableAvailable->setCurrentRow(row);
}

void DlgCreateBundle::removeSelected()
{
    int row = m_ui->tableSelected->currentRow();

    Q_FOREACH (QListWidgetItem *item, m_ui->tableSelected->selectedItems()) {
        m_ui->tableAvailable->addItem(m_ui->tableSelected->takeItem(m_ui->tableSelected->row(item)));
        m_selectedResourcesIds.removeAll(item->data(Qt::UserRole).toInt());
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

    m_tagSelectionWidgetController->setResourceType(resourceType);

    m_ui->tableAvailable->clear();
    m_ui->tableSelected->clear();

    QString standarizedResourceType = (resourceType == "presets" ? ResourceType::PaintOpPresets : resourceType);

    KisResourceModel model(standarizedResourceType);
    for (int i = 0; i < model.rowCount(); i++) {
        QModelIndex idx = model.index(i, 0);
        QString filename = model.data(idx, Qt::UserRole + KisAbstractResourceModel::Filename).toString();
        int id = model.data(idx, Qt::UserRole + KisAbstractResourceModel::Id).toInt();

        if (resourceType == ResourceType::Gradients) {
            if (filename == "Foreground to Transparent" || filename == "Foreground to Background") {
                continue;
            }
        }

        QImage image = (model.data(idx, Qt::UserRole + KisAbstractResourceModel::Thumbnail)).value<QImage>();
        QString name = model.data(idx, Qt::UserRole + KisAbstractResourceModel::Name).toString();

        // Function imageToIcon(QImage()) returns a square white pixmap and a warning "QImage::scaled: Image is a null image"
        //  while QPixmap() returns an empty pixmap.
        // The difference between them is relevant in case of Workspaces which has no images.
        // Using QPixmap() makes them appear in a dense list without icons, while imageToIcon(QImage())
        //  would give a list with big white rectangles and names of the workspaces.
        QListWidgetItem *item = new QListWidgetItem(image.isNull() ? QPixmap() : imageToIcon(image), name);
        item->setData(Qt::UserRole, id);

        if (m_selectedResourcesIds.contains(id)) {
            m_ui->tableSelected->addItem(item);
        }
        else {
            m_ui->tableAvailable->addItem(item);
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



