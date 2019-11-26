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
    d->tagFilter = new KisTagFilterWidget(parent);



    d->tagModel = KisTagModelProvider::tagModel(resourceType);
    d->resourceSourceModel = KisResourceModelProvider::resourceModel(resourceType);
    d->tagChooser = new KisTagChooserWidget(d->tagModel, parent);

    connect(d->tagChooser, SIGNAL(tagChosen(KisTagSP)), this, SLOT(tagChooserIndexChanged(KisTagSP)));
    connect(d->tagChooser, SIGNAL(newTagRequested(KisTagSP)), this, SLOT(contextCreateNewTag(KisTagSP)));
    connect(d->tagChooser, SIGNAL(tagDeletionRequested(KisTagSP)), this, SLOT(removeTagFromComboBox(KisTagSP)));
    connect(d->tagChooser, SIGNAL(tagRenamingRequested(KisTagSP,KisTagSP)), this, SLOT(renameTag(KisTagSP,KisTagSP)));
    connect(d->tagChooser, SIGNAL(tagUndeletionRequested(KisTagSP)), this, SLOT(undeleteTag(KisTagSP)));
    connect(d->tagChooser, SIGNAL(tagUndeletionListPurgeRequested()), this, SLOT(purgeTagUndeleteList()));

    connect(d->tagFilter, SIGNAL(saveButtonClicked()), this, SLOT(tagSaveButtonPressed()));
    connect(d->tagFilter, SIGNAL(filterTextChanged(QString)), this, SLOT(tagSearchLineEditTextChanged(QString)));

//    connect(d->model, SIGNAL(tagBoxEntryAdded(QString)), this, SLOT(syncTagBoxEntryAddition(QString)));
//    connect(d->model, SIGNAL(tagBoxEntryRemoved(QString)), this, SLOT(syncTagBoxEntryRemoval(QString)));
//    connect(d->model, SIGNAL(tagBoxEntryModified()), this, SLOT(syncTagBoxEntries()));

    // FIXME: fix tag completer
    // d->tagCompleter = new QCompleter(this);
    //  d->tagSearchLineEdit->setCompleter(d->tagCompleter);

    syncTagBoxEntries();
}

KisResourceTaggingManager::~KisResourceTaggingManager()
{
    delete d;
}

