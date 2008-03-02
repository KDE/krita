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

#include "KoFind.h"
#include "KoText.h"

#include <KoCanvasResourceProvider.h>

#include <KActionCollection>
#include <KWindowSystem>
#include <KFindDialog>
#include <KReplaceDialog>
#include <KFind>
#include <KLocale>
#include <KMessageBox>
#include <KAction>

#include <QAction>
#include <QTextDocument>
#include <QTextCursor>
#include <QTimer>

#include <KDebug>

#include "FindDirection_p.h"
#include "KoFindStrategy.h"
#include "KoReplaceStrategy.h"

class InUse
{
public:
    InUse( bool & variable )
    : m_variable( variable )
    {
        m_variable = true;
    }
    ~InUse()
    {
        m_variable = false;
    }

private:
    bool & m_variable;
};

class KoFind::Private {
public:
    Private(KoFind *find, KoCanvasResourceProvider *crp, QWidget *w)
        :provider(crp),
        findStrategy(w),
        replaceStrategy(w),
        strategy(&findStrategy),
        findNext(0),
        findPrev(0),
        document(0),
        restarted(false),
        start( false ),
        inFind( false ),
        findDirection( 0 ),
        findForward( crp ),
        findBackward( crp )
    {
        connect( findStrategy.dialog(), SIGNAL( okClicked() ), find, SLOT( startFind() ) );
        connect( replaceStrategy.dialog(), SIGNAL( okClicked() ), find, SLOT( startReplace() ) );
    }

    void resourceChanged(int key, const QVariant &variant)
    {
        if(key == KoText::CurrentTextDocument) {
            document = static_cast<QTextDocument*> (variant.value<void*>());
            if ( !inFind ) {
                start = true;
            }
        }
        else if (key == KoText::CurrentTextPosition || key == KoText::CurrentTextAnchor) {
            if ( !inFind ) {
                const int selectionStart = provider->intResource( KoText::CurrentTextPosition );
                const int selectionEnd = provider->intResource( KoText::CurrentTextAnchor );
                findStrategy.dialog()->setHasSelection( selectionEnd != selectionStart );
                replaceStrategy.dialog()->setHasSelection( selectionEnd != selectionStart );

                start = true;
                provider->clearResource( KoText::SelectedTextPosition );
                provider->clearResource( KoText::SelectedTextAnchor );
            }
        }
    }

    void findActivated()
    {
        start = true;

        findStrategy.dialog()->setFindHistory( strategy->dialog()->findHistory() );

        strategy = &findStrategy;

        strategy->dialog()->show();
        KWindowSystem::activateWindow( strategy->dialog()->winId() );

        findNext->setEnabled( true );
        findPrev->setEnabled( true );
    }

    void findNextActivated()
    {
        Q_ASSERT( strategy );
        findStrategy.dialog()->setOptions( (strategy->dialog()->options() | KFind::FindBackwards ) ^ KFind::FindBackwards );
        strategy = &findStrategy;
        parseSettingsAndFind();
    }

    void findPreviousActivated()
    {
        Q_ASSERT( strategy );
        findStrategy.dialog()->setOptions( strategy->dialog()->options() | KFind::FindBackwards );
        strategy = &findStrategy;
        parseSettingsAndFind();
    }

    void replaceActivated()
    {
        start = true;

        replaceStrategy.dialog()->setFindHistory( strategy->dialog()->findHistory() );

        strategy = &replaceStrategy;

        strategy->dialog()->show();
        KWindowSystem::activateWindow( strategy->dialog()->winId() );
    }

    // executed when the user presses the 'find' button.
    void startFind()
    {
        parseSettingsAndFind();

        QTimer::singleShot(0, findStrategy.dialog(), SLOT(show())); // show the findDialog again.
    }

    void startReplace()
    {
        replaceStrategy.dialog()->hide(); // We don't want the replace dialog to keep popping up
        parseSettingsAndFind();
    }

