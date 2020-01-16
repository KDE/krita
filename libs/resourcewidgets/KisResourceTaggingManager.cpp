/*
  *   This file is part of the KDE project
  *   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
  *   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
  *   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
  *   Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
  *   Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
  *   Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *
  *   This library is free software; you can redistribute it and/or
  *   modify it under the terms of the GNU Library General Public
  *   License as published by the Free Software Foundation; either
  *   version 2 of the License, or (at your option) any later version.
 *
  *   This library is distributed in the hope that it will be useful,
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  *   Library General Public License for more details.
 *
  *   You should have received a copy of the GNU Library General Public License
  *   along with this library; see the file COPYING.LIB.  If not, write to
  *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  *   Boston, MA 02110-1301, USA.
 */

#include "KisResourceTaggingManager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QPointer>
#include <QStringList>

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <KoResource.h>

#include <KisResourceModel.h>
#include <KisResourceModelProvider.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisTagModel.h>
#include <KisTagModelProvider.h>

#include "KisTagFilterWidget.h"
#include "KisTagChooserWidget.h"
#include "KisResourceItemChooserContextMenu.h"
#include "kis_debug.h"
#include "KisTag.h"


class TaggedResourceSet
{
public:
    TaggedResourceSet()
    {}

    TaggedResourceSet(const QString& tagName, const QList<KoResourceSP>& resources)
        : tagName(tagName)
        , resources(resources)
    {}

    QString tagName;
    QList<KoResourceSP> resources;
};


class KisResourceTaggingManager::Private
{
public:
    KisTagSP currentTag;

    KisTagChooserWidget *tagChooser;
    KisTagFilterWidget *tagFilter;

    QCompleter *tagCompleter;

    QPointer<KisTagFilterResourceProxyModel> model;

    KisTagModel* tagModel;
    KisResourceModel* resourceSourceModel;
};


KisResourceTaggingManager::KisResourceTaggingManager(QString resourceType, KisTagFilterResourceProxyModel *model, QWidget *parent)

    : QObject(parent)
    , d(new Private())
{
    d->model = model;

    d->tagModel = KisTagModelProvider::tagModel(resourceType);
    d->resourceSourceModel = KisResourceModelProvider::resourceModel(resourceType);
    d->tagChooser = new KisTagChooserWidget(d->tagModel, parent);
    d->tagFilter = new KisTagFilterWidget(d->tagModel, parent);

    connect(d->tagChooser, SIGNAL(tagChosen(KisTagSP)), this, SLOT(tagChooserIndexChanged(KisTagSP)));
    connect(d->tagChooser, SIGNAL(newTagRequested(KisTagSP)), this, SLOT(contextCreateNewTag(KisTagSP)));
    connect(d->tagChooser, SIGNAL(tagDeletionRequested(KisTagSP)), this, SLOT(removeTagFromComboBox(KisTagSP)));
    connect(d->tagChooser, SIGNAL(tagRenamingRequested(KisTagSP,KisTagSP)), this, SLOT(renameTag(KisTagSP,KisTagSP)));
    connect(d->tagChooser, SIGNAL(tagUndeletionRequested(KisTagSP)), this, SLOT(undeleteTag(KisTagSP)));
    connect(d->tagChooser, SIGNAL(tagUndeletionListPurgeRequested()), this, SLOT(purgeTagUndeleteList()));

    connect(d->tagFilter, SIGNAL(saveButtonClicked()), this, SLOT(tagSaveButtonPressed()));
    connect(d->tagFilter, SIGNAL(filterTextChanged(QString)), this, SLOT(tagSearchLineEditTextChanged(QString)));

    syncTagBoxEntries();
}

KisResourceTaggingManager::~KisResourceTaggingManager()
{
    delete d;
}

void KisResourceTaggingManager::showTaggingBar(bool show)
{
    show ? d->tagFilter->show() : d->tagFilter->hide();
    show ? d->tagChooser->show() : d->tagChooser->hide();
}

void KisResourceTaggingManager::purgeTagUndeleteList()
{
    ENTER_FUNCTION();
    //d->lastDeletedTag = TaggedResourceSet();
    //d->tagChooser->setUndeletionCandidate(QString());
}

