/* This file is part of the KDE project
   Copyright (C) 2004 Zack Rusin <zack@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "KoBgSpellCheck.h"
#include "KoBgSpellCheck.moc"
#include "KoTextParag.h"

#include "KoSpell.h"

#include "KoTextObject.h"
#include "KoTextDocument.h"


#include <kspell2/backgroundchecker.h>
#include <kspell2/broker.h>
#include <kspell2/dictionary.h>
#include <kspell2/settings.h>
#include <kspell2/filter.h>
using namespace KSpell2;

#include <klocale.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <QTimer>
#include <q3ptrdict.h>

// #define DEBUG_BGSPELLCHECKING

class KoBgSpellCheck::Private
{
public:
    int marked;
    KoSpell *backSpeller;
    Q3PtrDict<KoTextParag> paragCache;
    bool startupChecking;
    KoTextParag* intraWordParag;
    int intraWordPosition;
};

static const int delayAfterMarked = 10;

KoBgSpellCheck::KoBgSpellCheck( const KSpell2::Broker::Ptr& broker, QObject *parent,
                                const char *name )
    : QObject( parent, name )
{
#ifdef DEBUG_BGSPELLCHECKING
    kDebug(32500) << "KoBgSpellCheck::KoBgSpellCheck " << this << endl;
#endif
    d = new Private;
    d->startupChecking = false;
    d->marked = 0;
    d->intraWordParag = 0;
    d->intraWordPosition = 0;
    d->backSpeller = new KoSpell( broker, this, "KoSpell" );

    connect( d->backSpeller, SIGNAL(misspelling(const QString&, int)),
             SLOT(spellCheckerMisspelling(const QString &, int )) );
    connect( d->backSpeller, SIGNAL(done()),
             SLOT(spellCheckerDone()) );
    connect( d->backSpeller, SIGNAL(aboutToFeedText()),
             SLOT(slotClearPara()) );
}

KoBgSpellCheck::~KoBgSpellCheck()
{
    delete d; d = 0;
}

void KoBgSpellCheck::registerNewTextObject( KoTextObject *obj )
{
    Q_ASSERT( obj );

    connect( obj, SIGNAL(paragraphCreated(KoTextParag*)),
             SLOT(slotParagraphCreated(KoTextParag*)) );
    connect( obj, SIGNAL(paragraphModified(KoTextParag*, int, int, int)),
             SLOT(slotParagraphModified(KoTextParag*, int, int, int)) );
    connect( obj, SIGNAL(paragraphDeleted(KoTextParag*)),
             SLOT(slotParagraphDeleted(KoTextParag*)) );
}

void KoBgSpellCheck::setEnabled( bool b )
{
    d->backSpeller->settings()->setBackgroundCheckerEnabled( b );
    if ( b )
        start();
    else
        stop();
}

bool KoBgSpellCheck::enabled() const
{
    return d->backSpeller->settings()->backgroundCheckerEnabled();
}

void KoBgSpellCheck::start()
{
    if ( !enabled() )
        return;

    d->startupChecking = true;
    d->marked = 0;
    KoTextIterator *itr = createWholeDocIterator();
    d->backSpeller->check( itr );
    d->backSpeller->start();
}

void KoBgSpellCheck::spellCheckerMisspelling( const QString &old, int pos )
{
    KoTextParag* parag = d->backSpeller->currentParag();
#ifdef DEBUG_BGSPELLCHECKING
    kDebug(32500) << "KoBgSpellCheck::spellCheckerMisspelling parag=" << parag
                   << " (id=" << parag->paragId() << ", length="
                   << parag->length() << ") pos=" << pos << " length="
                   << old.length() << endl;
#endif
    markWord( parag, pos, old.length(), true );
    // Repaint immediately, since the checking is timer-based (slow), it looks
    // slow (chunky) if we only repaint once a paragraph is completely done.
    parag->document()->emitRepaintChanged();

    if ( d->startupChecking && d->marked > delayAfterMarked ) {
        d->marked = 0;
        QTimer::singleShot( 1000, this, SLOT(checkerContinue()) );
    } else {
        if ( d->startupChecking )
            ++d->marked;
        checkerContinue();
    }
}

void KoBgSpellCheck::markWord( KoTextParag* parag, int pos, int length, bool misspelled )
{
    if ( pos >= parag->length() ) {
        kDebug(32500) << "markWord: " << pos << " is out of parag (length=" << parag->length() << ")" << endl;
        return;
    }
    if ( misspelled && parag == d->intraWordParag &&
         d->intraWordPosition >= pos &&
         d->intraWordPosition < pos+length ) {
#ifdef DEBUG_BGSPELLCHECKING
        kDebug(32500) << "markWord: " << parag << " " << pos << " to " << pos+length << " - word being edited" << endl;
#endif
        return; // not yet
    }

    KoTextStringChar *ch = parag->at( pos );
    KoTextFormat format( *ch->format() );
    format.setMisspelled( misspelled );
#ifdef DEBUG_BGSPELLCHECKING
    kDebug(32500) << "markWord: changing mark from " << pos << " length=" << length << " misspelled=" << misspelled << endl;
#endif
    parag->setFormat( pos, length, &format, true, KoTextFormat::Misspelled );
    parag->setChanged( true );
    // don't repaint here, in the slotParagraphModified case we want to repaint only once at the end
}

void KoBgSpellCheck::checkerContinue()
{
    if(enabled())
        d->backSpeller->continueChecking();
}

void KoBgSpellCheck::spellCheckerDone()
{
    d->startupChecking = false;

    if ( d->paragCache.isEmpty() )
        return;

    Q3PtrDictIterator<KoTextParag> itr( d->paragCache );
    KoTextParag *parag = d->paragCache.take( itr.currentKey() );
#ifdef DEBUG_BGSPELLCHECKING
    kDebug(32500) << "spellCheckerDone : " << parag << ", cache = "<< d->paragCache.count() <<endl;
#endif
    d->backSpeller->check( parag );
}

void KoBgSpellCheck::stop()
{
#ifdef DEBUG_BGSPELLCHECKING
  kDebug(32500) << "KoBgSpellCheck::stopSpellChecking" << endl;
#endif
  d->backSpeller->stop();
}

void KoBgSpellCheck::slotParagraphCreated( KoTextParag* parag )
{
    parag->string()->setNeedsSpellCheck( true );
    if ( !enabled() )
        return;
    if ( !d->backSpeller->check( parag ) ) {
        d->paragCache.insert( parag, parag );
    }
}

void KoBgSpellCheck::slotParagraphModified( KoTextParag* parag, int /*ParagModifyType*/,
                                            int pos, int length )
{
    parag->string()->setNeedsSpellCheck( true );
    if ( !enabled() )
        return;

    if ( d->backSpeller->checking() ) {
        d->paragCache.insert( parag, parag );
        return;
    }
#ifdef DEBUG_BGSPELLCHECKING
    kDebug(32500) << "Para modified " << parag << " pos = "<<pos<<", length = "<< length <<endl;
#endif

    if ( length < 10 ) {
        QString str = parag->string()->stringToSpellCheck();
        /// ##### do we really need to create a Filter every time?
        Filter filter;
        filter.setBuffer( str );
        // pos - 1 wasn't enough for the case a splitting a word into two misspelled halves
        filter.setCurrentPosition( qMax( 0, pos - 2 ) );
        int curPos = filter.currentPosition(); // Filter adjusted it by going back to the last word
        //kDebug() << "str='" << str << "' set position " << qMax(0, pos-2) << " got back curPos=" << curPos << endl;
        filter.setSettings( d->backSpeller->settings() );

        // Tricky: KSpell2::Filter::nextWord's behavior makes the for() loop skip ignored words,
        // so it doesn't mark them as OK... So we need to clear the marks everywhere first.
        // To avoid flickering the repainting is only done once, after checking the parag.
        markWord( parag, curPos, parag->length() - curPos, false );

        for ( Word w = filter.nextWord(); !w.end; w = filter.nextWord() ) {
            bool misspelling = !d->backSpeller->checkWord( w.word );
            //kDebug()<<"Word = \""<< w.word<< "\" , misspelled = "<<misspelling<<endl;
            markWord( parag, w.start, w.word.length(), misspelling );
        }
        if ( parag->hasChanged() ) // always true currently
            parag->document()->emitRepaintChanged();
    } else
    {
        d->backSpeller->check( parag );
    }
}

