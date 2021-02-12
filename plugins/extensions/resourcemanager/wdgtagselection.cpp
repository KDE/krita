/*
 *  Copyright (c) 2020 Agata Cacko cacko.azh@gmail.com
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "wdgtagselection.h"

#include <QProcessEnvironment>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>
#include <QGridLayout>
#include <QTableWidget>
#include <QPainter>
#include <QListWidget>
#include <QAction>
#include <QMouseEvent>
#include <QMenu>
#include <QPair>
#include <QTimer>

#include <KisImportExportManager.h>
#include <KoDocumentInfo.h>
#include <KoFileDialog.h>
#include <kis_icon.h>
#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KisTagModel.h>

#include<KisWrappableHBoxLayout.h>


#include "kis_icon.h"

KisWdgTagSelectionControllerOneResource::KisWdgTagSelectionControllerOneResource(KisTagSelectionWidget *widget, bool editable)
    : QObject(widget)
    , m_tagSelectionWidget(widget)
    , m_editable(editable)
{
    connect(widget, SIGNAL(sigAddTagToSelection(KoID)), this, SLOT(slotAddTag(KoID)));
    connect(widget, SIGNAL(sigRemoveTagFromSelection(KoID)), this, SLOT(slotRemoveTag(KoID)));
}

KisWdgTagSelectionControllerOneResource::~KisWdgTagSelectionControllerOneResource()
{

}

void KisWdgTagSelectionControllerOneResource::setResource(KoResourceSP resource)
{
    m_resource = resource;
    if (resource.isNull()) {
        m_tagModel = 0;
        m_tagResourceModel = 0;
        QList<KoID> list;
        m_tagSelectionWidget->setTagList(m_editable, list, list);

    } else {
        m_tagResourceModel.reset(new KisTagResourceModel(resource->resourceType().first));
        m_tagResourceModel->setResourcesFilter(QVector<int>() << resource->resourceId());
        m_tagModel.reset(new KisTagModel(resource->resourceType().first));

        updateView();
    }
}

void KisWdgTagSelectionControllerOneResource::slotRemoveTag(KoID tag)
{
    if (m_resource.isNull()) return;

    KisTagSP tagsp = m_tagModel->tagForUrl(tag.id());
    m_tagResourceModel->untagResource(tagsp, m_resource->resourceId());
    updateView();
}

void KisWdgTagSelectionControllerOneResource::slotAddTag(KoID tag)
{
    if (m_resource.isNull()) return;

    KisTagSP tagsp = m_tagModel->tagForUrl(tag.id());
    m_tagResourceModel->tagResource(tagsp, m_resource->resourceId());
    updateView();
}

void KisWdgTagSelectionControllerOneResource::updateView()
{
    if (m_resource.isNull()) return;

    QList<KoID> selected;
    for (int i = 0; i < m_tagResourceModel->rowCount(); i++) {
        QModelIndex idx = m_tagResourceModel->index(i, 0);
        KisTagSP tag = m_tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::Tag).value<KisTagSP>();
        KoID custom = KoID(tag->url(), tag->name());
        selected << custom;
    }
    QList<KoID> toSelect;
    for (int i = 0; i < m_tagModel->rowCount(); i++) {
        QModelIndex idx = m_tagModel->index(i, 0);
        int id = m_tagModel->data(idx, Qt::UserRole + KisAllTagsModel::Id).toInt();
        if (id < 0) {
            continue;
        }
        QString tagUrl = m_tagModel->data(idx, Qt::UserRole + KisAllTagsModel::Url).toString();
        bool isSelected = false;
        for (int j = 0; j < selected.count(); j++) {
            KoID other = (selected[j]);
            if (other.id() == tagUrl) {
                isSelected = true;
                break;
            }
        }
        if (!isSelected) {
            QString tagName = m_tagModel->data(idx, Qt::UserRole + KisAllTagsModel::Name).toString();
            KoID custom = KoID(tagUrl, tagName);
            toSelect << custom;
        }
    }
    m_tagSelectionWidget->setTagList(m_editable, selected, toSelect);
}

KisWdgTagSelectionControllerBundleTags::KisWdgTagSelectionControllerBundleTags(KisTagSelectionWidget *widget, bool editable)
    : QObject(widget)
    , m_tagSelectionWidget(widget)
    , m_editable(editable)
{
    ENTER_FUNCTION() << ppVar(widget);
    connect(widget, SIGNAL(sigAddTagToSelection(KoID)), this, SLOT(slotAddTag(KoID)));
    connect(widget, SIGNAL(sigRemoveTagFromSelection(KoID)), this, SLOT(slotRemoveTag(KoID)));
    updateView();
}

KisWdgTagSelectionControllerBundleTags::~KisWdgTagSelectionControllerBundleTags()
{

}

QList<int> KisWdgTagSelectionControllerBundleTags::getSelectedTagIds() const
{
    QList<int> selectedTags;
    Q_FOREACH(QString resourceType, m_selectedTagsByResourceType.keys()) {
        KisTagModel* model = new KisTagModel(m_resourceType);
        QList<KoID> tagList = m_selectedTagsByResourceType[resourceType];
        Q_FOREACH(KoID tag, tagList) {
            KisTagSP tagSP = model->tagForUrl(tag.id());
            selectedTags << tagSP->id();
        }
    }
    return selectedTags;
}

void KisWdgTagSelectionControllerBundleTags::slotRemoveTag(KoID custom)
{
    ENTER_FUNCTION();
    if (m_selectedTagsByResourceType.contains(m_resourceType)) {
        if (m_selectedTagsByResourceType[m_resourceType].contains(custom)) {
            m_selectedTagsByResourceType[m_resourceType].removeAll(custom);
            updateView();
        }
    }
}

void KisWdgTagSelectionControllerBundleTags::slotAddTag(KoID custom)
{
    if (!m_selectedTagsByResourceType.contains(m_resourceType)) {
        m_selectedTagsByResourceType.insert(m_resourceType, QList<KoID>());
    }
    if (!m_selectedTagsByResourceType[m_resourceType].contains(custom)) {
        m_selectedTagsByResourceType[m_resourceType].append(custom);
        updateView();
    }
}

void KisWdgTagSelectionControllerBundleTags::updateView()
{

    ENTER_FUNCTION() << ppVar(m_tagSelectionWidget);

    typedef QPair<QString, QString> resourceTypePair;
    QList<QPair<QString, QString>> resourceTypes = {
        resourceTypePair(i18n("Brush presets"), ResourceType::PaintOpPresets),
        resourceTypePair(i18n("Brush tips"), ResourceType::Brushes),
        resourceTypePair(i18n("Workspaces"), ResourceType::Workspaces),
        resourceTypePair(i18n("Patterns"), ResourceType::Patterns),
        resourceTypePair(i18n("Palettes"), ResourceType::Palettes),
        resourceTypePair(i18n("Layer styles"), ResourceType::LayerStyles),
        resourceTypePair(i18n("Gradients"), ResourceType::Gradients),
        resourceTypePair(i18n("Gamut masks"), ResourceType::GamutMasks),
        resourceTypePair(i18n("SeExpr scripts"), ResourceType::SeExprScripts),
    };
    QList<CustomTagsCategorySP> categoriesList;

    KisTagModel* model = new KisTagModel(m_resourceType);

    QList<KoID> selected = m_selectedTagsByResourceType.contains(m_resourceType) ? m_selectedTagsByResourceType[m_resourceType] : QList<KoID>();
    QList<KoID> notSelected;


    for (int i = 0; i < model->rowCount(); i++) {
        QModelIndex idx = model->index(i, 0);
        KisTagSP tag = model->data(idx, Qt::UserRole + KisAllTagsModel::KisTagRole).value<KisTagSP>();

        if (tag.isNull() || tag->id() < 0) {
            continue;
        }

        KoID custom = KoID(tag->url(), tag->name());

        if (m_selectedTagsByResourceType.contains(m_resourceType)) {
            if (!m_selectedTagsByResourceType[m_resourceType].contains(custom)) {
                notSelected << custom;
            }
        } else { // no tags from this resource type are selected
            notSelected << custom;
        }
    }

    // m_selectedTags is already categorized correctly and is in KoID form

    m_tagSelectionWidget->setTagList(m_editable, selected, notSelected);



}

void KisWdgTagSelectionControllerBundleTags::setResourceType(const QString &resourceType)
{
    m_resourceType = resourceType;
    updateView();
}