void KisResourceTaggingManager::undeleteTag(const KisTagSP tagToUndelete)
{
    ENTER_FUNCTION();
    /*
    QString tagName = tagToUndelete;
    QStringList allTags = availableTags();

    if (allTags.contains(tagName)) {
        bool ok;
        tagName = QInputDialog::getText(
                    d->tagChooser, i18n("Unable to undelete tag"),
                    i18n("<qt>The tag you are trying to undelete already exists in tag list.<br>Please enter a new, unique name for it.</qt>"),
                    QLineEdit::Normal,
                    tagName, &ok);

        if (!ok || allTags.contains(tagName) || tagName.isEmpty()) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(i18n("Tag was not undeleted."));
            msgBox.exec();
            return;
        }
    }

    //    QList<KoResourceSP> serverResources = d->model->serverResources();

    //    Q_FOREACH(KoResourceSP resource, d->lastDeletedTag.resources) {
    //        if (serverResources.contains(resource)) {
    //            addResourceTag(resource, tagName);
    //        }
    //    }
    //    d->model->tagCategoryAdded(tagName);
    d->tagChooser->setCurrentIndex(d->tagChooser->findIndexOf(tagName));
    d->tagChooser->setUndeletionCandidate(QString());
    //    d->lastDeletedTag = TaggedResourceSet();
    */
}

void KisResourceTaggingManager::addResourceTag(KoResourceSP resource, const KisTagSP tag)
{
    d->tagModel->tagResource(tag, resource);
}

void KisResourceTaggingManager::syncTagBoxEntryAddition(const KisTagSP tag)
{
    ENTER_FUNCTION();
    //d->tagChooser->insertItem(tag);
}

void KisResourceTaggingManager::contextCreateNewTag(const KisTagSP tag)
{
    fprintf(stderr, "void KisResourceTaggingManager::contextCreateNewTag(const KisTagSP tag)");
    ENTER_FUNCTION();
    /*
    if (!tag.isEmpty()) {
//        d->model->addTag(0, tag);
//        d->model->tagCategoryAdded(tag);
        d->tagChooser->setCurrentIndex(d->tagChooser->findIndexOf(tag));
        updateTaggedResourceView();
    }
    */
}

void KisResourceTaggingManager::contextCreateNewTag(KoResourceSP resource , const KisTagSP tag)
{
    // TODO: RESOURCES: this function should use QString, not KisTagSP
    fprintf(stderr, "void KisResourceTaggingManager::contextCreateNewTag(KoResourceSP resource , const KisTagSP tag)");
    KisTagSP inserted = d->tagChooser->insertItem(tag);
    int previousIndex = d->tagChooser->currentIndex();
    d->tagModel->tagResource(inserted, resource);
    d->tagChooser->setCurrentIndex(previousIndex);
}

void KisResourceTaggingManager::syncTagBoxEntryRemoval(const KisTagSP tag)
{
    ENTER_FUNCTION();
    //d->tagChooser->removeItem(tag);
}

void KisResourceTaggingManager::syncTagBoxEntries()
{
    ENTER_FUNCTION();
    /*
    QList<KisTagSP> tags = d->tagModel->allTags();
    tags.sort();
    Q_FOREACH (const KisTagSP &tag, tags) {
        //d->tagChooser->insertItem(tag);
    }
    */
}

void KisResourceTaggingManager::contextAddTagToResource(KoResourceSP resource, const KisTagSP tag)
{
    fprintf(stderr, "void KisResourceTaggingManager::contextAddTagToResource(KoResourceSP resource, const KisTagSP tag)");
    ENTER_FUNCTION();
    addResourceTag(resource, tag);
    updateTaggedResourceView();
}

void KisResourceTaggingManager::contextRemoveTagFromResource(KoResourceSP resource, const KisTagSP tag)
{
    fprintf(stderr, "void KisResourceTaggingManager::contextRemoveTagFromResource(KoResourceSP resource, const KisTagSP tag)");
    ENTER_FUNCTION();
    removeResourceTag(resource, tag);
    updateTaggedResourceView();
}

void KisResourceTaggingManager::removeTagFromComboBox(const KisTagSP tag)
{
    fprintf(stderr, "void KisResourceTaggingManager::removeTagFromComboBox(const KisTagSP tag)");
    ENTER_FUNCTION();
    //    QList<KoResourceSP> resources = d->model->currentlyVisibleResources();
    //    Q_FOREACH (KoResourceSP resource, resources) {
    //        removeResourceTag(resource, tag);
    //    }
    //    d->model->tagCategoryRemoved(tag);
    //    d->lastDeletedTag = TaggedResourceSet(tag, resources);
    // d->tagChooser->setUndeletionCandidate(tag);
}

void KisResourceTaggingManager::removeResourceTag(KoResourceSP resource, const KisTagSP tag)
{
    ENTER_FUNCTION();
    int previousIndex = d->tagChooser->currentIndex();
    bool success = d->tagModel->untagResource(tag, resource);
    fprintf(stderr, "remove Resource tag: %d\n", success);
    d->tagChooser->setCurrentIndex(previousIndex);
}

