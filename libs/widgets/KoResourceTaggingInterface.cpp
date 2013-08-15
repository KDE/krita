/*
 *    This file is part of the KDE project
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */


#include <QInputDialog>
#include <QMessageBox>

#include <kdebug.h>

#include <klocale.h>

#include "KoResourceTaggingInterface.h"
#include "KoResourceModel.h"
#include "KoResource.h"
#include "KoResourceItemChooserContextMenu.h"

class TaggedResourceSet
{
public:
    TaggedResourceSet()
    {}

    TaggedResourceSet(const QString& tagName, const QList<KoResource*>& resources)
        :tagName(tagName), resources(resources)
    {}

    QString tagName;
    QList<KoResource*> resources;
};


class KoResourceTaggingInterface::Private
{
public:
    QString currentTag;
    QString unfilteredView;
    QList<KoResource*> originalResources;
    TaggedResourceSet lastDeletedTag;

    KoTagChooserWidget* tagChooser;
    KoTagFilterWidget* tagFilter;

    QCompleter* tagCompleter;

    KoResourceModel* model;
};

void KoResourceTaggingInterface::showTaggingBar(bool showSearchBar, bool showOpBar)
{
    showSearchBar ? d->tagFilter->show() : d->tagFilter->hide();
    showOpBar ? d->tagChooser->show() : d->tagChooser->hide();
}

void KoResourceTaggingInterface::purgeTagUndeleteList()
{
    d->lastDeletedTag = TaggedResourceSet();
    d->tagChooser->setUndeletionCandidate(QString());
}

void KoResourceTaggingInterface::undeleteTag(const QString & tagToUndelete)
{
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

    QList<KoResource*> serverResources = d->model->serverResources();

    foreach(KoResource* resource, d->lastDeletedTag.resources) {
        if (serverResources.contains(resource)) {
            addResourceTag(resource,tagName);
        }
    }
    d->model->tagCategoryAdded(tagName);
    d->tagChooser->setCurrentIndex(d->tagChooser->findIndexOf(tagName));
    d->tagChooser->setUndeletionCandidate(QString());
    d->lastDeletedTag = TaggedResourceSet();
}

QStringList KoResourceTaggingInterface::availableTags() const
{
    return d->tagChooser->allTags();
}

void KoResourceTaggingInterface::addResourceTag(KoResource* resource, const QString& tagName)
{
    QStringList tagsList = d->model->assignedTagsList(resource);
    if (tagsList.isEmpty()) {
        d->model->addTag(resource, tagName);
    }
    else {
        foreach(const QString &tag, tagsList) {
            if(tag.compare(tagName)) {
                d->model->addTag(resource, tagName);
            }
        }
    }
}

void KoResourceTaggingInterface::syncTagBoxEntryAddition(const QString& tag)
{
    d->tagChooser->insertItem(tag);
}

void KoResourceTaggingInterface::contextCreateNewTag(const QString& tag)
{
    if (!tag.isEmpty()) {
        d->model->tagCategoryAdded(tag);
        d->tagChooser->setCurrentIndex(d->tagChooser->findIndexOf(tag));
    }
}

void KoResourceTaggingInterface::contextCreateNewTag(KoResource* resource , const QString& tag)
{
    if (!tag.isEmpty()) {
        d->model->tagCategoryAdded(tag);
        if (resource) {
            addResourceTag(resource, tag);
        }
    }
}

void KoResourceTaggingInterface::syncTagBoxEntryRemoval(const QString& tag)
{
    d->tagChooser->removeItem(tag);
}

void KoResourceTaggingInterface::contextAddTagToResource(KoResource* resource, const QString& tag)
{
    addResourceTag(resource, tag);
    d->model->tagCategoryMembersChanged();
    updateTaggedResourceView();
}

void KoResourceTaggingInterface::contextRemoveTagFromResource(KoResource* resource, const QString& tag)
{
    removeResourceTag(resource, tag);
    d->model->tagCategoryMembersChanged();
    updateTaggedResourceView();
}

void KoResourceTaggingInterface::removeTagFromComboBox(const QString &tag)
{
    QList<KoResource*> resources = d->model->currentlyVisibleResources();
    foreach(KoResource* resource, resources) {
        removeResourceTag(resource, tag);
    }
    d->model->tagCategoryRemoved(tag);
    d->lastDeletedTag = TaggedResourceSet(tag,resources);
    d->tagChooser->setUndeletionCandidate(tag);
}

void KoResourceTaggingInterface::removeResourceTag(KoResource* resource, const QString& tagName)
{
    QStringList tagsList = d->model->assignedTagsList(resource);

    foreach(const QString &oldName, tagsList) {
        if(!oldName.compare(tagName)) {
            d->model->deleteTag(resource, oldName);
        }
    }
}

void KoResourceTaggingInterface::renameTag(const QString &oldName, const QString& newName)
{
    if (!d->model->tagNamesList().contains(newName)) {
        QList<KoResource*> resources = d->model->currentlyVisibleResources();

        foreach(KoResource* resource, resources) {
            removeResourceTag(resource, oldName);
            addResourceTag(resource,newName);
        }
        contextCreateNewTag(newName);
        d->model->tagCategoryRemoved(oldName);
        d->model->tagCategoryAdded(newName);
    }
}

void KoResourceTaggingInterface::updateTaggedResourceView()
{
    d->model->setCurrentTag(d->currentTag);
    d->model->updateServer();
    d->originalResources = d->model->currentlyVisibleResources();
}

