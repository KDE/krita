/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TagFilterProxyModelQmlWrapper.h"

#include <KisResourceModel.h>
#include <KisResourceModelProvider.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisResourceMetaDataModel.h>
#include <KisTagModel.h>
#include <KisTagResourceModel.h>
#include <kis_signal_compressor.h>
#include <KisResourceSearchBoxFilter.h>
#include <KisResourceUserOperations.h>
#include <KisResourceLoaderRegistry.h>

#include <KoFileDialog.h>
#include <QStandardPaths>
#include <QApplication>

#include <resources/KoFontFamily.h>

struct TagFilterProxyModelQmlWrapper::Private {
    Private(QObject *parent = nullptr)
        : parent(parent)
        , compressor(KisSignalCompressor(100, KisSignalCompressor::POSTPONE, parent))
    {
    }

    void setupNewResourceType() {
        tagFilterProxyModel = resourceType == ResourceType::FontFamilies? new FontFamilyTagFilterModel(parent)
                                                                        : new KisTagFilterResourceProxyModel(resourceType, parent);
        tagModel = new KisTagModel(resourceType, parent);
        allResourceModel = KisResourceModelProvider::resourceModel(resourceType);
        tagFilterProxyModel->sort(KisAbstractResourceModel::Name);
        tagFilterProxyModel->setTagFilter(tagModel->tagForIndex(tagModel->index(0, 0)));
    }

    QObject *parent;
    KisTagFilterResourceProxyModel *tagFilterProxyModel {nullptr};
    KisAllResourcesModel *allResourceModel {nullptr};
    KisTagModel *tagModel {nullptr};
    KisSignalCompressor compressor;
    QString currentSearchText;
    KoResourceSP currentResource;
    QString resourceType;
};

TagFilterProxyModelQmlWrapper::TagFilterProxyModelQmlWrapper(QObject *parent)
    : QObject(parent)
    , d(new Private(parent))
{

    connect(&d->compressor, SIGNAL(timeout()), this, SLOT(setSearchTextOnModel()));
}

TagFilterProxyModelQmlWrapper::~TagFilterProxyModelQmlWrapper()
{
}

QAbstractItemModel *TagFilterProxyModelQmlWrapper::model() const
{
    return d->tagFilterProxyModel;
}

QAbstractItemModel *TagFilterProxyModelQmlWrapper::tagModel() const
{
    return d->tagModel;
}

QString TagFilterProxyModelQmlWrapper::resourceType() const
{
    return d->resourceType;
}

void TagFilterProxyModelQmlWrapper::setResourceType(const QString &type)
{
    if (d->resourceType == type) return;

    d->resourceType = type;
    d->setupNewResourceType();
    d->currentSearchText = "";
    d->currentResource.reset();

    emit resourceTypeChanged();
    emit tagModelChanged();
    emit modelChanged();
    emit searchTextChanged();
    emit searchInTagChanged();
    emit activeTagChanged();
    emit modelSortUpdated();
    emit currentResourceChanged();
}

QString TagFilterProxyModelQmlWrapper::resourceTypeName() const
{
    if (d->resourceType.isEmpty()) return i18n("Resource");
    // TODO: We should have separate translated entries for singular.
    return ResourceName::resourceTypeToName(d->resourceType);
}

bool TagFilterProxyModelQmlWrapper::importEnabled() const
{
    return KisResourceLoaderRegistry::instance()->resourceTypes().contains(d->resourceType);
}

void TagFilterProxyModelQmlWrapper::tagActivated(const int &row)
{
    if (!d->tagModel || !d->tagFilterProxyModel) return;
    QModelIndex idx = d->tagModel->index(row, 0);
    if (idx.isValid()) {
        KisTagSP tag = d->tagModel->tagForIndex(idx);
        if (tag == d->tagFilterProxyModel->currentTagFilter()) return;
        d->tagFilterProxyModel->setTagFilter(tag);
        emit activeTagChanged();
    }
}

int TagFilterProxyModelQmlWrapper::currentTag() const
{
    if (!d->tagModel) return 0;
    return d->tagModel->indexForTag(d->tagFilterProxyModel->currentTagFilter()).row();
}

QString TagFilterProxyModelQmlWrapper::searchText() const
{
    return d->currentSearchText;
}

void TagFilterProxyModelQmlWrapper::setSearchText(const QString &text)
{
    if (d->currentSearchText == text) {
        return;
    }
    d->currentSearchText = text;
    emit searchTextChanged();
    d->compressor.start();
}

void TagFilterProxyModelQmlWrapper::setSearchInTag(const bool &newSearchInTag)
{
    if (!d->tagFilterProxyModel) return;
    if (d->tagFilterProxyModel->filterInCurrentTag() != newSearchInTag) {
        d->tagFilterProxyModel->setFilterInCurrentTag(newSearchInTag);
        emit searchInTagChanged();
    }
}

bool TagFilterProxyModelQmlWrapper::searchInTag()
{
    if (! d->tagFilterProxyModel) return false;
    return d->tagFilterProxyModel->filterInCurrentTag();
}

