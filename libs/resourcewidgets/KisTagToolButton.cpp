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
    UserInputTagAction *renameTagAction {0};
    UserInputTagAction *addTagAction {0};

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
    loadIcon();
    d->tagToolButton->setText(i18n("Tag"));
    d->tagToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->tagToolButton->setToolTip(i18nc("@info:tooltip", "<qt>Show the tag box options.</qt>"));
    d->tagToolButton->setPopupMode(QToolButton::InstantPopup);
    d->tagToolButton->setEnabled(true);

    QMenu* popup = new QMenu(this);

    d->addTagAction = new UserInputTagAction(popup);
    d->addTagAction->setPlaceholderText(i18n("New tag"));
    d->addTagAction->setIcon(koIcon("document-new"));
    d->addTagAction->setCloseParentOnTrigger(true);
    popup->addAction(d->addTagAction);

    connect(d->addTagAction, SIGNAL(triggered(QString)), this, SIGNAL(newTagRequested(QString)));

    d->renameTagAction = new UserInputTagAction(popup);
    d->renameTagAction->setPlaceholderText(i18n("Rename tag"));
    d->renameTagAction->setIcon(koIcon("edit-rename"));
    d->renameTagAction->setCloseParentOnTrigger(true);
    popup->addAction(d->renameTagAction);

    connect(d->renameTagAction, SIGNAL(triggered(QString)), this, SIGNAL(renamingOfCurrentTagRequested(QString)));

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
    d->deleteTagAction->setProperty("currentTag", QVariant::fromValue<KisTagSP>(tag));
}

void KisTagToolButton::loadIcon()
{
    d->tagToolButton->setIcon(koIcon("bookmarks"));
}

void KisTagToolButton::onTagUndeleteClicked()
{
    emit undeletionOfTagRequested(d->undeleteCandidate);
}


