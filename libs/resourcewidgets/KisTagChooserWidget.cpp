/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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
    QList<KisTagSP> readOnlyTags;
    QList<KisTagSP> tags;
    KisTagModel* model;
    QScopedPointer<KisActiveFilterTagProxyModel> activeFilterModel;

    Private(KisTagModel* model)
        : activeFilterModel(new KisActiveFilterTagProxyModel(model))
    {

    }
};

KisTagChooserWidget::KisTagChooserWidget(KisTagModel* model, QWidget* parent)
    : QWidget(parent)
    , d(new Private(model))
{
    d->comboBox = new QComboBox(this);

    d->comboBox->setToolTip(i18n("Tag"));
    d->comboBox->setSizePolicy(QSizePolicy::MinimumExpanding , QSizePolicy::Fixed );

    //d->comboBox->setModel(d->activeFilterModel.get());
    d->comboBox->setModel(model);

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
            this, SLOT (tagOptionsContextMenuAboutToShow()));

    connect(d->tagToolButton, SIGNAL(newTagRequested(KisTagSP)),
            this, SLOT(insertItem(KisTagSP)));
    connect(d->tagToolButton, SIGNAL(deletionOfCurrentTagRequested()),
            this, SLOT(contextDeleteCurrentTag()));

    connect(d->tagToolButton, SIGNAL(renamingOfCurrentTagRequested(KisTagSP)),
            this, SLOT(tagRenamingRequested(KisTagSP)));
    connect(d->tagToolButton, SIGNAL(undeletionOfTagRequested(KisTagSP)),
            this, SIGNAL(tagUndeletionRequested(KisTagSP)));
    connect(d->tagToolButton, SIGNAL(purgingOfTagUndeleteListRequested()),
            this, SIGNAL(tagUndeletionListPurgeRequested()));

}

KisTagChooserWidget::~KisTagChooserWidget()
{
    delete d;
}

void KisTagChooserWidget::contextDeleteCurrentTag()
{
    ENTER_FUNCTION();
    KisTagSP currentTag = currentlySelectedTag();
    d->model->removeTag(currentTag);
}

void KisTagChooserWidget::tagChanged(int)
{
    ENTER_FUNCTION();
    emit tagChosen(currentlySelectedTag());
}

void KisTagChooserWidget::tagRenamingRequested(const KisTagSP newName)
{
    ENTER_FUNCTION();
    // TODO: RESOURCES: implement renaming (implement update in KisTagModel?)
    warnKrita << "Renaming of tags not implemented";
}

void KisTagChooserWidget::setUndeletionCandidate(const KisTagSP tag)
{
    ENTER_FUNCTION();
    d->tagToolButton->setUndeletionCandidate(tag);
}

void KisTagChooserWidget::setCurrentIndex(int index)
{
    ENTER_FUNCTION();
    d->comboBox->setCurrentIndex(index);
}

int KisTagChooserWidget::findIndexOf(KisTagSP tagName)
{
    ENTER_FUNCTION();
    return -1;
    //return d->comboBox->findOriginalText(tagName);
}

void KisTagChooserWidget::addReadOnlyItem(KisTagSP tag)
{
    d->model->addTag(tag);
    ENTER_FUNCTION();
}

void KisTagChooserWidget::insertItem(KisTagSP tag)
{
    fprintf(stderr, "inserting item!!! %s\n", tag->name().toUtf8().toStdString().c_str());
    tag->setUrl(tag->name());
    tag->setComment(tag->name());
    tag->setActive(true);
    tag->setValid(true);
    ENTER_FUNCTION();
    bool added = d->model->addTag(tag);
    fprintf(stderr, "added = %d\n", added);
}

KisTagSP KisTagChooserWidget::currentlySelectedTag()
{
    int row = d->comboBox->currentIndex();
    // TODO: RESOURCES: there shouldn't be any +1 for "All", of course;
    //d->comboBox->currentData();
    fprintf(stderr, "current data type = %s", d->comboBox->currentData().typeName());

    QModelIndex index = d->model->index(row - 1, 0);
    KisTagSP tag =  d->model->tagForIndex(index);
    ENTER_FUNCTION() << tag;
    return tag;
}

QList<KisTagSP> KisTagChooserWidget::allTags()
{
    ENTER_FUNCTION();
    QList<KisTagSP> list;
    for (int i = 0; i < d->model->rowCount(); i++) {
        QModelIndex index = d->model->index(i, 0);
        KisTagSP tag = d->model->tagForIndex(index);
         if (!tag.isNull()) {
             list << tag;
         }
     }

    return list;
}

bool KisTagChooserWidget::selectedTagIsReadOnly()
{
    ENTER_FUNCTION();
    return false;
}

void KisTagChooserWidget::addItems(QList<KisTagSP> tags)
{
    ENTER_FUNCTION();
    warnKrita << "not implemented";

    Q_FOREACH(KisTagSP tag, tags) {
        insertItem(tag);
    }
}

void KisTagChooserWidget::clear()
{
    ENTER_FUNCTION();
}

void KisTagChooserWidget::removeItem(KisTagSP item)
{
    fprintf(stderr, "removing item: %s\n", item->name().toUtf8().toStdString().c_str());
    ENTER_FUNCTION();
    d->model->removeTag(item);
}

void KisTagChooserWidget::tagOptionsContextMenuAboutToShow()
{
    ENTER_FUNCTION();
    /* only enable the save button if the selected tag set is editable */
    d->tagToolButton->readOnlyMode(selectedTagIsReadOnly());
    emit popupMenuAboutToShow();
}

void KisTagChooserWidget::showTagToolButton(bool show)
{
    ENTER_FUNCTION();
    d->tagToolButton->setVisible(show);
}
