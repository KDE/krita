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

#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QTimer>
#include <QApplication>
#include <QDebug>

#include <KDE/KLocalizedString>
#include <KDE/KLineEdit>
#include <KDE/KSqueezedTextLabel>
#include <KDE/KHistoryComboBox>
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KColorScheme>

#include <KoIcon.h>

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
    void replace();
    void replaceAll();
    void inputTimeout();

    KoFindToolbar *q;

    KoFindBase *finder;

    QToolButton *closeButton;
    KHistoryComboBox *searchLine;
    KHistoryComboBox *replaceLine;
    QToolButton *previousButton;
    QToolButton *nextButton;
    QToolButton *optionsButton;
    QToolButton *replaceButton;
    QToolButton *replaceAllButton;
    QLabel *replaceLabel;
    KSqueezedTextLabel *information;
    QLabel *matchCounter;
    QTimer *textTimeout;

    static QStringList searchCompletionItems;
    static QStringList replaceCompletionItems;
};

QStringList KoFindToolbar::Private::searchCompletionItems = QStringList();
QStringList KoFindToolbar::Private::replaceCompletionItems = QStringList();

KoFindToolbar::KoFindToolbar(KoFindBase *finder, KActionCollection *ac, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), d(new Private(this))
{
    QGridLayout *layout = new QGridLayout();

    d->finder = finder;
    connect(d->finder, SIGNAL(matchFound(KoFindMatch)), this, SLOT(matchFound()));
    connect(d->finder, SIGNAL(noMatchFound()), this, SLOT(noMatchFound()));
    connect(d->finder, SIGNAL(wrapAround(bool)), this, SLOT(searchWrapped(bool)));

    d->textTimeout = new QTimer(this);
    d->textTimeout->setInterval(1000);
    d->textTimeout->setSingleShot(true);
    connect(d->textTimeout, SIGNAL(timeout()), this, SLOT(inputTimeout()));

    d->closeButton = new QToolButton(this);
    d->closeButton->setAutoRaise(true);
    d->closeButton->setIcon(koIcon("dialog-close"));
    d->closeButton->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(d->closeButton, SIGNAL(clicked(bool)), this, SLOT(hide()));
    connect(d->closeButton, SIGNAL(clicked(bool)), d->finder, SLOT(finished()));
    connect(d->closeButton, SIGNAL(clicked()), d->textTimeout, SLOT(stop()));
    layout->addWidget(d->closeButton, 0, 0);

    layout->addWidget(new QLabel(i18nc("Label for the Find text input box", "Find:"), this), 0, 1, Qt::AlignRight);

    d->searchLine = new KHistoryComboBox(true, this);
    d->searchLine->setCompletedItems(d->searchCompletionItems);
    d->searchLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(d->searchLine, SIGNAL(editTextChanged(QString)), d->textTimeout, SLOT(start()));
    connect(d->searchLine, SIGNAL(returnPressed()), d->finder, SLOT(findNext()));
    connect(d->searchLine, SIGNAL(returnPressed(QString)), d->searchLine, SLOT(addToHistory(QString)));
    connect(d->searchLine, SIGNAL(cleared()), finder, SLOT(finished()));
    layout->addWidget(d->searchLine, 0, 2);

    d->nextButton = new QToolButton(this);
    d->nextButton->setIcon(koIcon("go-down-search"));
    d->nextButton->setText(i18nc("Next search result", "Next"));
    d->nextButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->nextButton->setEnabled(false);
    connect(d->nextButton, SIGNAL(clicked(bool)), d->finder, SLOT(findNext()));
    connect(d->nextButton, SIGNAL(clicked(bool)), this, SLOT(addToHistory()));
    connect(d->finder, SIGNAL(hasMatchesChanged(bool)), d->nextButton, SLOT(setEnabled(bool)));
    layout->addWidget(d->nextButton, 0, 3);

    d->previousButton = new QToolButton(this);
    d->previousButton->setIcon(koIcon("go-up-search"));
    d->previousButton->setText(i18nc("Previous search result", "Previous"));
    d->previousButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->previousButton->setEnabled(false);
    connect(d->previousButton, SIGNAL(clicked(bool)), d->finder, SLOT(findPrevious()));
    connect(d->previousButton, SIGNAL(clicked(bool)), this, SLOT(addToHistory()));
    connect(d->finder, SIGNAL(hasMatchesChanged(bool)), d->previousButton, SLOT(setEnabled(bool)));
    layout->addWidget(d->previousButton, 0, 4);

    d->optionsButton = new QToolButton(this);
    d->optionsButton->setText(i18nc("Search options", "Options"));

    QMenu *menu = new QMenu(d->optionsButton);

    QList<KoFindOption *> options = finder->options()->options();
    foreach(KoFindOption * option, options) {
        if(option->value().type() == QVariant::Bool) {
            KAction *action = new KAction(option->title(), menu);
            action->setHelpText(option->description());
            action->setObjectName(option->name());
            action->setCheckable(true);
            action->setChecked(option->value().toBool());
            connect(action, SIGNAL(triggered(bool)), this, SLOT(optionChanged()));
            menu->addAction(action);
        }
    }

    d->optionsButton->setMenu(menu);
    d->optionsButton->setPopupMode(QToolButton::InstantPopup);
    if(menu->actions().count() == 0) {
        d->optionsButton->setEnabled(false);
    }
    layout->addWidget(d->optionsButton, 0, 5);

    d->information = new KSqueezedTextLabel(this);
    layout->addWidget(d->information, 0, 6);

    d->replaceLabel = new QLabel(i18nc("Label for the Replace text input box", "Replace:"), this);
    layout->addWidget(d->replaceLabel, 1, 1, Qt::AlignRight);

    d->replaceLine = new KHistoryComboBox(true, this);
    d->replaceLine->setHistoryItems(d->replaceCompletionItems);
    d->replaceLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(d->replaceLine, SIGNAL(returnPressed()), this, SLOT(replace()));
    layout->addWidget(d->replaceLine, 1, 2);

    d->replaceButton = new QToolButton(this);
    d->replaceButton->setText(i18nc("Replace the current match", "Replace"));
    d->replaceButton->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    connect(d->replaceButton, SIGNAL(clicked(bool)), this, SLOT(replace()));
    layout->addWidget(d->replaceButton, 1, 3);

    d->replaceAllButton = new QToolButton(this);
    d->replaceAllButton->setText(i18nc("Replace all found matches", "Replace All"));
    d->replaceAllButton->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    connect(d->replaceAllButton, SIGNAL(clicked(bool)), this, SLOT(replaceAll()));
    layout->addWidget(d->replaceAllButton, 1, 4);

    setLayout(layout);

    ac->addAction(KStandardAction::Find, "edit_find", this, SLOT(activateSearch()));
    ac->addAction(KStandardAction::Replace, "edit_replace", this, SLOT(activateReplace()));

    KAction *findNextAction = ac->addAction(KStandardAction::FindNext, "edit_findnext", d->nextButton, SIGNAL(clicked(bool)));
    connect(finder, SIGNAL(hasMatchesChanged(bool)), findNextAction, SLOT(setEnabled(bool)));
    connect(findNextAction, SIGNAL(triggered(bool)), this, SLOT(activateSearch()));
    findNextAction->setEnabled(false);
    KAction *findPrevAction = ac->addAction(KStandardAction::FindPrev, "edit_findprevious", d->previousButton, SIGNAL(clicked(bool)));
    connect(finder, SIGNAL(hasMatchesChanged(bool)), findPrevAction, SLOT(setEnabled(bool)));
    connect(findPrevAction, SIGNAL(triggered(bool)), this, SLOT(activateSearch()));
    findPrevAction->setEnabled(false);
}