void KoBgSpellCheck::slotParagraphDeleted( KoTextParag* parag )
{
    d->paragCache.take( parag );
    if ( parag == d->intraWordParag )
        d->intraWordParag = 0;

    // don't do it here, let KoTextIterator do that after adjusting itself better...
    //if ( parag == d->backSpeller->currentParag() )
    //    d->backSpeller->slotCurrentParagraphDeleted();
}

void KoBgSpellCheck::slotClearPara()
{
    KoTextParag *parag = d->backSpeller->currentParag();

    // We remove any misspelled format from the paragraph
    // - otherwise we'd never notice words being ok again :)
    // (e.g. due to adding a word to the ignore list, not due to editing)
    //
    // TODO: do this all only if there was a format with 'misspelled' in the paragraph,
    // to minimize repaints
    KoTextStringChar *ch = parag->at( 0 );
    KoTextFormat format( *ch->format() );
    format.setMisspelled( false );
#ifdef DEBUG_BGSPELLCHECKING
    kDebug(32500) << "clearPara: resetting mark on paragraph " << parag->paragId() << endl;
#endif
    parag->setFormat( 0, parag->length()-1, &format, true,
                      KoTextFormat::Misspelled );
    parag->setChanged( true );
    parag->document()->emitRepaintChanged();
}

KSpell2::Settings * KoBgSpellCheck::settings() const
{
    return d->backSpeller->settings();
}

void KoBgSpellCheck::setIntraWordEditing( KoTextParag* parag, int index )
{
    KoTextParag* oldIntraWordParag = d->intraWordParag;
    int oldIntraWordPosition = d->intraWordPosition;

    d->intraWordParag = parag;
    d->intraWordPosition = index;

    if ( oldIntraWordParag && !parag ) {
        // When typing a letter into an existing word and then going somewhere else,
        // we need to re-check that word - after moving d->intra* out of the way of course.
        slotParagraphModified( oldIntraWordParag, 0 /*unused*/, oldIntraWordPosition, 1 );
    }
}
