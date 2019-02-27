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

#include "KoTagChooserWidget.h"

#include <QDebug>
#include <QToolButton>
#include <QGridLayout>

#include <klocalizedstring.h>
#include <KisSqueezedComboBox.h>

#include <KoIcon.h>

#include "KoResourceItemChooserContextMenu.h"

#include "KoTagToolButton.h"

class Q_DECL_HIDDEN KoTagChooserWidget::Private
{
public:
    KisSqueezedComboBox* comboBox;
    KoTagToolButton* tagToolButton;
    QStringList readOnlyTags;
    QStringList tags;
};

KoTagChooserWidget::KoTagChooserWidget(QWidget* parent): QWidget(parent)
  , d(new Private())
{
    d->comboBox = new KisSqueezedComboBox(this);
    d->comboBox->setToolTip(i18n("Tag"));
    d->comboBox->setSizePolicy(QSizePolicy::MinimumExpanding , QSizePolicy::Fixed );


    QGridLayout* comboLayout = new QGridLayout(this);

    comboLayout->addWidget(d->comboBox, 0, 0);

    d->tagToolButton = new KoTagToolButton(this);
    comboLayout->addWidget(d->tagToolButton, 0, 1);

    comboLayout->setSpacing(0);
    comboLayout->setMargin(0);
    comboLayout->setColumnStretch(0, 3);
    this->setEnabled(true);
    clear();

    connect(d->comboBox, SIGNAL(currentTextChanged(QString)),
            this, SIGNAL(tagChosen(QString)));

    connect(d->tagToolButton, SIGNAL(popupMenuAboutToShow()),
            this, SLOT (tagOptionsContextMenuAboutToShow()));
    connect(d->tagToolButton, SIGNAL(newTagRequested(QString)),
            this, SIGNAL(newTagRequested(QString)));
    connect(d->tagToolButton, SIGNAL(deletionOfCurrentTagRequested()),
            this, SLOT(contextDeleteCurrentTag()));
    connect(d->tagToolButton, SIGNAL(renamingOfCurrentTagRequested(QString)),
            this, SLOT(tagRenamingRequested(QString)));
    connect(d->tagToolButton, SIGNAL(undeletionOfTagRequested(QString)),
            this, SIGNAL(tagUndeletionRequested(QString)));
    connect(d->tagToolButton, SIGNAL(purgingOfTagUndeleteListRequested()),
            this, SIGNAL(tagUndeletionListPurgeRequested()));

}

KoTagChooserWidget::~KoTagChooserWidget()
{
    delete d;
}

void KoTagChooserWidget::contextDeleteCurrentTag()
{
    if (selectedTagIsReadOnly()) {
        return;
    }
    emit tagDeletionRequested(currentlySelectedTag());
}

void KoTagChooserWidget::tagRenamingRequested(const QString& newName)
{
    if (newName.isEmpty() || selectedTagIsReadOnly()) {
        return;
    }
    emit tagRenamingRequested(currentlySelectedTag(), newName);
}

void KoTagChooserWidget::setUndeletionCandidate(const QString& tag)
{
    d->tagToolButton->setUndeletionCandidate(tag);
}

void KoTagChooserWidget::setCurrentIndex(int index)
{
    d->comboBox->setCurrentIndex(index);
}

int KoTagChooserWidget::findIndexOf(QString tagName)
{
    return d->comboBox->findOriginalText(tagName);
}

void KoTagChooserWidget::addReadOnlyItem(QString tagName)
{
    d->readOnlyTags.append(tagName);
}

void KoTagChooserWidget::insertItem(QString tagName)
{
    QStringList tags = allTags();
    tags.append(tagName);
    tags.sort();
    foreach (QString readOnlyTag, d->readOnlyTags) {
        tags.prepend(readOnlyTag);
    }

    int index = tags.indexOf(tagName);
    if (d->comboBox->findOriginalText(tagName) == -1) {
        d->comboBox->insertSqueezedItem(tagName, index);
        d->tags.append(tagName);
    }
}

QString KoTagChooserWidget::currentlySelectedTag()
{
    return d->comboBox->itemHighlighted();
}

QStringList KoTagChooserWidget::allTags()
{
    return d->tags;
}

bool KoTagChooserWidget::selectedTagIsReadOnly()
{
    return d->readOnlyTags.contains(d->comboBox->itemHighlighted()) ;
}

void KoTagChooserWidget::addItems(QStringList tagNames)
{
    d->tags.append(tagNames);
    d->tags.removeDuplicates();
    d->tags.sort();
    d->readOnlyTags.sort();

    QStringList items = d->readOnlyTags + d->tags;

    items.removeDuplicates();

    d->comboBox->resetOriginalTexts(items);
}

void KoTagChooserWidget::clear()
{
    d->comboBox->resetOriginalTexts(QStringList());
}

void KoTagChooserWidget::removeItem(QString item)
{
    int pos = findIndexOf(item);
    if (pos >= 0) {
        d->comboBox->removeSqueezedItem(pos);
        d->tags.removeOne(item);
    }
}

void KoTagChooserWidget::tagOptionsContextMenuAboutToShow()
{
    /* only enable the save button if the selected tag set is editable */
    d->tagToolButton->readOnlyMode(selectedTagIsReadOnly());
    emit popupMenuAboutToShow();
}

void KoTagChooserWidget::showTagToolButton(bool show)
{
    d->tagToolButton->setVisible(show);
}
