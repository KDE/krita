/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoFind_p.h"

#include <KoResourceManager.h>

#include <KWindowSystem>
#include <KFindDialog>
#include <KReplaceDialog>
#include <KFind>
#include <KLocale>
#include <KAction>

#include <QTextDocument>
#include <QTextCursor>
#include <QTimer>
#include <KDebug>

#include "KoFind.h"
#include "KoText.h"

class InUse
{
public:
    InUse(bool & variable)
            : m_variable(variable) {
        m_variable = true;
    }
    ~InUse() {
        m_variable = false;
    }

private:
    bool & m_variable;
};

KoFindPrivate::KoFindPrivate(KoFind *find, KoResourceManager *crp, QWidget *w)
        : findNext(0)
        , findPrev(0)
        , q(find)
        , provider(crp)
        , findStrategy(w)
        , replaceStrategy(w)
        , strategy(&findStrategy)
        , document(0)
        , restarted(false)
        , start(false)
        , inFind(false)
        , findDirection(0)
        , findForward(crp)
        , findBackward(crp)
{
    QObject::connect(findStrategy.dialog(), SIGNAL(okClicked()), q, SLOT(startFind()));
    QObject::connect(replaceStrategy.dialog(), SIGNAL(okClicked()), q, SLOT(startReplace()));
}

void KoFindPrivate::resourceChanged(int key, const QVariant &variant)
{
    if (key == KoText::CurrentTextDocument) {
        document = static_cast<QTextDocument*>(variant.value<void*>());
        if (!inFind) {
            start = true;
        }
    } else if (key == KoText::CurrentTextPosition || key == KoText::CurrentTextAnchor) {
        if (!inFind) {
            const int selectionStart = provider->intResource(KoText::CurrentTextPosition);
            const int selectionEnd = provider->intResource(KoText::CurrentTextAnchor);
            findStrategy.dialog()->setHasSelection(selectionEnd != selectionStart);
            replaceStrategy.dialog()->setHasSelection(selectionEnd != selectionStart);

            start = true;
            provider->clearResource(KoText::SelectedTextPosition);
            provider->clearResource(KoText::SelectedTextAnchor);
        }
    }
}

void KoFindPrivate::findActivated()
{
    start = true;

    findStrategy.dialog()->setFindHistory(strategy->dialog()->findHistory());

    strategy = &findStrategy;

    strategy->dialog()->show();
    KWindowSystem::activateWindow(strategy->dialog()->winId());

    findNext->setEnabled(true);
    findPrev->setEnabled(true);
}

void KoFindPrivate::findNextActivated()
{
    Q_ASSERT(strategy);
    findStrategy.dialog()->setOptions((strategy->dialog()->options() | KFind::FindBackwards) ^ KFind::FindBackwards);
    strategy = &findStrategy;
    parseSettingsAndFind();
}

void KoFindPrivate::findPreviousActivated()
{
    Q_ASSERT(strategy);
    findStrategy.dialog()->setOptions(strategy->dialog()->options() | KFind::FindBackwards);
    strategy = &findStrategy;
    parseSettingsAndFind();
}

void KoFindPrivate::replaceActivated()
{
    start = true;

    replaceStrategy.dialog()->setFindHistory(strategy->dialog()->findHistory());

    strategy = &replaceStrategy;

    strategy->dialog()->show();
    KWindowSystem::activateWindow(strategy->dialog()->winId());
}

void KoFindPrivate::startFind()
{
    parseSettingsAndFind();

    QTimer::singleShot(0, findStrategy.dialog(), SLOT(show())); // show the findDialog again.
}

void KoFindPrivate::startReplace()
{
    replaceStrategy.dialog()->hide(); // We don't want the replace dialog to keep popping up
    parseSettingsAndFind();
}

void KoFindPrivate::findDocumentSetNext(QTextDocument * document)
{
    emit q->findDocumentSetNext(document);
}

void KoFindPrivate::findDocumentSetPrevious(QTextDocument * document)
{
    emit q->findDocumentSetPrevious(document);
}

void KoFindPrivate::parseSettingsAndFind()
{
    if (document == 0)
        return;

    InUse used(inFind);

    long options = strategy->dialog()->options();

    QTextDocument::FindFlags flags;
    if ((options & KFind::WholeWordsOnly) != 0) {
        flags |= QTextDocument::FindWholeWords;
    }
    if ((options & KFind::CaseSensitive) != 0) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if ((options & KFind::FindBackwards) != 0) {
        flags |= QTextDocument::FindBackward;
        findDirection = &findBackward;
    } else {
        findDirection = &findForward;
    }

    const bool selectedText = (options & KFind::SelectedText) != 0;

    if (start) {
        start = false;
        restarted = false;
        strategy->reset();
        startDocument = document;
        lastKnownPosition = QTextCursor(document);
        if (selectedText) {
            int selectionStart = provider->intResource(KoText::CurrentTextPosition);
            int selectionEnd = provider->intResource(KoText::CurrentTextAnchor);
            if (selectionEnd < selectionStart) {
                qSwap(selectionStart, selectionEnd);
            }
            // TODO the SelectedTextPosition and SelectedTextAnchor are not highlighted yet
            // it would be cool to have the highlighted ligher when searching in selected text
            provider->setResource(KoText::SelectedTextPosition, selectionStart);
            provider->setResource(KoText::SelectedTextAnchor, selectionEnd);
            if ((options & KFind::FindBackwards) != 0) {
                lastKnownPosition.setPosition(selectionEnd);
                endPosition.setPosition(selectionStart);
            } else {
                lastKnownPosition.setPosition(selectionStart);
                endPosition.setPosition(selectionEnd);
            }
            startPosition = lastKnownPosition;
        } else {
            if ((options & KFind::FromCursor) != 0) {
                lastKnownPosition.setPosition(provider->intResource(KoText::CurrentTextPosition));
            } else {
                lastKnownPosition.setPosition(0);
            }
            endPosition = lastKnownPosition;
            startPosition = lastKnownPosition;
        }
        //kDebug() << "start" << lastKnownPosition.position();
    }

    QRegExp regExp;
    QString pattern = strategy->dialog()->pattern();
    if (options & KFind::RegularExpression) {
        regExp = QRegExp(pattern);
    }

    QTextCursor cursor;
    if (!regExp.isEmpty() && regExp.isValid()) {
        cursor = document->find(regExp, lastKnownPosition, flags);
    } else {
        cursor = document->find(pattern, lastKnownPosition, flags);
    }

    //kDebug() << "r" << restarted << "c > e" << ( document == startDocument && cursor > endPosition ) << ( startDocument == document && findDirection->positionReached(  cursor, endPosition ) )<< "e" << cursor.atEnd() << "n" << cursor.isNull();
    if ((((document == startDocument) && restarted) || selectedText)
            && (cursor.isNull() || findDirection->positionReached(cursor, endPosition))) {
        restarted = false;
        strategy->displayFinalDialog();
        lastKnownPosition = startPosition;
        return;
    } else if (cursor.isNull()) {
        restarted = true;
        findDirection->nextDocument(document, this);
        lastKnownPosition = QTextCursor(document);
        findDirection->positionCursor(lastKnownPosition);
        // restart from the beginning
        parseSettingsAndFind();
        return;
    } else {
        // found something
        bool goOn = strategy->foundMatch(cursor, findDirection);
        lastKnownPosition = cursor;
        if (goOn) {
            parseSettingsAndFind();
        }
    }
}
