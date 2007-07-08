/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoFind.h"
#include "KoText.h"

#include <KoCanvasResourceProvider.h>

#include <KActionCollection>
#include <KWindowSystem>
#include <KFindDialog>
#include <KFind>
#include <KLocale>
#include <KMessageBox>

#include <QAction>
#include <QTextDocument>
#include <QTextCursor>
#include <QTimer>

class NonClosingFindDialog : public KFindDialog {
public:
    NonClosingFindDialog(QWidget *parent) : KFindDialog(parent) {}

    virtual void accept() {}
};

class KoFind::Private {
public:
    Private(KoFind *find, KoCanvasResourceProvider *crp, QWidget *w)
        :provider(crp),
        widget(w),
        dialog(0),
        parent(find),
        findNext(0),
        findPrev(0),
        document(0),
        lastKnownPosition(-1),
        startedPosition(-1),
        matches(0),
        restarted(false)
    {
    }

    void resourceChanged(int key, const QVariant &variant) {
        if(key == KoText::CurrentTextDocument) {
            document = static_cast<QTextDocument*> (variant.value<void*>());
            lastKnownPosition = -1;
        }
        else if(key == KoText::CurrentTextPosition) {
            // TODO
        }
    }
    void findActivated() {
        lastKnownPosition = -1;
        if(dialog) {
            dialog->show();
            KWindowSystem::activateWindow( dialog->winId() );
            return;
        }
        dialog = new NonClosingFindDialog(widget);
        connect( dialog, SIGNAL(okClicked()), parent, SLOT(startFind()) );
        dialog->show();

        findNext->setEnabled(true);
        findPrev->setEnabled(true);

    }
    void findNextActivated() {
        if(dialog == 0) {
            findActivated();
            return;
        }
        parseSettingsAndFind();
    }
    void findPreviousActivated() {
        // set 'backwards' as option
        // search again
        parseSettingsAndFind();
    }
    void replaceActivated() {
        // TODO
    }

    void startFind() { // executed when the user presses the 'find' button.
        parseSettingsAndFind();

        QTimer::singleShot(0, dialog, SLOT(show())); // show the dialog again.
    }

    void parseSettingsAndFind() {
        if(document == 0)
            return;
        QTextDocument::FindFlags flags;
        if((dialog->options() & KFind::WholeWordsOnly) != 0)
            flags |= QTextDocument::FindWholeWords;
        if((dialog->options() & KFind::CaseSensitive) != 0)
            flags |= QTextDocument::FindCaseSensitively;
        if((dialog->options() & KFind::FindBackwards) != 0)
            flags |= QTextDocument::FindBackward;
        if(lastKnownPosition == -1) {
            if((dialog->options() & KFind::FromCursor) != 0)
                lastKnownPosition = provider->intResource(KoText::CurrentTextPosition);
            else
                lastKnownPosition = 0;
            startedPosition = lastKnownPosition;
            restarted = false;
            matches = 0;
        }
        // TODO SelectedText option ?

        QTextCursor cursor;
        QRegExp regExp;
        if(dialog->options() & KFind::RegularExpression)
            regExp = QRegExp(dialog->pattern());
        if(!regExp.isEmpty() && regExp.isValid())
            cursor = document->find(regExp, lastKnownPosition, flags);
        else
            cursor = document->find(dialog->pattern(), lastKnownPosition, flags);
        if(cursor.position() == -1 && !restarted && startedPosition <= lastKnownPosition) { // end of doc, restart
            lastKnownPosition = 0;
            restarted = true;
            parseSettingsAndFind();
            return;
        }

        if(restarted && cursor.position() > startedPosition || cursor.position() == -1) { // looped round.
            KMessageBox::information(dialog, i18n("Found %1 matches", matches));
            restarted = false; // allow to restart again.
            matches = 0;
            return;
        }

        provider->setResource(KoText::CurrentTextPosition, cursor.position());
        provider->setResource(KoText::CurrentTextAnchor, cursor.anchor());
        lastKnownPosition = cursor.position();
        matches++;
    }

    KoCanvasResourceProvider *provider;
    QWidget *widget;
    KFindDialog *dialog;
    KoFind *parent;
    QAction *findNext, *findPrev;

    QTextDocument *document;
    int lastKnownPosition, startedPosition, matches;
    bool restarted;
};

KoFind::KoFind(QWidget *parent, KoCanvasResourceProvider *provider, KActionCollection *ac)
    : QObject(parent),
    d (new Private(this, provider, parent))
{
    connect(provider, SIGNAL(sigResourceChanged(int,const QVariant&)), this, SLOT(resourceChanged(int,const QVariant& )));
    ac->addAction(KStandardAction::Find, "edit_find", this, SLOT( findActivated() ));
    d->findNext = ac->addAction(KStandardAction::FindNext, "edit_findnext", this, SLOT( findNextActivated() ));
    d->findNext->setEnabled(false);
    d->findPrev = ac->addAction(KStandardAction::FindPrev, "edit_findprevious", this, SLOT( findPreviousActivated() ));
    d->findPrev->setEnabled(false);
    ac->addAction(KStandardAction::Replace, "edit_replace", this, SLOT( replaceActivated() ));
}

KoFind::~KoFind() {
    delete d;
}

#include <KoFind.moc>
