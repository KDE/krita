/* This file is part of KOffice
   Copyright (C) 2004 Zack Rusin <zack@kde.org>

   Original version written by David Sweet <dsweet@kde.org> and
         Wolfram Diestel <wolfram@steloj.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "KoSpell.h"

#include "KoTextObject.h"
#include "KoTextParag.h"
#include "KoTextIterator.h"

#include <sonnet/loader.h>
#include <sonnet/filter.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <QTimer>

//#define DEBUG_SPELL

using namespace KSpell2;

class KoSpell::Private
{
public:
    KoTextIterator *itr;
    KoTextParag    *parag;
    bool            dialog;
    bool            needsIncrement;
    KoTextDocument *lastTxtDocument;
};

KoSpell::KoSpell( const KSpell2::Loader::Ptr& loader,  QObject *parent,
                  const char* /*name*/ )
    : BackgroundChecker( loader, parent )
{
    d = new Private;
    d->parag = 0;
    d->itr = 0;
    d->dialog = false;
    d->needsIncrement = false;
    d->lastTxtDocument = 0;
}

KoSpell::~KoSpell()
{
    delete d;
}

bool KoSpell::check( KoTextIterator *itr, bool dialog )
{
    bool ret = false;

    if ( !itr )
        return ret;

    d->itr = itr;
    connect( d->itr, SIGNAL( currentParagraphDeleted() ), SLOT( slotCurrentParagraphDeleted() ) );
    d->lastTxtDocument = d->itr->currentTextObject()->textDocument();
    d->needsIncrement = false;
    ret = !d->itr->atEnd();
    d->dialog = dialog;

    return ret;
}

bool KoSpell::check( KoTextParag *parag )
{
    if ( checking() || !parag )
        return false;

    d->parag = parag;
    d->lastTxtDocument = d->parag->textDocument();

    start();

    return true;
}

bool KoSpell::checkWordInParagraph( KoTextParag *parag, int pos,
                                    QString& foundWord, int& start )
{
    if ( !parag ) {
        start = -1;
        return false;
    }

    d->parag = parag;
    d->lastTxtDocument = d->parag->textDocument();
    QString str = parag->string()->stringToSpellCheck();
    /// ##### do we really need to create a Filter every time?
    Filter filter;
    filter.setBuffer( str );
    filter.setSettings( loader()->settings() );
    Word w = filter.wordAtPosition( pos );
    foundWord = w.word;
    start = w.start;
#ifdef DEBUG_SPELL
    kDebug()<<"KoSpell: WORD IS " << w.word << ", pos = "<< pos
             << ", start = "<< w.start <<endl;
#endif
    return checkWord( w.word );
}

QString KoSpell::getMoreText()
{
#ifdef DEBUG_SPELL
    kDebug()<<"getMoreText: dialog = " << d->dialog << ", itr = "
             << d->itr << ", atEnd = "
             << ( ( d->itr ) ? d->itr->atEnd() : true )
             << " d->parag=" << d->parag
             << endl;
#endif

    if ( d->needsIncrement && d->itr && !d->itr->atEnd() ) {
        ++( *d->itr );
        if ( !d->itr->atEnd() )
            d->lastTxtDocument = d->itr->currentTextObject()->textDocument();
    }

    if ( d->itr && d->itr->atEnd() )
        return QString::null;

    if ( !d->dialog && !d->itr ) {
        QString str = d->parag ? d->parag->string()->stringToSpellCheck() : QString::null;
        if ( !str.isEmpty() )
            emit aboutToFeedText();
        return str;
    }

    d->needsIncrement = true;

    QString text = d->itr->currentText();
    d->parag = d->itr->currentParag();

    emit aboutToFeedText();
    //kDebug()<<"here 2"<<endl;
    while ( !d->dialog && d->parag ) {
        if ( d->parag->string()->needsSpellCheck() &&
             d->parag->length() > 1 )
            break;
        ++(*d->itr);
        if ( d->itr->atEnd() ) {
            d->needsIncrement = false;
            return QString::null;
        }
        d->parag = d->itr->currentParag();
        d->lastTxtDocument = d->parag->textDocument();
        text = d->itr->currentText();
    }

    d->parag->string()->setNeedsSpellCheck( false );

    return text;
}

void KoSpell::finishedCurrentFeed()
{
    emit paragraphChecked( d->parag );
}

KoTextParag  *KoSpell::currentParag() const
{
    return d->parag;
}

KoTextObject *KoSpell::currentTextObject() const
{
    if ( d->itr && !d->itr->atEnd() )
        return d->itr->currentTextObject();
    return 0;
}

int KoSpell::currentStartIndex() const
{
    if ( d->itr && !d->itr->atEnd() )
        return d->itr->currentStartIndex();
    return 0;
}

void KoSpell::slotCurrentParagraphDeleted()
{
#ifdef DEBUG_SPELL
    kDebug() << "KoSpell::slotCurrentParagraphDeleted itr=" << d->itr << endl;
#endif
    stop();
    if ( d->itr ) {
        d->needsIncrement = false;
        d->parag = d->itr->currentParag();
#ifdef DEBUG_SPELL
        kDebug() << "KoSpell::slotCurrentParagraphDeleted d->parag=" << d->parag << endl;
#endif
        start();
    } else {
        d->parag = 0;
    }
}

bool KoSpell::checking() const
{
#ifdef DEBUG_SPELL
    kDebug()<< "KoSpell::checking: itr=" << d->itr
             << ", atEnd=" << ( ( d->itr ) ? d->itr->atEnd() : false )
             << ", filter()->atEnd()=" << filter()->atEnd()
             << endl;
#endif
    if ( d->itr ) {
        if ( d->itr->atEnd() &&
             filter()->atEnd() )
            return false;
        else
            return true;
    } else
        return !filter()->atEnd();
}

KoTextDocument * KoSpell::textDocument() const
{
    return d->lastTxtDocument;
}

KSpell2::Settings * KoSpell::settings() const
{
    return loader()->settings();
}

#include "KoSpell.moc"
