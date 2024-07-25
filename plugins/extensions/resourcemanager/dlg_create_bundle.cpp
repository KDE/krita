/*
 *  SPDX-FileCopyrightText: 2014 Victor Lafon metabolic.ewilan @hotmail.fr
 *  SPDX-FileCopyrightText: 2020 Agata Cacko cacko.azh @gmail.com
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "dlg_create_bundle.h"
#include "page_resource_chooser.h"
#include "page_tag_chooser.h"
#include "page_metadata_info.h"
#include "page_bundle_saver.h"
#include "ui_wdgdlgcreatebundle.h"
#include "wdg_side.h"

#include <QDebug>
#include <QFileInfo>
#include <QGridLayout>
#include <QHash>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QProcessEnvironment>
#include <QStack>
#include <QStandardPaths>
#include <QTableWidget>
#include <QVBoxLayout>

#include <KisImportExportManager.h>
#include <KoDocumentInfo.h>
#include <KoFileDialog.h>
#include <kis_config.h>
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
#include "KisBundleStorage.h"
#include <KisResourceLocator.h>
#include <KisResourceStorage.h>
#include <QUuid>
#include <KisStorageModel.h>
#include <wdgtagselection.h>


#define ICON_SIZE 48

DlgCreateBundle::DlgCreateBundle(KoResourceBundleSP bundle, QWidget *parent)
    : QWizard(parent)
    , m_ui(new Ui::WdgDlgCreateBundle)
    , m_bundle(bundle)
    , m_storageAdded(false)
{
    m_ui->setupUi(this);
    setWizardStyle(QWizard::ClassicStyle);

    WdgSide *wdgSide = new WdgSide(m_bundle);

    m_pageResourceChooser = new PageResourceChooser(m_bundle);
    m_pageTagChooser = new PageTagChooser(m_bundle);
    m_pageMetadataInfo = new PageMetadataInfo(m_bundle);
    m_pageBundleSaver = new PageBundleSaver(m_bundle);

    setPage(1, m_pageResourceChooser);
    setPage(2, m_pageTagChooser);
    setPage(3, m_pageMetadataInfo);
    setPage(4, m_pageBundleSaver);

    setSideWidget(wdgSide);
    setButtonText(QWizard::FinishButton, i18n("Save"));

    connect(this, SIGNAL(currentIdChanged(int)), wdgSide, SLOT(focusLabel(int)));
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(updateTitle(int)));

    KisResourceTypeModel resourceTypesModel;
    for (int i = 0; i < resourceTypesModel.rowCount(); i++) {
        QModelIndex idx = resourceTypesModel.index(i, 0);
        QString resourceType = resourceTypesModel.data(idx, Qt::UserRole + KisResourceTypeModel::ResourceType).toString();
        m_count.insert(resourceType, 0);
    }

    m_bundleCreaterMode = "Creator";

    if (m_bundle) {
        m_bundleCreaterMode = "Editor";
        m_bundleStorage = new KisBundleStorage(m_bundle->filename());

        QStringList resourceTypes = QStringList() << ResourceType::Brushes << ResourceType::PaintOpPresets << ResourceType::Gradients << ResourceType::GamutMasks;
        #if defined HAVE_SEEXPR
        resourceTypes << ResourceType::SeExprScripts;
        #endif
        resourceTypes << ResourceType::Patterns << ResourceType::Palettes << ResourceType::Workspaces;

        for (int i = 0; i < resourceTypes.size(); i++) {
            QSharedPointer<KisResourceStorage::ResourceIterator> iter = m_bundleStorage->resources(resourceTypes[i]);
            int count = 0;
            while (iter->hasNext()) {
                iter->next();
                count++;
            }
            m_count[resourceTypes[i]] = count;

            QSharedPointer<KisResourceStorage::TagIterator> tagIter = m_bundleStorage->tags(resourceTypes[i]);
            while (tagIter->hasNext()) {
                tagIter->next();
                m_tags.insert(tagIter->tag()->name());
            }
        }

    }

    connect(m_pageResourceChooser, SIGNAL(countUpdated()), m_pageBundleSaver, SLOT(onCountUpdated()));
    connect(m_pageTagChooser, SIGNAL(tagsUpdated()), m_pageBundleSaver, SLOT(onTagsUpdated()));
}

void DlgCreateBundle::updateTitle(int id)
{
    if (!m_bundle) {
        QString title = i18n("Create Resource Bundle");

        switch(currentId()) {
        case 1: title = i18n("Choose Resources"); break;
        case 2: title = i18n("Choose Tags"); break;
        case 3: title = i18n("Enter Bundle Details"); break;
        case 4: title = i18n("Enter Save Location"); break;
        }

        setWindowTitle(title);
    } else {
        QString title = i18n("Edit Resource Bundle");
        
        switch(currentId()) {
        case 1: title = i18n("Edit Resources"); break;
        case 2: title = i18n("Edit Tags"); break;
        case 3: title = i18n("Edit Bundle Details"); break;
        case 4: title = i18n("Edit Save Location"); break;
        }

        setWindowTitle(title);
    }
}

DlgCreateBundle::~DlgCreateBundle()
{
    delete m_ui;
}

QVector<KisTagSP> DlgCreateBundle::getTagsForEmbeddingInResource(QVector<KisTagSP> resourceTags, QString resourceType) const
{
    QVector<KisTagSP> tagsToEmbed;
    KisTagModel *tagModel = new KisTagModel(resourceType);

    for (int i = 0; i < tagModel->rowCount(); i++) {
        QModelIndex idx = tagModel->index(i, 0);
        int id = tagModel->data(idx, Qt::UserRole + KisAllTagsModel::Id).toInt();
        QString url = tagModel->data(idx, Qt::UserRole + KisAllTagsModel::Url).toString();
        if (m_selectedTagIds.contains(id)) {
            KisTagSP tag = tagModel->tagForUrl(url);
            tagsToEmbed << tag;
        }
    }

    return tagsToEmbed;
}

bool DlgCreateBundle::putResourcesInTheBundle(KoResourceBundleSP bundle)
{
    QMap<QString, QSharedPointer<KisResourceModel>> modelsPerResourceType;
    KisResourceTypeModel resourceTypesModel;
    for (int i = 0; i < resourceTypesModel.rowCount(); i++) {
        QModelIndex idx = resourceTypesModel.index(i, 0);
        QString resourceType = resourceTypesModel.data(idx, Qt::UserRole + KisResourceTypeModel::ResourceType).toString();
        QSharedPointer<KisResourceModel> model = QSharedPointer<KisResourceModel>(new KisResourceModel(resourceType));
        // BUG: 445408 Ensure potentially linked but disabled resources are visible
        model->setResourceFilter(KisResourceModel::ShowAllResources);
        modelsPerResourceType.insert(resourceType, model);
    }

    QStringList resourceTypes = modelsPerResourceType.keys();
    QList<int> selectedResourcesIds = m_pageResourceChooser->getSelectedResourcesIds();

    QStack<int> allResourcesIds;
    Q_FOREACH(int id, selectedResourcesIds) {
        allResourcesIds << id;
    }

    // note: if there are repetitions, it's fine; the bundle will filter them out
    QHash<QPair<QString, QString>, std::size_t> usedFilenames;

    while(!allResourcesIds.isEmpty()) {
        const int id = allResourcesIds.takeFirst();
        KoResourceSP res;
        QString resourceTypeHere = "";
        QSharedPointer<KisResourceModel> resModel;
        for (const auto &type: resourceTypes) {
            res = modelsPerResourceType[type]->resourceForId(id);
            if (!res.isNull()) {
                resModel = modelsPerResourceType[type];
                resourceTypeHere = type;
                break;
            }
        }
        if (!res) {
            warnKrita << "No resource for id " << id;
            continue;
        }
        const auto prettyFilename = createPrettyFilenameFromName(res);

        if (usedFilenames.value({res->resourceType().first, prettyFilename}, 0) > 0) {
            QMessageBox::warning(
                this,
                i18nc("@title:window", "Krita"),
                i18nc("Warning message", "More than one resource share the same file name '%1'. Please export them in separate bundles.", prettyFilename));
            return false;
        }

        usedFilenames[{res->resourceType().first, prettyFilename}]+= 1;

        m_selectedTagIds = m_pageTagChooser->selectedTagIds();

        QVector<KisTagSP> tags = getTagsForEmbeddingInResource(resModel->tagsForResource(id), res->resourceType().first);
        bundle->addResource(res->resourceType().first, res->filename(), tags, res->md5Sum(), res->resourceId(), prettyFilename);

        if (m_bundleCreaterMode == "Creator") {
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
    return true;
}

void DlgCreateBundle::putMetaDataInTheBundle(KoResourceBundleSP bundle) const
{
    bundle->setMetaData(KisResourceStorage::s_meta_author, m_pageMetadataInfo->authorName());
    bundle->setMetaData(KisResourceStorage::s_meta_title,  m_pageMetadataInfo->bundleName());
    bundle->setMetaData(KisResourceStorage::s_meta_description, m_pageMetadataInfo->description());
    if (bundle->metaData(KisResourceStorage::s_meta_initial_creator, "").isEmpty()) {
        bundle->setMetaData(KisResourceStorage::s_meta_initial_creator,  m_pageMetadataInfo->authorName());
    }
    bundle->setMetaData(KisResourceStorage::s_meta_creator, m_pageMetadataInfo->authorName());
    bundle->setMetaData(KisResourceStorage::s_meta_email, m_pageMetadataInfo->email());
    bundle->setMetaData(KisResourceStorage::s_meta_license, m_pageMetadataInfo->license());
    bundle->setMetaData(KisResourceStorage::s_meta_website, m_pageMetadataInfo->website());

    if (m_bundle) {
        bundle->setThumbnail(m_pageMetadataInfo->thumbnail());
    } else {
        bundle->setThumbnail(m_pageMetadataInfo->previewImage());
    }

    // For compatibility
    bundle->setMetaData("email", m_pageMetadataInfo->email());
    bundle->setMetaData("license", m_pageMetadataInfo->license());
    bundle->setMetaData("website", m_pageMetadataInfo->website());
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
    const auto nameWithoutSuffix = QFileInfo(resource->name()).completeBaseName();
    const auto suffix = fileInfo.suffix();
    return QDir::cleanPath(prefix.filePath(nameWithoutSuffix + "." + suffix));
}

void DlgCreateBundle::saveToConfiguration(bool full)
{
    KisConfig cfg(false);
    if (full) {
        cfg.writeEntry<QString>("BundleName", m_pageMetadataInfo->bundleName());
        cfg.writeEntry<QString>("BundleDescription", m_pageMetadataInfo->description());
        cfg.writeEntry<QString>("BundleImage", m_pageMetadataInfo->previewImage());
    } else {
        cfg.writeEntry<QString>("BundleName", "");
        cfg.writeEntry<QString>("BundleDescription", "");
        cfg.writeEntry<QString>("BundleImage", "");
    }
    cfg.writeEntry<QString>("BundleExportLocation", m_pageBundleSaver->saveLocation());
    cfg.writeEntry<QString>("BundleAuthorName", m_pageMetadataInfo->authorName());
    cfg.writeEntry<QString>("BundleAuthorEmail", m_pageMetadataInfo->email());
    cfg.writeEntry<QString>("BundleWebsite", m_pageMetadataInfo->website());
    cfg.writeEntry<QString>("BundleLicense", m_pageMetadataInfo->license());
}

QPixmap imageToIcon(const QImage &img, Qt::AspectRatioMode aspectRatioMode) {
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    return pixmap;
}

void DlgCreateBundle::accept()
{
    QString name = m_pageMetadataInfo->bundleName();
    QString filename = QString("%1/%2.bundle").arg(m_pageBundleSaver->saveLocation(), name.replace(" ", "_"));

    if (name.isEmpty()) {
        m_pageMetadataInfo->showWarning();
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The resource bundle name cannot be empty."));
        return;
    }
    m_pageMetadataInfo->removeWarning();

    if (m_pageBundleSaver->saveLocation().isEmpty() ){
        m_pageBundleSaver->showWarning();
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The save location cannot be empty."));
        return;
    }
    else {
        QFileInfo fileInfo(filename);

        if (fileInfo.exists()) {
            m_pageMetadataInfo->showWarning();

            QMessageBox msgBox(this);
            msgBox.setIcon(QMessageBox::Question);
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
            if (!putResourcesInTheBundle(m_bundle)) {
                return;
            }
            if (!m_bundle->save()) {
                m_pageBundleSaver->showWarning();
                QMessageBox::critical(this,
                    i18nc("@title:window", "Krita"),
                    i18n("Could not open '%1' for saving.", filename));
                m_bundle.reset();
                return;
            }
        } else {
            saveToConfiguration(false);

            m_bundle.reset(new KoResourceBundle(filename));
            putMetaDataInTheBundle(m_bundle);
            if (!putResourcesInTheBundle(m_bundle)) {
                return;
            }
            if (!m_bundle->save()) {
                m_pageBundleSaver->showWarning();
                QMessageBox::critical(this,
                    i18nc("@title:window", "Krita"),
                    i18n("Could not open '%1' for saving.", filename));
                m_bundle.reset();
                return;
            }
        }
        QWizard::accept();
    }
}

void DlgCreateBundle::reject()
{
    if (!m_bundle) {
        saveToConfiguration(true);
    }

    QWizard::reject();
}


