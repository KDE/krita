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

#include "KisTagToolButton.h"

#include <QToolButton>
#include <QGridLayout>

#include <klocalizedstring.h>

#include <KoIcon.h>

#include <kis_debug.h>
#include <KisTagModel.h>

#include "KisResourceItemChooserContextMenu.h"


class KisTagToolButton::Private
{
public:
    QToolButton *tagToolButton {0};

    QAction *undeleteTagAction {0};
    QAction *deleteTagAction {0};
    TagEditAction *renameTagAction {0};
    NewTagAction *addTagAction {0};

    KisTagSP undeleteCandidate;
    KisTagSP currentTag;

};

KisTagToolButton::KisTagToolButton(QWidget* parent)
    : QWidget(parent)
    , d(new Private())
{
    QGridLayout* buttonLayout = new QGridLayout(this);
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);

    d->tagToolButton = new QToolButton(this);
    d->tagToolButton->setIcon(koIcon("bookmarks"));
    d->tagToolButton->setText(i18n("Tag"));
    d->tagToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->tagToolButton->setToolTip(i18nc("@info:tooltip", "<qt>Show the tag box options.</qt>"));
    d->tagToolButton->setPopupMode(QToolButton::InstantPopup);
    d->tagToolButton->setEnabled(true);

    QMenu* popup = new QMenu(this);

    d->addTagAction = new NewTagAction(0, popup);
    d->addTagAction->setPlaceholderText(i18n("New tag"));
    d->addTagAction->setIcon(koIcon("document-new"));
    d->addTagAction->closeParentOnTrigger(true);
    popup->addAction(d->addTagAction);

    connect(d->addTagAction, SIGNAL(triggered(QString)), this, SIGNAL(newTagRequested(QString)));

    d->renameTagAction = new TagEditAction(0, 0, popup);
    d->renameTagAction->setPlaceholderText(i18n("Rename tag"));
    d->renameTagAction->setIcon(koIcon("edit-rename"));
    d->renameTagAction->closeParentOnTrigger(true);
    popup->addAction(d->renameTagAction);

    connect(d->renameTagAction, SIGNAL(triggered(KisTagSP)), this, SIGNAL(renamingOfCurrentTagRequested(KisTagSP)));

    popup->addSeparator();

    d->deleteTagAction = new QAction(popup);
    d->deleteTagAction->setText(i18n("Delete this tag"));
    d->deleteTagAction->setIcon(koIcon("edit-delete"));
    popup->addAction(d->deleteTagAction);

    connect(d->deleteTagAction, SIGNAL(triggered()), this, SIGNAL(deletionOfCurrentTagRequested()));

    popup->addSeparator();

    d->undeleteTagAction = new QAction(popup);
    d->undeleteTagAction->setIcon(koIcon("edit-redo"));
    d->undeleteTagAction->setVisible(false);
    popup->addAction(d->undeleteTagAction);

    connect(d->undeleteTagAction, SIGNAL(triggered()), this, SLOT(onTagUndeleteClicked()));

    connect(popup, SIGNAL(aboutToShow()), this, SIGNAL(popupMenuAboutToShow()));

    d->tagToolButton->setMenu(popup);
    buttonLayout->addWidget(d->tagToolButton);
}

KisTagToolButton::~KisTagToolButton()
{
    delete d;
}

void KisTagToolButton::readOnlyMode(bool activate)
{
    activate = !activate;
    d->renameTagAction->setVisible(activate);
    d->deleteTagAction->setVisible(activate);
}

void KisTagToolButton::setUndeletionCandidate(const KisTagSP deletedTag)
{
    if (deletedTag.isNull() || deletedTag->name().isEmpty()) {
        d->undeleteTagAction->setVisible(false);
        return;
    } else {
        d->undeleteCandidate = deletedTag;
        d->undeleteTagAction->setText(i18n("Undelete") +" "+ deletedTag->name());
        d->undeleteTagAction->setVisible(true);
    }
}

void KisTagToolButton::setCurrentTag(const KisTagSP tag)
{
    d->currentTag = tag;
    d->addTagAction->setTag(tag);
    d->renameTagAction->setTag(tag);
    d->deleteTagAction->setProperty("currentTag", QVariant::fromValue<KisTagSP>(tag));
}

void KisTagToolButton::onTagUndeleteClicked()
{
    emit undeletionOfTagRequested(d->undeleteCandidate);
}

