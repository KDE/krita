/*
 *    This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *    SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *    SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
 *    SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisTagChooserWidget.h"

#include <QDebug>
#include <QToolButton>
#include <QGridLayout>
#include <QComboBox>
#include <QMessageBox>

#include <klocalizedstring.h>
#include <KisSqueezedComboBox.h>

#include <KoIcon.h>

#include "KisResourceItemChooserContextMenu.h"
#include "KisTagToolButton.h"
#include "kis_debug.h"
#include <KisTagResourceModel.h>

class Q_DECL_HIDDEN KisTagChooserWidget::Private
{
public:
    QComboBox *comboBox;
    KisTagToolButton *tagToolButton;
    KisTagModel *model;
    KisTagSP cachedTag;
    QString resourceType;
};

KisTagChooserWidget::KisTagChooserWidget(KisTagModel *model, QString resourceType, QWidget* parent)
    : QWidget(parent)
    , d(new Private)
{
    d->resourceType = resourceType;

    d->comboBox = new QComboBox(this);

    d->comboBox->setToolTip(i18n("Tag"));
    d->comboBox->setSizePolicy(QSizePolicy::MinimumExpanding , QSizePolicy::Fixed);
    d->comboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
    model->sort(KisAllTagsModel::Name);
    d->comboBox->setModel(model);

    d->model = model;

    QGridLayout* comboLayout = new QGridLayout(this);

    comboLayout->addWidget(d->comboBox, 0, 0);

    d->tagToolButton = new KisTagToolButton(this);
    d->tagToolButton->setToolTip(i18n("Tag options"));
    comboLayout->addWidget(d->tagToolButton, 0, 1);

    comboLayout->setSpacing(0);
    comboLayout->setMargin(0);
    comboLayout->setColumnStretch(0, 3);
    this->setEnabled(true);

    connect(d->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(tagChanged(int)));

    connect(d->tagToolButton, SIGNAL(popupMenuAboutToShow()),
            this, SLOT (tagToolContextMenuAboutToShow()));

    connect(d->tagToolButton, SIGNAL(newTagRequested(QString)),
            this, SLOT(addTag(QString)));

    connect(d->tagToolButton, SIGNAL(deletionOfCurrentTagRequested()),
            this, SLOT(tagToolDeleteCurrentTag()));

    connect(d->tagToolButton, SIGNAL(renamingOfCurrentTagRequested(const QString&)),
            this, SLOT(tagToolRenameCurrentTag(const QString&)));

    connect(d->tagToolButton, SIGNAL(undeletionOfTagRequested(KisTagSP)),
            this, SLOT(tagToolUndeleteLastTag(KisTagSP)));


    // Workaround for handling tag selection deselection when model resets.
    // Occurs when model changes under the user e.g. +/- a resource storage.
    connect(d->model, SIGNAL(modelAboutToBeReset()), this, SLOT(cacheSelectedTag()));
    connect(d->model, SIGNAL(modelReset()), this, SLOT(restoreTagFromCache()));

}

KisTagChooserWidget::~KisTagChooserWidget()
{
    delete d;
}

void KisTagChooserWidget::tagToolDeleteCurrentTag()
{
    KisTagSP currentTag = currentlySelectedTag();
    if (!currentTag.isNull() && currentTag->id() >= 0) {
        d->model->setTagInactive(currentTag);
        setCurrentIndex(0);
        d->tagToolButton->setUndeletionCandidate(currentTag);
        d->model->sort(KisAllTagsModel::Name);
    }
}

void KisTagChooserWidget::tagChanged(int tagIndex)
{
    if (tagIndex >= 0) {
        KisTagSP tag = currentlySelectedTag();
        d->tagToolButton->setCurrentTag(tag);
        d->model->sort(KisAllTagsModel::Name);
        emit sigTagChosen(tag);
    }
}

void KisTagChooserWidget::tagToolRenameCurrentTag(const QString& tagName)
{
    KisTagSP tag = currentlySelectedTag();
    bool canRenameCurrentTag = !tag.isNull();

    if (canRenameCurrentTag && !tagName.isEmpty()) {
        tag->setName(tagName);
        bool result = d->model->renameTag(tag);
        Q_ASSERT(result);
        d->model->sort(KisAllTagsModel::Name);
    }
}

void KisTagChooserWidget::tagToolUndeleteLastTag(KisTagSP tag)
{
    int previousIndex = d->comboBox->currentIndex();

    bool success = d->model->setTagActive(tag);
    setCurrentIndex(previousIndex);
    if (success) {
        d->tagToolButton->setUndeletionCandidate(KisTagSP());
        setCurrentItem(tag->name());
        d->model->sort(KisAllTagsModel::Name);
    }
}

void KisTagChooserWidget::cacheSelectedTag()
{
    d->cachedTag = currentlySelectedTag();
}

void KisTagChooserWidget::restoreTagFromCache()
{
    if (d->cachedTag) {
        QModelIndex cachedIndex = d->model->indexForTag(d->cachedTag);
        setCurrentIndex(cachedIndex.row());
        d->cachedTag = nullptr;
    }
}

void KisTagChooserWidget::setCurrentIndex(int index)
{
    d->comboBox->setCurrentIndex(index);
}

int KisTagChooserWidget::currentIndex() const
{
    return d->comboBox->currentIndex();
}

void KisTagChooserWidget::setCurrentItem(const QString &tag)
{
    for (int i = 0; i < d->model->rowCount(); i++) {
        QModelIndex index = d->model->index(i, 0);
        QString currentRowTag = d->model->data(index, Qt::UserRole + KisAllTagsModel::Url).toString();
        if (currentRowTag == tag) {
            setCurrentIndex(i);
        }
    }
}

void KisTagChooserWidget::addTag(const QString &tag)
{
    addTag(tag, 0);
}

KisTagChooserWidget::OverwriteDialogOptions KisTagChooserWidget::overwriteTagDialog(KisTagChooserWidget* parent, bool tagIsActive)
{
    QString undeleteOption = !tagIsActive ? i18nc("Option in a dialog to undelete (reactivate) existing tag with its old assigned resources", "Restore previous tag")
                                      : i18nc("Option in a dialog to use existing tag with its old assigned resources", "Use existing tag");
    // if you use this simple cast, the order of buttons must match order of options in the enum
    return (KisTagChooserWidget::OverwriteDialogOptions)QMessageBox::question(parent, i18nc("Dialog title", "Overwrite tag?"), i18nc("Question to the user in a dialog about creating a tag",
                                                                                      "A tag with this unique name already exists. Do you want to replace it?"),
                                       i18nc("Option in a dialog to discard the previously existing tag and creating a new one in its place", "Replace (overwrite) tag"),
                                       undeleteOption, i18n("Cancel"));
}

void KisTagChooserWidget::addTag(const QString &tagName, KoResourceSP resource)
{
    KisTagSP tagForUrl = d->model->tagForUrl(tagName);
    if (!tagForUrl.isNull()) {
        int response = overwriteTagDialog(this, tagForUrl->active());
        if (response == Undelete) { // Undelete
            d->model->setTagActive(tagForUrl);
            KisTagResourceModel(d->resourceType).tagResource(tagForUrl, resource->resourceId());
            d->model->sort(KisAllTagsModel::Name);
            return;
        } else if (response == Cancel) { // Cancel
            return;
        }
    }
    d->model->addTag(tagName, true, {resource}); // this will overwrite the tag
    d->model->sort(KisAllTagsModel::Name);
}

void KisTagChooserWidget::addTag(KisTagSP tag, KoResourceSP resource)
{
    KisTagSP tagForUrl = d->model->tagForUrl(tag->url());
    if (!tagForUrl.isNull()) {
        int response = overwriteTagDialog(this, tagForUrl->active());
        if (response == Undelete) { // Undelete
            d->model->setTagActive(tagForUrl);
            KisTagResourceModel(d->resourceType).tagResource(tagForUrl, resource->resourceId());
            d->model->sort(KisAllTagsModel::Name);
            return;
        } else if (response == Cancel) { // Cancel
            return;
        }
    }
    d->model->addTag(tag, true, {resource}); // this will overwrite the tag
    d->model->sort(KisAllTagsModel::Name);
}

KisTagSP KisTagChooserWidget::currentlySelectedTag()
{
    int row = d->comboBox->currentIndex();
    if (row < 0) {
        return nullptr;
    }

    QModelIndex index = d->model->index(row, 0);
    KisTagSP tag =  d->model->tagForIndex(index);
    return tag;
}

void KisTagChooserWidget::updateIcons()
{
    d->tagToolButton->loadIcon();
}

void KisTagChooserWidget::tagToolContextMenuAboutToShow()
{
    /* only enable the save button if the selected tag set is editable */
    if (currentlySelectedTag()) {
        d->tagToolButton->readOnlyMode(currentlySelectedTag()->id() < 0);
    }
    else {
        d->tagToolButton->readOnlyMode(true);
    }
}