KoFindToolbar::~KoFindToolbar()
{
    delete d;
}

void KoFindToolbar::activateSearch()
{
    d->replaceLabel->setVisible(false);
    d->replaceLine->setVisible(false);
    d->replaceButton->setVisible(false);
    d->replaceAllButton->setVisible(false);

    if(!isVisible()) {
        show();
    }
    d->searchLine->setFocus();

    if(d->finder->matches().size() == 0) {
        d->textTimeout->start();
    }
}

void KoFindToolbar::activateReplace()
{
    if(!isVisible()) {
        show();
    }
    d->searchLine->setFocus();

    d->replaceLabel->setVisible(true);
    d->replaceLine->setVisible(true);
    d->replaceButton->setVisible(true);
    d->replaceAllButton->setVisible(true);

    if(d->finder->matches().size() == 0) {
        d->textTimeout->start();
    }
}

void KoFindToolbar::Private::matchFound()
{
    QPalette current = searchLine->palette();
    KColorScheme::adjustBackground(current, KColorScheme::PositiveBackground);
    searchLine->setPalette(current);
    replaceLine->setPalette(current);

    information->setText(i18ncp("Total number of matches", "1 match found", "%1 matches found", finder->matches().count()));
}

void KoFindToolbar::Private::noMatchFound()
{
    QPalette current = searchLine->palette();
    KColorScheme::adjustBackground(current, KColorScheme::NegativeBackground);
    searchLine->setPalette(current);
    replaceLine->setPalette(current);

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
    textTimeout->stop();

    if(pattern.length() > 0) {
        finder->find(pattern);
    } else {
        finder->finished();
        information->setText(QString());
        searchLine->setPalette(qApp->palette());
        replaceLine->setPalette(qApp->palette());
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

void KoFindToolbar::Private::replace()
{
    finder->replaceCurrent(replaceLine->currentText());
    replaceLine->addToHistory(replaceLine->currentText());
}

void KoFindToolbar::Private::replaceAll()
{
    finder->replaceAll(replaceLine->currentText());
    replaceLine->addToHistory(replaceLine->currentText());
}

void KoFindToolbar::Private::inputTimeout()
{
    find(searchLine->currentText());
}

#include "KoFindToolbar.moc"
