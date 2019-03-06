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
#include <KisTagFilterResourceProxyModel.h>

#include "KisTagFilterWidget.h"
#include "KisTagChooserWidget.h"
#include "KisResourceItemChooserContextMenu.h"


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
    QString currentTag;

    KisTagChooserWidget *tagChooser;
    KisTagFilterWidget *tagFilter;

    QCompleter *tagCompleter;

    QPointer<KisTagFilterResourceProxyModel> model;
};


KisResourceTaggingManager::KisResourceTaggingManager(KisTagFilterResourceProxyModel *model, QWidget *parent)
    : QObject(parent)
    , d(new Private())
{
    d->model = model;

    d->tagChooser = new KisTagChooserWidget(parent);
    d->tagChooser->addReadOnlyItem("All"); // not translatable until other tags made translatable!
    //d->tagChooser->addItems(d->model->tagNamesList());

    d->tagFilter = new KisTagFilterWidget(parent);

    connect(d->tagChooser, SIGNAL(tagChosen(QString)), this, SLOT(tagChooserIndexChanged(QString)));
    connect(d->tagChooser, SIGNAL(newTagRequested(QString)), this, SLOT(contextCreateNewTag(QString)));
    connect(d->tagChooser, SIGNAL(tagDeletionRequested(QString)), this, SLOT(removeTagFromComboBox(QString)));
    connect(d->tagChooser, SIGNAL(tagRenamingRequested(QString,QString)), this, SLOT(renameTag(QString,QString)));
    connect(d->tagChooser, SIGNAL(tagUndeletionRequested(QString)), this, SLOT(undeleteTag(QString)));
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
    //d->lastDeletedTag = TaggedResourceSet();
    d->tagChooser->setUndeletionCandidate(QString());
}

void KisResourceTaggingManager::undeleteTag(const QString & tagToUndelete)
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
}

QStringList KisResourceTaggingManager::availableTags() const
{
    return d->tagChooser->allTags();
}

void KisResourceTaggingManager::addResourceTag(KoResourceSP resource, const QString& tagName)
{
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

void KisResourceTaggingManager::syncTagBoxEntryAddition(const QString& tag)
{
    d->tagChooser->insertItem(tag);
}

void KisResourceTaggingManager::contextCreateNewTag(const QString& tag)
{
    if (!tag.isEmpty()) {
//        d->model->addTag(0, tag);
//        d->model->tagCategoryAdded(tag);
        d->tagChooser->setCurrentIndex(d->tagChooser->findIndexOf(tag));
        updateTaggedResourceView();
    }
}

void KisResourceTaggingManager::contextCreateNewTag(KoResourceSP resource , const QString& tag)
{
    if (!tag.isEmpty()) {
//        d->model->tagCategoryAdded(tag);
        if (resource) {
            addResourceTag(resource, tag);
        }
    }
}

void KisResourceTaggingManager::syncTagBoxEntryRemoval(const QString& tag)
{
    d->tagChooser->removeItem(tag);
}

void KisResourceTaggingManager::syncTagBoxEntries()
{
    QStringList tags; // = d->model->tagNamesList();
    tags.sort();
    Q_FOREACH (const QString &tag, tags) {
        d->tagChooser->insertItem(tag);
    }
}

void KisResourceTaggingManager::contextAddTagToResource(KoResourceSP resource, const QString& tag)
{
    addResourceTag(resource, tag);
    //    d->model->tagCategoryMembersChanged();
    updateTaggedResourceView();
}

void KisResourceTaggingManager::contextRemoveTagFromResource(KoResourceSP resource, const QString& tag)
{
    removeResourceTag(resource, tag);
    //    d->model->tagCategoryMembersChanged();
    updateTaggedResourceView();
}

void KisResourceTaggingManager::removeTagFromComboBox(const QString &tag)
{
    //    QList<KoResourceSP> resources = d->model->currentlyVisibleResources();
    //    Q_FOREACH (KoResourceSP resource, resources) {
    //        removeResourceTag(resource, tag);
    //    }
    //    d->model->tagCategoryRemoved(tag);
    //    d->lastDeletedTag = TaggedResourceSet(tag, resources);
    d->tagChooser->setUndeletionCandidate(tag);
}

void KisResourceTaggingManager::removeResourceTag(KoResourceSP resource, const QString& tagName)
{
    //    QStringList tagsList = d->model->assignedTagsList(resource);

    //    Q_FOREACH (const QString & oldName, tagsList) {
    //        if (!oldName.compare(tagName)) {
    //            d->model->deleteTag(resource, oldName);
    //        }
    //    }
}

void KisResourceTaggingManager::renameTag(const QString &oldName, const QString& newName)
{
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
    //    d->model->setCurrentTag(d->currentTag);
    //    d->model->updateServer();
    //    d->originalResources = d->model->currentlyVisibleResources();
    emit updateView();
}

void KisResourceTaggingManager::tagChooserIndexChanged(const QString& lineEditText)
{
    if (!d->tagChooser->selectedTagIsReadOnly()) {
        d->currentTag = lineEditText;
        d->tagFilter->allowSave(true);
        //        d->model->enableResourceFiltering(true);
    } else {
        //        d->model->enableResourceFiltering(false);
        d->tagFilter->allowSave(false);
        d->currentTag.clear();
    }

    d->tagFilter->clear();
    updateTaggedResourceView();
}

void KisResourceTaggingManager::tagSearchLineEditTextChanged(const QString& lineEditText)
{
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

void KisResourceTaggingManager::contextMenuRequested(KoResourceSP resource, const QStringList& resourceTags, const QPoint& pos)
{
    // No visible tag chooser usually means no intended tag interaction,
    // context menu makes no sense then either
    if (!resource || !d->tagChooser->isVisible())
        return;

    KisResourceItemChooserContextMenu menu(resource,
                                           resourceTags,
                                           d->tagChooser->currentlySelectedTag(),
                                           d->tagChooser->allTags());

    connect(&menu, SIGNAL(resourceTagAdditionRequested(KoResourceSP,QString)),
            this, SLOT(contextAddTagToResource(KoResourceSP,QString)));

    connect(&menu, SIGNAL(resourceTagRemovalRequested(KoResourceSP,QString)),
            this, SLOT(contextRemoveTagFromResource(KoResourceSP,QString)));

    connect(&menu, SIGNAL(resourceAssignmentToNewTagRequested(KoResourceSP,QString)),
            this, SLOT(contextCreateNewTag(KoResourceSP,QString)));
    menu.exec(pos);
}

void KisResourceTaggingManager::contextMenuRequested(KoResourceSP currentResource, QPoint pos)
{
//    if (currentResource) {
//        contextMenuRequested(currentResource, d->model->assignedTagsList(currentResource), pos);
//    }
}

KisTagChooserWidget *KisResourceTaggingManager::tagChooserWidget()
{
    return d->tagChooser;
}

QLineEdit *KisResourceTaggingManager::searchField()
{
    return d->tagFilter->searchField();
}

KisTagFilterWidget *KisResourceTaggingManager::tagFilterWidget()
{
    return d->tagFilter;
}