void TagFilterProxyModelQmlWrapper::addNewTag(const QString &newTagName, const int &resourceIndex)
{
    if (!d->tagModel || !d->tagFilterProxyModel) return;
    KisTagSP tagsp = d->tagModel->tagForUrl(newTagName);
    QModelIndex resourceIdx = d->tagFilterProxyModel->index(resourceIndex, 0);
    if (tagsp.isNull()) {
        QVector<KoResourceSP> vec;
        tagsp = d->tagModel->addTag(newTagName, false, vec);

    }
    // TODO: figure out how to get a tag reactivated again, without doing too much code duplication :|
    if (resourceIdx.isValid()) {
        int resourceId = d->tagFilterProxyModel->data(resourceIdx, Qt::UserRole + KisResourceModel::Id).toInt();
        d->tagFilterProxyModel->tagResources(tagsp, {resourceId});
    }

}

void TagFilterProxyModelQmlWrapper::tagResource(const int &tagIndex, const int &resourceIndex)
{
    if (!d->tagModel || !d->tagFilterProxyModel) return;
    QModelIndex idx = d->tagModel->index(tagIndex, 0);
    QModelIndex resourceIdx = d->tagFilterProxyModel->index(resourceIndex, 0);
    KisTagSP tagsp;
    if (idx.isValid()) {
        tagsp = d->tagModel->tagForIndex(idx);
    }
    if (tagsp && resourceIdx.isValid()) {
        int resourceId = d->tagFilterProxyModel->data(resourceIdx, Qt::UserRole + KisResourceModel::Id).toInt();
        d->tagFilterProxyModel->tagResources(tagsp, {resourceId});
    }
}

void TagFilterProxyModelQmlWrapper::untagResource(const int &tagIndex, const int &resourceIndex)
{
    if (!d->tagModel || !d->tagFilterProxyModel) return;
    QModelIndex idx = d->tagModel->index(tagIndex, 0);
    QModelIndex resourceIdx = d->tagFilterProxyModel->index(resourceIndex, 0);
    KisTagSP tagsp;
    if (idx.isValid()) {
        tagsp = d->tagModel->tagForIndex(idx);
    }
    if (tagsp && resourceIdx.isValid()) {
        int resourceId = d->tagFilterProxyModel->data(resourceIdx, Qt::UserRole + KisResourceModel::Id).toInt();
        d->tagFilterProxyModel->untagResources(tagsp, {resourceId});
    }
}

QVariantList TagFilterProxyModelQmlWrapper::taggedResourceModel (const int &resourceIndex) const
{
    QVariantList taggedResourceModel;
    if (!d->tagModel || !d->tagFilterProxyModel) return taggedResourceModel;
    QModelIndex resourceIdx = d->tagFilterProxyModel->index(resourceIndex, 0);
    KisTagSP tagsp;
    for (int i = 0; i< d->tagModel->rowCount(); i++) {
        QModelIndex idx = d->tagModel->index(i, 0);
        tagsp = d->tagModel->tagForIndex(idx);
        bool visible = tagsp->id() >= 0;
        bool enabled = false;
        if (resourceIdx.isValid()) {
            int resourceId = d->tagFilterProxyModel->data(resourceIdx, Qt::UserRole + KisResourceModel::Id).toInt();
            enabled = d->tagFilterProxyModel->isResourceTagged(tagsp, resourceId) > 0 && visible;
        }
        QVariantMap tag { {"name", tagsp->name()}, {"value", i}, {"visible", visible}, {"enabled", enabled} };
        taggedResourceModel.append(tag);
    }
    return taggedResourceModel;
}

bool TagFilterProxyModelQmlWrapper::showResourceTagged(const int &tagIndex, const int &resourceIndex) const
{
    if (!d->tagModel || !d->tagFilterProxyModel) return false;
    QModelIndex idx = d->tagModel->index(tagIndex, 0);
    QModelIndex resourceIdx = d->tagFilterProxyModel->index(resourceIndex, 0);
    KisTagSP tagsp;
    if (idx.isValid()) {
        tagsp = d->tagModel->tagForIndex(idx);
    }
    if (tagsp) {
        if (tagsp->id() < 0) return false;
        return true;
        if (resourceIdx.isValid()) {
            int resourceId = d->tagFilterProxyModel->data(resourceIdx, Qt::UserRole + KisResourceModel::Id).toInt();
            return d->tagFilterProxyModel->isResourceTagged(tagsp, resourceId);
        }
    }
    return false;
}

int TagFilterProxyModelQmlWrapper::currentIndex() const
{
    if (!d->tagFilterProxyModel) return -1;
    return d->tagFilterProxyModel->indexForResource(d->currentResource).row();
}

void TagFilterProxyModelQmlWrapper::setCurrentIndex(const int &index)
{
    if (!d->tagFilterProxyModel) return;
    QModelIndex resourceIdx = d->tagFilterProxyModel->index(index, 0);
    if (!resourceIdx.isValid()) return;
    KoResourceSP newResource = d->tagFilterProxyModel->resourceForIndex(resourceIdx);
    if (newResource != d->currentResource) {
        d->currentResource = newResource;
        emit currentResourceChanged();
    }
}