    void parseSettingsAndFind()
    {
        if(document == 0)
            return;

        InUse used( inFind );

        long options = strategy->dialog()->options();

        QTextDocument::FindFlags flags;
        if ( ( options & KFind::WholeWordsOnly ) != 0 ) {
            flags |= QTextDocument::FindWholeWords;
        }
        if ( ( options & KFind::CaseSensitive ) != 0 ) {
            flags |= QTextDocument::FindCaseSensitively;
        }
        if ( ( options & KFind::FindBackwards ) != 0 ) {
            flags |= QTextDocument::FindBackward;
            findDirection = &findBackward;
        }
        else {
            findDirection = &findForward;
        }

        const bool selectedText = ( options & KFind::SelectedText) != 0;

        if ( start ) {
            start = false;
            restarted = false;
            strategy->reset();
            lastKnownPosition = QTextCursor( document );
            if ( selectedText ) {
                int selectionStart = provider->intResource( KoText::CurrentTextPosition );
                int selectionEnd = provider->intResource( KoText::CurrentTextAnchor );
                if ( selectionEnd < selectionStart ) {
                    qSwap( selectionStart, selectionEnd );
                }
                // TODO the SelectedTextPosition and SelectedTextAnchor are not highlighted yet
                // it would be cool to have the highlighted ligher when searching in selected text
                provider->setResource( KoText::SelectedTextPosition, selectionStart );
                provider->setResource( KoText::SelectedTextAnchor, selectionEnd );
                if ( ( options & KFind::FindBackwards ) != 0 ) {
                    lastKnownPosition.setPosition( selectionEnd );
                    endPosition.setPosition( selectionStart );
                }
                else {
                    lastKnownPosition.setPosition( selectionStart );
                    endPosition.setPosition( selectionEnd );
                }
                startPosition = lastKnownPosition;
            }
            else {
                if ( ( options & KFind::FromCursor ) != 0 ) {
                    lastKnownPosition.setPosition( provider->intResource(KoText::CurrentTextPosition ) );
                }
                else {
                    lastKnownPosition.setPosition( 0 );
                }
                endPosition = lastKnownPosition;
                startPosition = lastKnownPosition;
            }
            //kDebug() << "start" << lastKnownPosition.position();
        }

        QRegExp regExp;
        QString pattern = strategy->dialog()->pattern();
        if ( options & KFind::RegularExpression ) {
            regExp = QRegExp( pattern );
        }

        QTextCursor cursor;
        if ( !regExp.isEmpty() && regExp.isValid() ) {
            cursor = document->find( regExp, lastKnownPosition, flags );
        }
        else {
            cursor = document->find( pattern, lastKnownPosition, flags );
        }

        //kDebug() << "r" << restarted << "c > e" << ( cursor > endPosition ) << "e" << cursor.atEnd() << "n" << cursor.isNull();
        if ( ( restarted || selectedText ) && ( cursor.isNull() || findDirection->positionReached( cursor, endPosition ) ) ) {
            restarted = false;
            strategy->displayFinalDialog();
            lastKnownPosition = startPosition;
            return;
        }
        else if ( cursor.isNull() ) {
            // TODO go to next document
            restarted = true;
            findDirection->positionCursor( lastKnownPosition );
            // restart from the beginning
            parseSettingsAndFind();
            return;
        }
        else {
            // found something
            bool goOn = strategy->foundMatch( cursor, findDirection );
            lastKnownPosition = cursor;
            if ( goOn ) {
                parseSettingsAndFind();
            }
        }
    }

    KoCanvasResourceProvider *provider;
    KoFindStrategy findStrategy;
    KoReplaceStrategy replaceStrategy;
    KoFindStrategyBase * strategy;

    QAction *findNext;
    QAction *findPrev;

    QTextDocument *document;
    QTextCursor lastKnownPosition;
    bool restarted;
    bool start;
    bool inFind;
    QTextCursor startPosition;
    QTextCursor endPosition;
    FindDirection * findDirection;
    FindForward findForward;
    FindBackward findBackward;
};

KoFind::KoFind(QWidget *parent, KoCanvasResourceProvider *provider, KActionCollection *ac)
    : QObject(parent),
    d (new Private(this, provider, parent))
{
    connect(provider, SIGNAL(resourceChanged(int,const QVariant&)), this, SLOT(resourceChanged(int,const QVariant& )));
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
