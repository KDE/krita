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
#include <kstandardguiitem.h>

#include <kis_workspace_resource.h>
#include <brushengine/kis_paintop_preset.h>
#include <dlg_embed_tags.h>
#include <KisGlobalResourcesInterface.h>
#include <KisResourceTypeModel.h>

#include <wdgtagselection.h>

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
    setFixedSize(m_page->sizeHint());
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setButtonGuiItem(Ok, KStandardGuiItem::save());

    connect(m_ui->bnSelectSaveLocation, SIGNAL(clicked()), SLOT(selectSaveLocation()));

    KoDocumentInfo info;
    info.updateParameters();

    if (bundle) {

        setCaption(i18n("Edit Resource Bundle"));
#if 0
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

    QStringList resourceTypes = QStringList() << ResourceType::Brushes << ResourceType::PaintOpPresets << ResourceType::Gradients << ResourceType::GamutMasks;
#if defined HAVE_SEEXPR
    resourceTypes << ResourceType::SeExprScripts;
#endif
    resourceTypes << ResourceType::Patterns << ResourceType::Palettes << ResourceType::Workspaces;

    for (int i = 0; i < resourceTypes.size(); i++) {
        m_ui->cmbResourceTypes->addItem(ResourceName::resourceTypeToName(resourceTypes[i]), resourceTypes[i]);
    }
    connect(m_ui->cmbResourceTypes, SIGNAL(activated(int)), SLOT(resourceTypeSelected(int)));

    m_ui->tableAvailable->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_ui->tableAvailable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_ui->tableSelected->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_ui->tableSelected->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(m_ui->bnGetPreview, SIGNAL(clicked()), SLOT(getPreviewImage()));
    connect(m_ui->bnEmbedTags, SIGNAL(clicked()), SLOT(slotEmbedTags()));

    resourceTypeSelected(0);
}

DlgCreateBundle::~DlgCreateBundle()
{
    delete m_ui;
}