void KoResourceTaggingInterface::tagChooserIndexChanged(const QString& lineEditText)
{
    if ( !d->tagChooser->selectedTagIsReadOnly()) {
        d->currentTag = lineEditText;
        d->tagFilter->allowSave(true);
        d->model->enableResourceFiltering(true);
    }
    else {
        d->model->enableResourceFiltering(false);
        d->tagFilter->allowSave(false);
        d->currentTag.clear();
    }

    d->tagFilter->clear();
    updateTaggedResourceView();
}

QString KoResourceTaggingInterface::currentTag()
{
    return d->tagChooser->currentlySelectedTag();
}

void KoResourceTaggingInterface::tagSearchLineEditTextChanged(const QString& lineEditText)
{
    d->model->searchTextChanged(lineEditText);
    d->model->updateServer();
    ///FIXME: fix completer
    //     d->tagCompleter = new QCompleter(tagNamesList(lineEditText),this);
    //    d->tagSearchLineEdit->setCompleter(d->tagCompleter);
    if (d->tagChooser->selectedTagIsReadOnly()) {
        d->model->enableResourceFiltering(!lineEditText.isEmpty());
    }
    else {
        d->model->enableResourceFiltering(true);
    }
}

void KoResourceTaggingInterface::tagSaveButtonPressed()
{
    if (!d->tagChooser->selectedTagIsReadOnly()) {
        QList<KoResource*> newResources = d->model->currentlyVisibleResources();
        foreach(KoResource* oldRes, d->originalResources) {
            if (!newResources.contains(oldRes))
                removeResourceTag(oldRes, d->currentTag);
        }
        foreach(KoResource* newRes, newResources) {
            if (!d->originalResources.contains(newRes))
                addResourceTag(newRes, d->currentTag);
        }
        d->model->tagCategoryMembersChanged();
    }
    updateTaggedResourceView();
}

void KoResourceTaggingInterface::contextMenuRequested (KoResource* resource, const QStringList& resourceTags, const QPoint& pos)
{
    /* no visible tag chooser usually means no intended tag interaction,
     * context menu makes no sense then either */
    if (!resource || !d->tagChooser->isVisible())
        return;

    KoResourceItemChooserContextMenu menu(
            resource,
            resourceTags,
            d->tagChooser->currentlySelectedTag(),
            d->tagChooser->allTags());

    connect(&menu, SIGNAL(resourceTagAdditionRequested(KoResource*,QString)),
            this, SLOT(contextAddTagToResource(KoResource*,QString)));

    connect(&menu, SIGNAL(resourceTagRemovalRequested(KoResource*,QString)),
            this, SLOT(contextRemoveTagFromResource(KoResource*,QString)));

    connect(&menu, SIGNAL(resourceAssignmentToNewTagRequested(KoResource*,QString)),
            this, SLOT(contextCreateNewTag(KoResource*,QString)));
    menu.exec(pos);
}

void KoResourceTaggingInterface::contextMenuRequested(KoResource* currentResource, QPoint pos)
{
    if(currentResource) {
       contextMenuRequested(currentResource, d->model->assignedTagsList(currentResource), pos);
    }
}

KoTagChooserWidget* KoResourceTaggingInterface::tagChooserWidget()
{
    return d->tagChooser;
}

KoTagFilterWidget* KoResourceTaggingInterface::tagFilterWidget()
{
    return d->tagFilter;
}

KoResourceTaggingInterface::KoResourceTaggingInterface(KoResourceModel* model, QWidget* parent)
    : d(new Private())
{

    d->model = model;
    d->unfilteredView = i18n("All Presets");

    d->tagChooser = new KoTagChooserWidget(parent);
    d->tagChooser->addReadOnlyItem(d->unfilteredView);
    d->tagChooser->addItems(d->model->tagNamesList());

    d->tagFilter = new KoTagFilterWidget(parent);

    connect(d->tagChooser, SIGNAL(tagChosen(QString)),
            this, SLOT(tagChooserIndexChanged(QString)));
    connect(d->tagChooser, SIGNAL(newTagRequested(QString)),
            this, SLOT(contextCreateNewTag(QString)));
    connect(d->tagChooser, SIGNAL(tagDeletionRequested(QString)),
            this, SLOT(removeTagFromComboBox(QString)));
    connect(d->tagChooser, SIGNAL(tagRenamingRequested(QString,QString)),
            this, SLOT(renameTag(QString,QString)));
    connect(d->tagChooser, SIGNAL(tagUndeletionRequested(QString)),
            this, SLOT(undeleteTag(QString)));
    connect(d->tagChooser, SIGNAL(tagUndeletionListPurgeRequested()),
            this, SLOT(purgeTagUndeleteList()));

    connect(d->tagFilter, SIGNAL(saveButtonClicked()),
            this, SLOT(tagSaveButtonPressed()));
    connect(d->tagFilter, SIGNAL(filterTextChanged(QString)),
            this, SLOT(tagSearchLineEditTextChanged(QString)));

    connect(d->model, SIGNAL(tagBoxEntryAdded(QString)),
            this, SLOT(syncTagBoxEntryAddition(QString)));
    connect(d->model, SIGNAL(tagBoxEntryRemoved(QString)),
            this, SLOT(syncTagBoxEntryRemoval(QString)));

    /// FIXME: fix tag completer
    /// d->tagCompleter = new QCompleter(this);
    ///  d->tagSearchLineEdit->setCompleter(d->tagCompleter);

}
