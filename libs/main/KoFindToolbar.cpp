/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */



#include "KoFindToolbar.h"
#include <QtGui/QHBoxLayout>
#include <QtGui/QToolButton>

#include <KDE/KLocalizedString>
#include <KDE/KLineEdit>
#include <KDE/KSqueezedTextLabel>
#include <KDE/KIcon>
#include <KDE/KHistoryComboBox>
#include <KDE/KAction>
#include <KDE/KActionCollection>

#include "KoFindBase.h"
#include <KColorScheme>
#include <QMenu>
#include <QShowEvent>

class KoFindToolbar::Private
{
public:
    Private(KoFindToolbar *qq) : q(qq) { }
    void textEdited(QString text);
    void matchFound();
    void noMatchFound();
    void searchWrapped();

    KoFindToolbar *q;

    KoFindBase *find;

    QToolButton *closeButton;
    KHistoryComboBox *searchLine;
    QToolButton *previousButton;
    QToolButton *nextButton;
    QToolButton *optionsButton;
    KSqueezedTextLabel* information;

    static QStringList completionItems;
};

QStringList KoFindToolbar::Private::completionItems = QStringList(); 

KoFindToolbar::KoFindToolbar(KoFindBase* find, KActionCollection *ac, QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), d(new Private(this))
{
    QHBoxLayout *layout = new QHBoxLayout();

    d->find = find;
    connect(d->find, SIGNAL(matchFound(KoFindMatch)), this, SLOT(matchFound()));
    connect(d->find, SIGNAL(noMatchFound()), this, SLOT(noMatchFound()));
    connect(d->find, SIGNAL(wrapAround()), this, SLOT(searchWrapped()));

    d->closeButton = new QToolButton(this);
    d->closeButton->setAutoRaise(true);
    d->closeButton->setIcon(KIcon("dialog-close"));
    d->closeButton->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(d->closeButton, SIGNAL(clicked(bool)), this, SLOT(hide()));
    layout->addWidget(d->closeButton);

    layout->addWidget(new QLabel(i18n("Find:"), this));

    d->searchLine = new KHistoryComboBox(true, this);
    d->searchLine->setCompletedItems(d->completionItems);
    d->searchLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(d->searchLine, SIGNAL(editTextChanged(QString)), this, SLOT(textEdited(QString)));
    layout->addWidget(d->searchLine);

    d->nextButton = new QToolButton(this);
    d->nextButton->setIcon(KIcon ("go-down-search"));
    d->nextButton->setText(i18nc("Next search result", "Next"));
    d->nextButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->nextButton->setEnabled(false);
    connect(d->nextButton, SIGNAL(clicked(bool)), d->find, SLOT(findNext()));
    connect(d->find, SIGNAL(hasMatchesChanged(bool)), d->nextButton, SLOT(setEnabled(bool)));
    layout->addWidget(d->nextButton);

    d->previousButton = new QToolButton(this);
    d->previousButton->setIcon(KIcon("go-up-search"));
    d->previousButton->setText(i18nc("Previous search result", "Previous"));
    d->previousButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->previousButton->setEnabled(false);
    connect(d->previousButton, SIGNAL(clicked(bool)), d->find, SLOT(findPrevious()));
    connect(d->find, SIGNAL(hasMatchesChanged(bool)), d->previousButton, SLOT(setEnabled(bool)));
    layout->addWidget(d->previousButton);

    d->optionsButton = new QToolButton(this);
    d->optionsButton->setText(i18nc("Search options", "Options"));
    QMenu* menu = new QMenu(d->optionsButton);
    menu->addAction(i18n("Case Sensitive"));
    menu->addAction(i18n("Whole Words Only"));
    d->optionsButton->setMenu(menu);
    layout->addWidget(d->optionsButton);

    d->information = new KSqueezedTextLabel(this);
    layout->addWidget(d->information);

    setLayout(layout);

    ac->addAction(KStandardAction::Find, "edit_find", this, SLOT(show()));

    KAction *findNextAction = ac->addAction(KStandardAction::FindNext, "edit_findnext", d->find, SLOT(findNext()));
    connect(find, SIGNAL(hasMatchesChanged(bool)), findNextAction, SLOT(setEnabled(bool)));
    findNextAction->setEnabled(false);
    KAction *findPrevAction = ac->addAction(KStandardAction::FindPrev, "edit_findprevious", d->find, SLOT(findPrevious()));
    connect(find, SIGNAL(hasMatchesChanged(bool)), findPrevAction, SLOT(setEnabled(bool)));
    findPrevAction->setEnabled(false);
}

KoFindToolbar::~KoFindToolbar()
{
    delete d;
}

void KoFindToolbar::showEvent(QShowEvent* evt)
{
    if(!evt->spontaneous()) {
        d->searchLine->setFocus();
    }
}

void KoFindToolbar::Private::textEdited(QString text)
{
    if(text.length() > 1) {
        find->find(text);
    }
}

void KoFindToolbar::Private::matchFound()
{
    QPalette current = searchLine->palette();
    KColorScheme::adjustBackground(current, KColorScheme::PositiveBackground);
    searchLine->setPalette(current);
    information->setText(QString());
}

void KoFindToolbar::Private::noMatchFound()
{
    QPalette current = searchLine->palette();
    KColorScheme::adjustBackground(current, KColorScheme::NegativeBackground);
    searchLine->setPalette(current);

    information->setText(i18n("No matches found"));
}

void KoFindToolbar::Private::searchWrapped()
{
    information->setText(i18n("Search hit bottom, continuing from top."));
}

#include "KoFindToolbar.moc"