QString DlgCreateBundle::bundleName() const
{
    return m_ui->editBundleName->text();
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

QVector<KisTagSP> DlgCreateBundle::getTagsForEmbeddingInResource(QVector<KisTagSP> resourceTags) const
{
    QVector<KisTagSP> tagsToEmbed;
    Q_FOREACH(KisTagSP tag, resourceTags) {
        if (m_selectedTagIds.contains(tag->id())) {
            tagsToEmbed << tag;
        }
    }
    return tagsToEmbed;
}

void DlgCreateBundle::putResourcesInTheBundle(KoResourceBundleSP bundle) const
{
    KisResourceModel emptyModel("");
    QMap<QString, QSharedPointer<KisResourceModel>> modelsPerResourceType;
    KisResourceTypeModel resourceTypesModel;
    for (int i = 0; i < resourceTypesModel.rowCount(); i++) {
        QModelIndex idx = resourceTypesModel.index(i, 0);
        QString resourceType = resourceTypesModel.data(idx, Qt::UserRole + KisResourceTypeModel::ResourceType).toString();
        QSharedPointer<KisResourceModel> model = QSharedPointer<KisResourceModel>(new KisResourceModel(resourceType));
        modelsPerResourceType.insert(resourceType, model);
    }

    QStringList resourceTypes = modelsPerResourceType.keys();

    QStack<int> allResourcesIds;
    Q_FOREACH(int id, m_selectedResourcesIds) {
        allResourcesIds << id;
    }

    // note: if there are repetitions, it's fine; the bundle will filter them out
    qWarning() << resourceTypes << allResourcesIds;
    while(!allResourcesIds.isEmpty()) {
        int id = allResourcesIds.takeFirst();
        KoResourceSP res;
        QString resourceTypeHere = "";
        QSharedPointer<KisResourceModel> resModel;
        for (int i = 0; i < resourceTypes.size(); i++) {
            res = modelsPerResourceType[resourceTypes[i]]->resourceForId(id);
            if (!res.isNull()) {
                resModel = modelsPerResourceType[resourceTypes[i]];
                resourceTypeHere = resourceTypes[i];
                break;
            }
        }
        if (!res) {
            warnKrita << "No resource for id " << id;
            continue;
        }
        QVector<KisTagSP> tags = getTagsForEmbeddingInResource(resModel->tagsForResource(id));
        bundle->addResource(res->resourceType().first, res->filename(), tags, res->md5Sum(), res->resourceId(), createPrettyFilenameFromName(res));

        QList<KoResourceLoadResult> linkedResources = res->linkedResources(KisGlobalResourcesInterface::instance());
        Q_FOREACH(KoResourceLoadResult linkedResource, linkedResources) {
            // we have requested linked resources, how can it be an embedded one?
            KIS_SAFE_ASSERT_RECOVER(linkedResource.type() != KoResourceLoadResult::EmbeddedResource) { continue; }

            KoResourceSP resource = linkedResource.resource();

            if (!resource) {
                qWarning() << "WARNING: DlgCreateBundle::putResourcesInTheBundle couldn't fetch a linked resource" << linkedResource.signature();
                continue;
            }

            if (!allResourcesIds.contains(resource->resourceId())) {
                allResourcesIds.append(resource->resourceId());
            }
        }
    }
}

void DlgCreateBundle::putMetaDataInTheBundle(KoResourceBundleSP bundle) const
{
    bundle->setMetaData(KisResourceStorage::s_meta_author, authorName());
    bundle->setMetaData(KisResourceStorage::s_meta_title,  bundleName());
    bundle->setMetaData(KisResourceStorage::s_meta_description, description());
    if (bundle->metaData(KisResourceStorage::s_meta_initial_creator, "").isEmpty()) {
        bundle->setMetaData(KisResourceStorage::s_meta_initial_creator,  authorName());
    }
    bundle->setMetaData(KisResourceStorage::s_meta_creator, authorName());
    bundle->setMetaData(KisResourceStorage::s_meta_email, email());
    bundle->setMetaData(KisResourceStorage::s_meta_license, license());
    bundle->setMetaData(KisResourceStorage::s_meta_website, website());

    bundle->setThumbnail(previewImage());

    // For compatibility
    bundle->setMetaData("email", email());
    bundle->setMetaData("license", license());
    bundle->setMetaData("website", website());
}

QString DlgCreateBundle::createPrettyFilenameFromName(KoResourceSP resource) const
{
    QString resourceType = resource->resourceType().first;
    if (resourceType == ResourceType::Patterns
            || resourceType == ResourceType::Brushes) {
        // don't change the filename, if the resource is likely to be linked
        // by filename to another resource
        return resource->filename();
    }
    // to make sure patterns are saved correctly
    // note that for now (TM) there are no double-suffixes in resources (like '.tar.gz')
    // otherwise this code should use completeSuffix() and remove the versioning number
    // (since that's something we want to get rid of)
    const auto fileInfo = QFileInfo(resource->filename());
    const auto prefix = fileInfo.dir();
    // remove the suffix if the name has a suffix (happens for png patterns)
    const auto nameWithoutSuffix = QFileInfo(resource->name()).baseName();
    const auto suffix = fileInfo.suffix();
    return QDir::cleanPath(prefix.filePath(nameWithoutSuffix + "." + suffix));
}

void DlgCreateBundle::accept()
{
    QString name = bundleName();
    QString filename = QString("%1/%2.bundle").arg(m_ui->lblSaveLocation->text(), name.replace(" ", "_"));

    if (name.isEmpty()) {
        m_ui->editBundleName->setStyleSheet(QString(" border: 1px solid red"));
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The resource bundle name cannot be empty."));
        return;
    }
    m_ui->editBundleName->setStyleSheet(QString(""));

    if (m_ui->lblSaveLocation->text().isEmpty() ){
        m_ui->lblSaveLocation->setStyleSheet(QString(" border: 1px solid red"));
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The save location cannot be empty."));
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
            putMetaDataInTheBundle(m_bundle);
            putResourcesInTheBundle(m_bundle);
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

void DlgCreateBundle::slotEmbedTags()
{
    DlgEmbedTags* dlg = new DlgEmbedTags(m_selectedTagIds);
    int response = dlg->exec();
    if (response == KoDialog::Accepted) {
        m_selectedTagIds = dlg->selectedTagIds();
    }
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
    m_ui->tableSelected->sortItems();
}

void DlgCreateBundle::removeSelected()
{
    int row = m_ui->tableSelected->currentRow();

    Q_FOREACH (QListWidgetItem *item, m_ui->tableSelected->selectedItems()) {
        m_ui->tableAvailable->addItem(m_ui->tableSelected->takeItem(m_ui->tableSelected->row(item)));
        m_selectedResourcesIds.removeAll(item->data(Qt::UserRole).toInt());
    }

    m_ui->tableSelected->setCurrentRow(row);
    m_ui->tableAvailable->sortItems();
}

QPixmap imageToIcon(const QImage &img, Qt::AspectRatioMode aspectRatioMode) {
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    pixmap.fill();
    QImage scaled = img.scaled(ICON_SIZE, ICON_SIZE, aspectRatioMode, Qt::SmoothTransformation);
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
        Qt::AspectRatioMode scalingAspectRatioMode = Qt::KeepAspectRatio;
        if (image.height() == 1) { // affects mostly gradients, which are very long but only 1px tall
            scalingAspectRatioMode = Qt::IgnoreAspectRatio;
        }
        QListWidgetItem *item = new QListWidgetItem(image.isNull() ? QPixmap() : imageToIcon(image, scalingAspectRatioMode), name);
        item->setData(Qt::UserRole, id);

        if (m_selectedResourcesIds.contains(id)) {
            m_ui->tableSelected->addItem(item);
        }
        else {
            m_ui->tableAvailable->addItem(item);
        }
    }

    m_ui->tableSelected->sortItems();
    m_ui->tableAvailable->sortItems();
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



