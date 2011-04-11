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
#include <QtGui/QMenu>
#include <QtGui/QApplication>
#include <QtCore/QDebug>

#include <KDE/KLocalizedString>
#include <KDE/KLineEdit>
#include <KDE/KSqueezedTextLabel>
#include <KDE/KIcon>
#include <KDE/KHistoryComboBox>
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KColorScheme>

#include "KoFindBase.h"
#include "KoFindOptionSet.h"
#include "KoFindOption.h"

class KoFindToolbar::Private
{
public:
    Private(KoFindToolbar *qq) : q(qq) { }

    void matchFound();
    void noMatchFound();
    void searchWrapped(bool direction);
    void addToHistory();
    void find(const QString &pattern);
    void optionChanged();

    KoFindToolbar *q;

    KoFindBase *finder;

    QToolButton *closeButton;
    KHistoryComboBox *searchLine;
    QToolButton *previousButton;
    QToolButton *nextButton;
    QToolButton *optionsButton;
    KSqueezedTextLabel *information;

    static QStringList completionItems;
};

QStringList KoFindToolbar::Private::completionItems = QStringList();

KoFindToolbar::KoFindToolbar(KoFindBase *finder, KActionCollection *ac, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), d(new Private(this))
{
    QHBoxLayout *layout = new QHBoxLayout();

    d->finder = finder;
    connect(d->finder, SIGNAL(matchFound(KoFindMatch)), this, SLOT(matchFound()));
    connect(d->finder, SIGNAL(noMatchFound()), this, SLOT(noMatchFound()));
    connect(d->finder, SIGNAL(wrapAround(bool)), this, SLOT(searchWrapped(bool)));

    d->closeButton = new QToolButton(this);
    d->closeButton->setAutoRaise(true);
    d->closeButton->setIcon(KIcon("dialog-close"));
    d->closeButton->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(d->closeButton, SIGNAL(clicked(bool)), this, SLOT(hide()));
    connect(d->closeButton, SIGNAL(clicked(bool)), d->finder, SLOT(finished()));
    layout->addWidget(d->closeButton);

    layout->addWidget(new QLabel(i18n("Find:"), this));

    d->searchLine = new KHistoryComboBox(true, this);
    d->searchLine->setCompletedItems(d->completionItems);
    d->searchLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(d->searchLine, SIGNAL(editTextChanged(QString)), this, SLOT(find(QString)));
    connect(d->searchLine, SIGNAL(returnPressed()), d->finder, SLOT(findNext()));
    connect(d->searchLine, SIGNAL(returnPressed(QString)), d->searchLine, SLOT(addToHistory(QString)));
    connect(d->searchLine, SIGNAL(cleared()), finder, SLOT(finished()));
    layout->addWidget(d->searchLine);

    d->nextButton = new QToolButton(this);
    d->nextButton->setIcon(KIcon("go-down-search"));
    d->nextButton->setText(i18nc("Next search result", "Next"));
    d->nextButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->nextButton->setEnabled(false);
    connect(d->nextButton, SIGNAL(clicked(bool)), d->finder, SLOT(findNext()));
    connect(d->nextButton, SIGNAL(clicked(bool)), this, SLOT(addToHistory()));
    connect(d->finder, SIGNAL(hasMatchesChanged(bool)), d->nextButton, SLOT(setEnabled(bool)));
    layout->addWidget(d->nextButton);

    d->previousButton = new QToolButton(this);
    d->previousButton->setIcon(KIcon("go-up-search"));
    d->previousButton->setText(i18nc("Previous search result", "Previous"));
    d->previousButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->previousButton->setEnabled(false);
    connect(d->previousButton, SIGNAL(clicked(bool)), d->finder, SLOT(findPrevious()));
    connect(d->previousButton, SIGNAL(clicked(bool)), this, SLOT(addToHistory()));
    connect(d->finder, SIGNAL(hasMatchesChanged(bool)), d->previousButton, SLOT(setEnabled(bool)));
    layout->addWidget(d->previousButton);

    d->optionsButton = new QToolButton(this);
    d->optionsButton->setText(i18nc("Search options", "Options"));

    QMenu *menu = new QMenu(d->optionsButton);

    QList<KoFindOption *> options = finder->options()->options();
    foreach(KoFindOption * option, options) {
        KAction *action = new KAction(option->title(), menu);
        action->setHelpText(option->description());
        action->setObjectName(option->name());
        if(option->value().type() == QVariant::Bool) {
            action->setCheckable(true);
            action->setChecked(option->value().toBool());
            connect(action, SIGNAL(triggered(bool)), this, SLOT(optionChanged()));
        }
        menu->addAction(action);
    }

    d->optionsButton->setMenu(menu);
    d->optionsButton->setPopupMode(QToolButton::InstantPopup);
    layout->addWidget(d->optionsButton);

    d->information = new KSqueezedTextLabel(this);
    layout->addWidget(d->information);

    setLayout(layout);

    ac->addAction(KStandardAction::Find, "edit_find", this, SLOT(activate()));

    KAction *findNextAction = ac->addAction(KStandardAction::FindNext, "edit_findnext", d->nextButton, SIGNAL(clicked(bool)));
    connect(finder, SIGNAL(hasMatchesChanged(bool)), findNextAction, SLOT(setEnabled(bool)));
    connect(findNextAction, SIGNAL(triggered(bool)), this, SLOT(activate()));
    findNextAction->setEnabled(false);
    KAction *findPrevAction = ac->addAction(KStandardAction::FindPrev, "edit_findprevious", d->previousButton, SIGNAL(clicked(bool)));
    connect(finder, SIGNAL(hasMatchesChanged(bool)), findPrevAction, SLOT(setEnabled(bool)));
    connect(findPrevAction, SIGNAL(triggered(bool)), this, SLOT(activate()));
    findPrevAction->setEnabled(false);
}

KoFindToolbar::~KoFindToolbar()
{
    delete d;
}

void KoFindToolbar::activate()
{
    if(!isVisible()) {
        show();
    }
    d->searchLine->setFocus();

    if(d->finder->matches().size() == 0) {
        d->find(d->searchLine->currentText());
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

void KoFindToolbar::Private::searchWrapped(bool direction)
{
    if(direction) {
        information->setText(i18n("Search hit bottom, continuing from top."));
    } else {
        information->setText(i18n("Search hit top, continuing from bottom."));
    }
}

void KoFindToolbar::Private::addToHistory()
{
    searchLine->addToHistory(searchLine->currentText());
}

void KoFindToolbar::Private::find(const QString &pattern)
{
    if(pattern.length() > 0) {
        finder->find(pattern);
    } else {
        finder->finished();
        information->setText(QString());
        searchLine->setPalette(qApp->palette());
    }
}

void KoFindToolbar::Private::optionChanged()
{
    QAction *action = qobject_cast<QAction *>(q->sender());
    if(action) {
        finder->options()->setOptionValue(action->objectName(), action->isChecked());
        find(searchLine->currentText());
    }
}

#include "KoFindToolbar.moc"