void KisResourceTaggingManager::renameTag(const KisTagSP oldTag, const KisTagSP newName)
{
    ENTER_FUNCTION();
    //d->tagModel
    //    if (!d->model->tagNamesList().contains(newName)) {
    //        QList<KoResourceSP> resources = d->model->currentlyVisibleResources();

    //        Q_FOREACH (KoResourceSP resource, resources) {
    //            removeResourceTag(resource, oldName);
    //            addResourceTag(resource, newName);
    //        }
    //        contextCreateNewTag(newName);
    //        d->model->tagCategoryRemoved(oldName);
    //        d->model->tagCategoryAdded(newName);
    //    }
}

void KisResourceTaggingManager::updateTaggedResourceView()
{
    ENTER_FUNCTION();
    //    d->model->setCurrentTag(d->currentTag);
    //    d->model->updateServer();
    //    d->originalResources = d->model->currentlyVisibleResources();
    emit updateView();
}

void KisResourceTaggingManager::tagChooserIndexChanged(const KisTagSP tag)
{
    ENTER_FUNCTION();
    d->model->setTag(tag);
    d->currentTag = tag;
/*
    if (!d->tagChooser->selectedTagIsReadOnly()) {
        d->currentTag = d->tagChooser->currentlySelectedTag();
        d->tagFilter->allowSave(true);
        //        d->model->enableResourceFiltering(true);
    } else {
        //        d->model->enableResourceFiltering(false);
        d->tagFilter->allowSave(false);
        d->currentTag.clear();
    }
*/
    d->tagFilter->clear();
    d->tagFilter->allowSave(tag->id() >= 0); // disallow save if the chosen tag has negative id (i.e. 'All' tag)
    updateTaggedResourceView();
}

void KisResourceTaggingManager::tagSearchLineEditTextChanged(const QString& lineEditText)
{
    fprintf(stderr, "void KisResourceTaggingManager::tagSearchLineEditTextChanged(const QString& lineEditText): %s \n", lineEditText.toStdString().c_str());
    d->model->setSearchBoxText(lineEditText);
    ENTER_FUNCTION() << ppVar(lineEditText);
    emit updateView();
}

void KisResourceTaggingManager::tagSaveButtonPressed()
{
    fprintf(stderr, "void KisResourceTaggingManager::tagSaveButtonPressed()\n");

    int previousTagIndex = d->tagChooser->currentIndex();

    KisTagSP tag = d->tagChooser->currentlySelectedTag();

    // untag all previous resources
    int allResources = d->resourceSourceModel->rowCount();
    for (int i = 0; i < allResources; i++) {
        QModelIndex index = d->resourceSourceModel->index(i, 0);
        KoResourceSP resource = d->resourceSourceModel->resourceForIndex(index);
        QVector<KisTagSP> tags = d->resourceSourceModel->tagsForResource(resource->resourceId());
        QVector<KisTagSP>::iterator iter = std::find_if(tags.begin(), tags.end(), [tag](KisTagSP tagFromResource) { return tagFromResource->url() == tag->url(); });
        if (iter != tags.end()) {
            d->tagModel->untagResource(tag, resource);
        }
    }

    // tag all resources that are here now
    int rows = d->model->rowCount();
    for (int i = 0; i < rows; i++) {
        QModelIndex index = d->model->index(i, 0);
        KoResourceSP resource = d->model->resourceForIndex(index);
        if (!tag.isNull() && !resource.isNull()) {
            d->tagModel->tagResource(tag, resource);
        }
    }

    d->tagChooser->setCurrentIndex(previousTagIndex);

    ENTER_FUNCTION();
    updateTaggedResourceView();
}

void KisResourceTaggingManager::contextMenuRequested(KoResourceSP resource, QPoint pos)
{
    ENTER_FUNCTION();
    // No visible tag chooser usually means no intended tag interaction,
    // context menu makes no sense then either
    fprintf(stderr, "context menu requested!");

    if (!resource || !d->tagChooser->isVisible())
        return;

    KisResourceItemChooserContextMenu menu(resource, d->tagChooser->currentlySelectedTag());

    connect(&menu, SIGNAL(resourceTagAdditionRequested(KoResourceSP,const KisTagSP)),
            this, SLOT(contextAddTagToResource(KoResourceSP,const KisTagSP)));

    connect(&menu, SIGNAL(resourceTagRemovalRequested(KoResourceSP,const KisTagSP)),
            this, SLOT(contextRemoveTagFromResource(KoResourceSP,const KisTagSP)));

    connect(&menu, SIGNAL(resourceAssignmentToNewTagRequested(KoResourceSP,const KisTagSP)),
            this, SLOT(contextCreateNewTag(KoResourceSP,const KisTagSP)));
    menu.exec(pos);
}

KisTagChooserWidget *KisResourceTaggingManager::tagChooserWidget()
{
    return d->tagChooser;
}

KisTagFilterWidget *KisResourceTaggingManager::tagFilterWidget()
{
    return d->tagFilter;
}

