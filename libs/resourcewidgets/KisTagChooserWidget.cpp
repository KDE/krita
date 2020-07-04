/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
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

#include "KisTagChooserWidget.h"

#include <QDebug>
#include <QToolButton>
#include <QGridLayout>
#include <QComboBox>

#include <klocalizedstring.h>
#include <KisSqueezedComboBox.h>

#include <KoIcon.h>

#include "KisResourceItemChooserContextMenu.h"
#include "KisTagToolButton.h"
#include "kis_debug.h"
#include <KisActiveFilterTagProxyModel.h>

class Q_DECL_HIDDEN KisTagChooserWidget::Private
{
public:
    QComboBox *comboBox;
    KisTagToolButton *tagToolButton;
    KisTagModel* model;
    QScopedPointer<KisActiveFilterTagProxyModel> activeFilterModel;
    KisTagSP rememberedTag;

    Private(KisTagModel* model)
        : activeFilterModel(new KisActiveFilterTagProxyModel(0))
    {
        activeFilterModel->setSourceModel(model);
    }
};

KisTagChooserWidget::KisTagChooserWidget(KisTagModel* model, QWidget* parent)
    : QWidget(parent)
    , d(new Private(model))
{
    d->comboBox = new QComboBox(this);

    d->comboBox->setToolTip(i18n("Tag"));
    d->comboBox->setSizePolicy(QSizePolicy::MinimumExpanding , QSizePolicy::Fixed );

    d->comboBox->setModel(d->activeFilterModel.data());

    d->model = model;

    QGridLayout* comboLayout = new QGridLayout(this);

    comboLayout->addWidget(d->comboBox, 0, 0);

    d->tagToolButton = new KisTagToolButton(this);
    comboLayout->addWidget(d->tagToolButton, 0, 1);

    comboLayout->setSpacing(0);
    comboLayout->setMargin(0);
    comboLayout->setColumnStretch(0, 3);
    this->setEnabled(true);

    connect(d->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(tagChanged(int)));

    connect(d->tagToolButton, SIGNAL(popupMenuAboutToShow()),
            this, SLOT (tagToolContextMenuAboutToShow()));

    connect(d->tagToolButton, SIGNAL(newTagRequested(KisTagSP)),
            this, SLOT(tagToolCreateNewTag(KisTagSP)));

    connect(d->tagToolButton, SIGNAL(deletionOfCurrentTagRequested()),
            this, SLOT(tagToolDeleteCurrentTag()));

    connect(d->tagToolButton, SIGNAL(renamingOfCurrentTagRequested(KisTagSP)),
            this, SLOT(tagToolRenameCurrentTag(KisTagSP)));
    connect(d->tagToolButton, SIGNAL(undeletionOfTagRequested(KisTagSP)),
            this, SLOT(tagToolUndeleteLastTag(KisTagSP)));

    connect(d->model, SIGNAL(modelAboutToBeReset()), this, SLOT(slotModelAboutToBeReset()));
    connect(d->model, SIGNAL(modelReset()), this, SLOT(slotModelReset()));


}

KisTagChooserWidget::~KisTagChooserWidget()
{
    delete d;
}

void KisTagChooserWidget::tagToolDeleteCurrentTag()
{
    KisTagSP currentTag = currentlySelectedTag();
    if (!currentTag.isNull() && currentTag->id() >= 0) {
        d->model->removeTag(currentTag);
        setCurrentIndex(0);
        d->tagToolButton->setUndeletionCandidate(currentTag);
    }
}

void KisTagChooserWidget::tagChanged(int tagIndex)
{
    if (tagIndex >= 0) {
        emit sigTagChosen(currentlySelectedTag());
    }
}

void KisTagChooserWidget::tagToolRenameCurrentTag(const KisTagSP newName)
{
    // TODO: RESOURCES: it should use QString, not KisTagSP
    KisTagSP currentTag = currentlySelectedTag();
    QString name = newName.isNull() ? "" : newName->name();
    bool canRenameCurrentTag = !currentTag.isNull() && currentTag->id() < 0;
    if (canRenameCurrentTag && !name.isEmpty()) {
        d->model->renameTag(currentTag, newName->name());
    }
}

void KisTagChooserWidget::tagToolUndeleteLastTag(const KisTagSP tag)
{
    int previousIndex = d->comboBox->currentIndex();
    bool success = d->model->changeTagActive(tag, true);
    setCurrentIndex(previousIndex);
    if (success) {
        d->tagToolButton->setUndeletionCandidate(KisTagSP());
        setCurrentItem(tag);
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

bool KisTagChooserWidget::setCurrentItem(KisTagSP tag)
{
    for (int i = 0; i < d->model->rowCount(); i++) {
        QModelIndex index = d->model->index(i, 0);
        KisTagSP temp = d->model->tagForIndex(index);
        if (!temp.isNull() && temp->url() == tag->url()) {
            setCurrentIndex(i);
            return true;
        }
    }
    return false;
}

KisTagSP KisTagChooserWidget::tagToolCreateNewTag(KisTagSP tag)
{
    // TODO: RESOURCES: this function should use QString, not KisTagSP
    int previous = d->comboBox->currentIndex();

    if(tag.isNull() || tag->name().isNull() || tag->name().isEmpty()) {
        return KisTagSP();
    }

    tag->setUrl(tag->name());
    tag->setComment(tag->name());
    tag->setActive(true);
    tag->setValid(true);
    bool added = d->model->addTag(tag);

    if (added) {
        bool found = setCurrentItem(tag);
        if (found) {
            return currentlySelectedTag();
        } else {
            return KisTagSP();
        }
    }

    setCurrentIndex(previous);
    return KisTagSP();
}

KisTagSP KisTagChooserWidget::currentlySelectedTag()
{
    int row = d->comboBox->currentIndex();
    if (row < 0) {
        return KisTagSP();
    }

    QModelIndex index = d->model->index(row, 0);
    KisTagSP tag =  d->model->tagForIndex(index);
    return tag;
}

bool KisTagChooserWidget::selectedTagIsReadOnly()
{
    return currentlySelectedTag()->id() < 0;
}

void KisTagChooserWidget::tagToolContextMenuAboutToShow()
{
    /* only enable the save button if the selected tag set is editable */
    d->tagToolButton->readOnlyMode(selectedTagIsReadOnly());
}

void KisTagChooserWidget::slotModelAboutToBeReset()
{
    d->rememberedTag = currentlySelectedTag();
}

void KisTagChooserWidget::slotModelReset()
{
    bool selected = setCurrentItem(d->rememberedTag);
    if (!selected) {
        setCurrentIndex(0); // last used tag was most probably removed
    }
}