void KisResourceTaggingManager::showTaggingBar(bool show)
{
    ENTER_FUNCTION();
    show ? d->tagFilter->show() : d->tagFilter->hide();
    show ? d->tagChooser->show() : d->tagChooser->hide();

//    blockSignals(!show);

//    QString tag("All");
//    if (show) {
//        KConfigGroup group =  KSharedConfig::openConfig()->group("SelectedTags");
//        tag = group.readEntry<QString>(d->model->serverType(), "All");
//    }
//    int idx = d->tagChooser->findIndexOf(tag);
//    if (idx < 0) idx = 0;
//    d->tagChooser->setCurrentIndex(idx);
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

QStringList KisResourceTaggingManager::availableTags() const
{
    ENTER_FUNCTION();
    return QStringList();
    //return d->tagChooser->allTags();
}

void KisResourceTaggingManager::addResourceTag(KoResourceSP resource, const KisTagSP tag)
{
    ENTER_FUNCTION();
    fprintf(stderr, "trying to tag a resource... %s %s\n", resource->name().toUtf8().toStdString().c_str(), tag->name().toUtf8().toStdString().c_str());
    d->tagModel->tagResource(tag, resource);
    //d->tagModels->tagModel(resource->resourceType())->tagResource(tag, resource);
    // we need to find a tag from a tagName?
    // or...

    //    QStringList tagsList = d->model->assignedTagsList(resource);
    //    if (tagsList.isEmpty()) {
    //        d->model->addTag(resource, tagName);
    //    } else {
    //        Q_FOREACH (const QString & tag, tagsList) {
    //            if (tag.compare(tagName)) {
    //                d->model->addTag(resource, tagName);
    //            }
    //        }
    //    }
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
    fprintf(stderr, "void KisResourceTaggingManager::contextCreateNewTag(KoResourceSP resource , const KisTagSP tag)");
    ENTER_FUNCTION();
    /*
    if (!tag.isEmpty()) {
//        d->model->tagCategoryAdded(tag);
        if (resource) {
            addResourceTag(resource, tag);
        }
    }
    */
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
    //    d->model->tagCategoryMembersChanged();
    updateTaggedResourceView();
}

void KisResourceTaggingManager::contextRemoveTagFromResource(KoResourceSP resource, const KisTagSP tag)
{
    fprintf(stderr, "void KisResourceTaggingManager::contextRemoveTagFromResource(KoResourceSP resource, const KisTagSP tag)");
    ENTER_FUNCTION();
    removeResourceTag(resource, tag);
    //    d->model->tagCategoryMembersChanged();
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
    bool success = d->tagModel->untagResource(tag, resource);
    fprintf(stderr, "remove Resource tag: %d\n", success);

    //    QStringList tagsList = d->model->assignedTagsList(resource);

    //    Q_FOREACH (const QString & oldName, tagsList) {
    //        if (!oldName.compare(tagName)) {
    //            d->model->deleteTag(resource, oldName);
    //        }
    //    }
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
    updateTaggedResourceView();
}

void KisResourceTaggingManager::tagSearchLineEditTextChanged(const QString& lineEditText)
{
    ENTER_FUNCTION() << ppVar(lineEditText);
    //    if (d->tagChooser->selectedTagIsReadOnly()) {
    //        d->model->enableResourceFiltering(!lineEditText.isEmpty());
    //    } else {
    //        d->model->enableResourceFiltering(true);
    //    }

    //    d->model->searchTextChanged(lineEditText);
    //    d->model->updateServer();

    //FIXME: fix completer
    //      d->tagCompleter = new QCompleter(tagNamesList(lineEditText),this);
    //      d->tagSearchLineEdit->setCompleter(d->tagCompleter);

    emit updateView();
}

void KisResourceTaggingManager::tagSaveButtonPressed()
{
    ENTER_FUNCTION();
    //    if (!d->tagChooser->selectedTagIsReadOnly()) {
    //        QList<KoResourceSP> newResources = d->model->currentlyVisibleResources();
    //        Q_FOREACH (KoResourceSP oldRes, d->originalResources) {
    //            if (!newResources.contains(oldRes))
    //                removeResourceTag(oldRes, d->currentTag);
    //        }
    //        Q_FOREACH (KoResourceSP newRes, newResources) {
    //            if (!d->originalResources.contains(newRes))
    //                addResourceTag(newRes, d->currentTag);
    //        }
    //        d->model->tagCategoryMembersChanged();
    //    }
    updateTaggedResourceView();
}

void KisResourceTaggingManager::contextMenuRequested(KoResourceSP resource, const QList<KisTagSP> resourceTags, const QPoint& pos)
{
    ENTER_FUNCTION();
    // No visible tag chooser usually means no intended tag interaction,
    // context menu makes no sense then either
    fprintf(stderr, "context menu requested!");

    if (!resource || !d->tagChooser->isVisible())
        return;

    KisResourceItemChooserContextMenu menu(resource,
                                           resourceTags,
                                           d->tagChooser->currentlySelectedTag(),
                                           d->tagChooser->allTags());

    connect(&menu, SIGNAL(resourceTagAdditionRequested(KoResourceSP,const KisTagSP)),
            this, SLOT(contextAddTagToResource(KoResourceSP,const KisTagSP)));

    connect(&menu, SIGNAL(resourceTagRemovalRequested(KoResourceSP,const KisTagSP)),
            this, SLOT(contextRemoveTagFromResource(KoResourceSP,const KisTagSP)));

    connect(&menu, SIGNAL(resourceAssignmentToNewTagRequested(KoResourceSP,const KisTagSP)),
            this, SLOT(contextCreateNewTag(KoResourceSP,const KisTagSP)));
    menu.exec(pos);
}

void KisResourceTaggingManager::contextMenuRequested(KoResourceSP currentResource, QPoint pos)
{
    ENTER_FUNCTION();
    if (currentResource) {
        contextMenuRequested(currentResource, d->tagModel->tagsForResource(currentResource->resourceId()).toList(), pos);
    }

}

KisTagChooserWidget *KisResourceTaggingManager::tagChooserWidget()
{
    ENTER_FUNCTION();
    return d->tagChooser;
}

KisTagFilterWidget *KisResourceTaggingManager::tagFilterWidget()
{
    ENTER_FUNCTION();
    return d->tagFilter;
}