void TagFilterProxyModelQmlWrapper::setResourceToFileName(const QString &filename)
{
    if (!d->allResourceModel) return;
    if (d->currentResource && d->currentResource->filename() == filename) return;
    KoResourceSP resource = d->currentResource;
    QVector<KoResourceSP> resources = d->allResourceModel->resourcesForFilename(filename);
    if (!resources.isEmpty()) {
        resource = resources.first();
    }
    if (resource != d->currentResource) {
        d->currentResource = resource;
        emit currentResourceChanged();
    }
}

QString TagFilterProxyModelQmlWrapper::resourceFilename()
{
    return d->currentResource? d->currentResource->filename(): "";
}

KoResourceSP TagFilterProxyModelQmlWrapper::currentResource() const
{
    return d->currentResource;
}

void TagFilterProxyModelQmlWrapper::importResource()
{
    // Below copied from KisResourceItemChooser::slotButtonClicked
    QStringList mimeTypes = KisResourceLoaderRegistry::instance()->mimeTypes(d->resourceType);
    KoFileDialog dialog(QApplication::activeWindow(), KoFileDialog::OpenFiles, "OpenDocument");
    dialog.setMimeTypeFilters(mimeTypes);
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setCaption(i18nc("@title:window", "Choose File to Add"));
    Q_FOREACH(const QString &filename, dialog.filenames()) {
        if (QFileInfo(filename).exists() && QFileInfo(filename).isReadable()) {

            KoResourceSP previousResource = this->currentResource();
            KoResourceSP newResource = KisResourceUserOperations::importResourceFileWithUserInput(QApplication::activeWindow(), "", d->resourceType, filename);

            if (previousResource && newResource && !currentResource()) {
                /// We have overridden the currently selected resource and
                /// nothing is selected now
                QModelIndex index = d->tagFilterProxyModel->indexForResource(newResource);
                setCurrentIndex(index.row());
            }
        }
    }
}

void TagFilterProxyModelQmlWrapper::removeResource()
{
    // Below copied from KisResourceItemChooser::slotButtonClicked
    QModelIndex index = d->tagFilterProxyModel->indexForResource(d->currentResource);
    if (index.isValid()) {
        d->tagFilterProxyModel->setResourceInactive(index);
    }
    int row = index.row();
    int rowMin = --row;
    row = qBound(0, rowMin, row);
    setCurrentIndex(row);
}

void TagFilterProxyModelQmlWrapper::setSearchTextOnModel()
{
    if (!d->tagFilterProxyModel) return;
    d->tagFilterProxyModel->setSearchText(d->currentSearchText);
    emit modelSortUpdated();
}

FontFamilyTagFilterModel::FontFamilyTagFilterModel(QObject *parent)
    : KisTagFilterResourceProxyModel(ResourceType::FontFamilies, parent)
{
}

QVariant FontFamilyTagFilterModel::data(const QModelIndex &index, int role) const
{
    QModelIndex sourceIdx = mapToSource(index);
    if (sourceIdx.isValid()) {
        if (role == (Qt::UserRole + KisAbstractResourceModel::Name)) {
            const int resourceId = sourceModel()->data(sourceIdx, Qt::UserRole + KisAbstractResourceModel::Id).toInt();
            const QVariantMap localizedNames = KisResourceModelProvider::resourceMetadataModel()->metaDataValue(resourceId, "localized_font_family").toMap();
            const QString fallBack = sourceModel()->data(sourceIdx, Qt::UserRole + KisAbstractResourceModel::Name).toString();
            QString name = fallBack.isEmpty()? localizedNames.value("en").toString(): fallBack;

            Q_FOREACH(const QLocale locale, KLocalizedString::languages()) {
                bool found = false;
                Q_FOREACH(const QString key, localizedNames.keys()) {
                    if (QLocale(key) == locale) {
                        name = localizedNames.value(key, name).toString();
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            return name;
        } else {
            return sourceModel()->data(sourceIdx, role);
        }
    }
    return QVariant();
}

bool FontFamilyTagFilterModel::additionalResourceNameChecks(const QModelIndex &index, const KisResourceSearchBoxFilter *filter) const
{
    bool match = false;
    if (index.isValid()) {
        const QStringList resourceTags = sourceModel()->data(index, Qt::UserRole + KisAbstractResourceModel::Tags).toStringList();

        KisResourceMetaDataModel *metadataModel = KisResourceModelProvider::resourceMetadataModel();
        const int resourceId = sourceModel()->data(index, Qt::UserRole + KisResourceModel::Id).toInt();
        const QVariantMap localizedNames = metadataModel->metaDataValue(resourceId, "localized_font_family").toMap();

        Q_FOREACH(const QVariant localizedName, localizedNames.values()) {
            match = filter->matchesResource(localizedName.toString(), resourceTags);
            if (match) {
                break;
            }
        }
    }
    return match;
}
