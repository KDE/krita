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

#include <QToolButton>
#include <QGridLayout>

#include <klocale.h>
#include <kcombobox.h>

#include <KoIcon.h>

#include "KoResourceItemChooserContextMenu.h"

#include "KoTagChooserWidget.h"
class KoTagToolButton::Private
{
public:
    QToolButton* tagToolButton;
    QAction* action_undeleteTag;
    QAction* action_deleteTag;
    KoLineEditAction* action_renameTag;
    QAction* action_purgeTagUndeleteList;
    QString undeleteCandidate;
};

KoTagToolButton::KoTagToolButton(QWidget* parent)
    :QWidget(parent), d(new Private())
{
    QGridLayout* buttonLayout = new QGridLayout(this);
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);

    d->tagToolButton = new QToolButton(this);
    d->tagToolButton->setIcon(koIcon("list-add"));
    d->tagToolButton->setToolTip(i18nc("@info:tooltip", "<qt>Show the tag box options.</qt>"));
    d->tagToolButton->setPopupMode(QToolButton::InstantPopup);
    d->tagToolButton->setEnabled(true);

    QMenu* popup = new QMenu(this);

    KoLineEditAction*  addTagAction = new KoLineEditAction(popup);
    addTagAction->setClickMessage(i18n("New tag"));
    addTagAction->setIcon(koIcon("document-new"));
    addTagAction->closeParentOnTrigger(true);
    popup->addAction(addTagAction);

    connect(addTagAction, SIGNAL(triggered(QString)),
            this, SIGNAL(newTagRequested(QString)));

    d->action_renameTag = new KoLineEditAction(popup);
    d->action_renameTag->setClickMessage(i18n("Rename tag"));
    d->action_renameTag->setIcon(koIcon("edit-rename"));
    d->action_renameTag->closeParentOnTrigger(true);
    popup->addAction(d->action_renameTag);

    connect(d->action_renameTag, SIGNAL(triggered(QString)),
            this, SIGNAL(renamingOfCurrentTagRequested(QString)));

    popup->addSeparator();

    d->action_deleteTag = new QAction(popup);
    d->action_deleteTag->setText(i18n("Delete this tag"));
    d->action_deleteTag->setIcon(koIcon("edit-delete"));
    popup->addAction(d->action_deleteTag);

    connect(d->action_deleteTag, SIGNAL(triggered()),
            this, SIGNAL(deletionOfCurrentTagRequested()));

    popup->addSeparator();

    d->action_undeleteTag = new QAction(popup);
    d->action_undeleteTag->setIcon(koIcon("edit-redo"));
    d->action_undeleteTag->setVisible(false);
    popup->addAction(d->action_undeleteTag);

    connect(d->action_undeleteTag, SIGNAL(triggered()),
            this, SLOT(onTagUndeleteClicked()));

    d->action_purgeTagUndeleteList = new QAction(popup);
    d->action_purgeTagUndeleteList->setText(i18n("Clear undelete list"));
    d->action_purgeTagUndeleteList->setIcon(koIcon("edit-clear"));
    d->action_purgeTagUndeleteList->setVisible(false);
    popup->addAction(d->action_purgeTagUndeleteList);

    connect(d->action_purgeTagUndeleteList, SIGNAL(triggered()),
            this, SIGNAL(purgingOfTagUndeleteListRequested()));

    connect(popup, SIGNAL(aboutToShow()),
            this, SIGNAL(popupMenuAboutToShow()));

    d->tagToolButton->setMenu(popup);
    buttonLayout->addWidget(d->tagToolButton);
}

KoTagToolButton::~KoTagToolButton()
{
    delete d;
}

void KoTagToolButton::readOnlyMode(bool activate)
{
    activate = !activate;
    d->action_renameTag->setVisible(activate);
    d->action_deleteTag->setVisible(activate);
}

void KoTagToolButton::setUndeletionCandidate(const QString& deletedTagName)
{
    d->undeleteCandidate = deletedTagName;
    d->action_undeleteTag->setText(i18n("Undelete") +" "+ deletedTagName);
    d->action_undeleteTag->setVisible(!deletedTagName.isEmpty());
    d->action_purgeTagUndeleteList->setVisible(!deletedTagName.isEmpty());
}

void KoTagToolButton::onTagUndeleteClicked()
{
    emit undeletionOfTagRequested(d->undeleteCandidate);
}

class KoTagChooserWidget::Private
{
public:
    KComboBox* comboBox;
    KoTagToolButton* tagToolButton;
    QList<QString> readOnlyTags;
    QList<QString> tags;
};

KoTagChooserWidget::KoTagChooserWidget(QWidget* parent): QWidget(parent)
, d(new Private())
{
    d->comboBox = new KComboBox(this);
    d->comboBox->setInsertPolicy(KComboBox::InsertAlphabetically);

    QGridLayout* comboLayout = new QGridLayout(this);

    comboLayout->addWidget(d->comboBox, 0, 0);

    d->tagToolButton = new KoTagToolButton(this);
    comboLayout->addWidget(d->tagToolButton, 0, 1);

    comboLayout->setSpacing(0);
    comboLayout->setMargin(0);
    comboLayout->setColumnStretch(0, 3);
    this->setEnabled(true);
    clear();

    connect(d->comboBox, SIGNAL(currentIndexChanged(QString)),
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
    return d->comboBox->findText(tagName);
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
    if (findIndexOf(tagName) == -1) {
        insertItemAt(index, tagName);
    }
}

void KoTagChooserWidget::insertItemAt(int index, QString tag)
{
    d->comboBox->insertItem(index,tag);
}

QString KoTagChooserWidget::currentlySelectedTag()
{
    return d->comboBox->currentText();
}

QStringList KoTagChooserWidget::allTags()
{
    return d->tags;
}

bool KoTagChooserWidget::selectedTagIsReadOnly()
{
    return d->readOnlyTags.contains(d->comboBox->currentText()) ;
}

void KoTagChooserWidget::addItems(QStringList tagNames)
{
    tagNames.sort();
    QStringList items;

    foreach(const QString & readOnlyTag, d->readOnlyTags) {
        items.append(readOnlyTag);
    }

    items.append(tagNames);
    d->tags.append(tagNames);

    d->comboBox->addItems(items);
}

void KoTagChooserWidget::clear()
{
    d->comboBox->clear();
}

void KoTagChooserWidget::removeItem(QString item)
{
    int pos = d->comboBox->findText(item);
    if (pos >= 0) {
        d->comboBox->removeItem(pos);
    }
}

void KoTagChooserWidget::tagOptionsContextMenuAboutToShow()
{
    /* only enable the save button if the selected tag set is editable */
    d->tagToolButton->readOnlyMode(selectedTagIsReadOnly());
    emit popupMenuAboutToShow();
}
