/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include "KoTextFormatter.h"
#include "KoTextParag.h"
#include "KoTextFormat.h"
#include "KoTextDocument.h"
#include "KoTextZoomHandler.h"
#include "kohyphen/kohyphen.h"
#include "KoParagCounter.h"

#include <kdebug.h>
#include <assert.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>

//#define DEBUG_FORMATTER

// Vertical info (height, baseline etc.)
//#define DEBUG_FORMATTER_VERT

// Line and paragraph width
//#define DEBUG_FORMATTER_WIDTH

// Hyphenation
//#define DEBUG_HYPHENATION

/////// keep in sync with kotextformat.cc !
//#define REF_IS_LU

KoTextFormatter::KoTextFormatter()
{
    try {
        m_hyphenator = KoHyphenator::self();
    } catch ( KoHyphenatorException& e )
    {
        m_hyphenator = 0L;
    }
}

KoTextFormatter::~KoTextFormatter()
{
}

// Hyphenation can break anywhere in the word, so
// remember the temp data for every char.
struct TemporaryWordData
{
    int baseLine;
    int height;
    int lineWidth; // value of wused
};

bool KoTextFormatter::format( KoTextDocument *doc, KoTextParag *parag,
                              int start, const QMap<int, KoTextParagLineStart*> &,
                              int& y, int& widthUsed )
{
    KoTextFormatterCore formatter( this, doc, parag, start );
    bool worked = formatter.format();
    y = formatter.resultY();
    widthUsed = formatter.widthUsed();
    return worked;
}

KoTextFormatterCore::KoTextFormatterCore( KoTextFormatter* _settings,
                                          KoTextDocument *_doc, KoTextParag *_parag,
                                          int _start )
    : settings(_settings), doc(_doc), parag(_parag), start(_start)
{
}

QPair<int, int> KoTextFormatterCore::determineCharWidth()
{
    int ww, pixelww;
    KoTextZoomHandler *zh = doc->formattingZoomHandler();
    if ( c->c != '\t' || c->isCustom() ) {
        KoTextFormat *charFormat = c->format();
        if ( c->isCustom() ) {
            ww = c->customItem()->width;
            Q_ASSERT( ww >= 0 );
            ww = qMax(0, ww);
#ifndef REF_IS_LU
            pixelww = zh->layoutUnitToPixelX( ww );
#endif
        } else {
            ww = charFormat->charWidthLU( c, parag, i );
#ifndef REF_IS_LU
            // Pixel size - we want the metrics of the font that's going to be used.
            pixelww = charFormat->charWidth( zh, true, c, parag, i );
#endif
        }
    } else { // tab
        int nx = parag->nextTab( i, x, availableWidth );
        if ( nx < x )
            ww = availableWidth - x;
        else
            ww = nx - x;
#ifdef DEBUG_FORMATTER
        kDebug(32500) << "nextTab for x=" << x << " returned nx=" << nx << "  (=> ww=" << ww << ")" << endl;
#endif
#ifndef REF_IS_LU
        pixelww = zh->layoutUnitToPixelX( ww );
#endif
    }
    Q_ASSERT( ww >= 0 );
    c->width = ww;
    return qMakePair(ww, pixelww);
}


int KoTextFormatterCore::leftMargin( bool firstLine, bool includeFirstLineMargin /* = true */ ) const
{
    int left = /*doc ?*/ parag->leftMargin() + doc->leftMargin() /*: 0*/;
    if ( firstLine && !parag->string()->isRightToLeft() )
    {
        if ( includeFirstLineMargin )
            left += parag->firstLineMargin();
        // Add the width of the paragraph counter - first line of parag only.
        if( parag->counter() &&
            ( parag->counter()->alignment() == Qt::AlignLeft ||
              parag->counter()->alignment() == Qt::AlignLeft ) )
            left += parag->counterWidth(); // in LU pixels
    }
    return left;
}

int KoTextFormatterCore::rightMargin( bool firstLine ) const
{
    int right = parag->rightMargin(); // 'rm' in QRT
    if ( /*doc &&*/ firstLine && parag->string()->isRightToLeft() )
        right += parag->firstLineMargin();
    return right;
}

bool KoTextFormatterCore::format()
{
    start = 0; // we don't do partial formatting yet
    KoTextString *string = parag->string();
    if ( start == 0 )
        c = &string->at( start );
    else
        c = 0;

    KoTextStringChar *firstChar = 0;
    int left = doc ? leftMargin( true, false ) : 0;
    int initialLMargin = leftMargin( true );

    y = doc && doc->addMargins() ? parag->topMargin() : 0;
    // #57555, top margin doesn't apply if parag at top of page
    // (but a portion of the margin can be needed, to complete the prev page)
    // So we apply formatVertically() on the top margin, to find where to break it.
    if ( !parag->prev() )
        y = 0; // no top margin on very first parag
    else if ( parag->breakableTopMargin() )
    {
        int shift = doc->flow()->adjustFlow( parag->rect().y(),
                                             0 /*w, unused*/,
                                             parag->breakableTopMargin() );
        if ( shift > 0 )
        {
            // The shift is in fact the amount of top-margin that should remain
            // The remaining portion should be eaten away.
            y = shift;
        }

    }
    // Now add the rest of the top margin (e.g. the one for the border)
    y += parag->topMargin() - parag->breakableTopMargin();
    int len = parag->length();

    int initialHeight = c->height(); // remember what adjustMargins was called with

    int currentRightMargin = rightMargin( true );
    int initialRMargin = currentRightMargin;
    // Those three things must be done before calling determineCharWidth
    i = start;
    parag->tabCache().clear();
    x = 0;

    // We need the width of the first char for adjustMargins
    // The result might not be 100% accurate when using a tab (it'll use x=0
    // but with counters/margins this might be different). This is why
    // we call determineCharWidth() again from within the loop.
    QPair<int, int> widths = determineCharWidth();
    int ww = widths.first; // width in layout units
#ifndef REF_IS_LU
    int pixelww = widths.second; // width in pixels
#endif

    // dw is the document width, i.e. the maximum available width, all included.
    // We are in a variable-width design, so it is returned by each call to adjustMargins.
    int dw = 0;
    //if (doc) // always true in kotext
    doc->flow()->adjustMargins( y + parag->rect().y(), initialHeight, // input params
                                ww, initialLMargin, initialRMargin, dw,  // output params
                                parag );
    //else dw = parag->documentVisibleWidth();

    x = initialLMargin; // as modified by adjustMargins

    int maxY = doc ? doc->flow()->availableHeight() : -1;

    availableWidth = dw - initialRMargin; // 'w' in QRT
#if defined(DEBUG_FORMATTER) || defined(DEBUG_FORMATTER_WIDTH)
    kDebug(32500) << "KoTextFormatter::format formatting parag " << parag->paragId()
                   << " text:" << parag->string()->toString() << "\n"
                   << " left=" << left << " initialHeight=" << initialHeight << " initialLMargin=" << initialLMargin << " initialRMargin=" << initialRMargin << " availableWidth=" << availableWidth << " maxY=" << maxY << endl;
#else
    if ( availableWidth == 0 )
        kDebug(32500) << "KoTextFormatter::format " << parag->paragId() << " warning, availableWidth=0" << endl;
    if ( maxY == 0 )
        kDebug(32500) << "KoTextFormatter::format " << parag->paragId() << " warning, maxY=0" << endl;
#endif
    bool fullWidth = TRUE;
    //int marg = left + initialRMargin;

    // minw is the really minimum width needed for this paragraph, i.e.
    // the width of the longest set of non-breakable characters together.
    // Currently unused.
    //int minw = 0;

    wused = 0;

    Q3ValueList<TemporaryWordData> tempWordData;

#ifdef DEBUG_FORMATTER
    kDebug(32500) << "Initial KoTextParagLineStart at y=" << y << endl;
#endif
    KoTextParagLineStart *lineStart = new KoTextParagLineStart( y, 0, 0 );
    parag->insertLineStart( 0, lineStart );
    int lastBreak = -1;
    // tmph, tmpBaseLine and tminw are used after the last breakable char
    // we don't know yet if we'll break there, or later.
    int tmpBaseLine = 0, tmph = 0;
    //int tminw = marg;
    int tmpWused = 0;
    bool lastWasNonInlineCustom = FALSE;
    bool abort = false;

    int align = parag->alignment();
    if ( align == Qt::AlignLeft && doc && doc->alignment() != Qt::AlignLeft )
        align = doc->alignment();

    int col = 0;

    maxAvailableWidth = qMakePair( 0, 0 );

    KoTextZoomHandler *zh = doc->formattingZoomHandler();
    int pixelx = zh->layoutUnitToPixelX( x );
    int lastPixelx = 0;

    KoTextStringChar* lastChr = 0;
    for ( ; i < len; ++i, ++col ) {
        if ( c )
            lastChr = c;
        c = &string->at( i );
        if ( i > 0 && (x > initialLMargin || ww == 0) || lastWasNonInlineCustom ) {
            c->lineStart = 0;
        } else {
            c->lineStart = 1;
            firstChar = c;
            tmph = c->height();
            tmpBaseLine = c->ascent();
#ifdef DEBUG_FORMATTER_VERT
            kDebug(32500) << "New line, initializing tmpBaseLine=" << tmpBaseLine << " tmph=" << tmph << endl;
#endif
        }

        if ( c->isCustom() && c->customItem()->placement() != KoTextCustomItem::PlaceInline )
            lastWasNonInlineCustom = TRUE;
        else
            lastWasNonInlineCustom = FALSE;

        QPair<int, int> widths = determineCharWidth();
        ww = widths.first;
        pixelww = widths.second;

        // We're "aborting" the formatting. This still means we need to set the
        // lineStart bools to false (trouble ahead, otherwise!), and while we're at
        // it we also calculate the widths etc.
        if ( abort ) {
            x += ww;
            c->x = x;
            continue; // yeah, this seems a bit confusing :)
        }

        //code from qt-3.1beta2
        if ( c->isCustom() && c->customItem()->ownLine() ) {
#ifdef DEBUG_FORMATTER
            kDebug(32500) << "i=" << i << "/" << len << " custom item with ownline" << endl;
#endif
            int rightMargin = currentRightMargin;
            x = left;
            if ( doc )
                doc->flow()->adjustMargins( y + parag->rect().y(), parag->rect().height(), 15,
                                            x, rightMargin, dw, parag );
            int w = dw - rightMargin;
            c->customItem()->resize( w - x );
            y += lineStart->h;
            lineStart = new KoTextParagLineStart( y, c->ascent(), c->height() );
            // Added for kotext (to be tested)
            lineStart->lineSpacing = doc ? parag->calculateLineSpacing( (int)parag->lineStartList().count()-1, i, i ) : 0;
            lineStart->h += lineStart->lineSpacing;
            lineStart->w = dw;
            parag->insertLineStart( i, lineStart );
            tempWordData.clear();
            c->lineStart = 1;
            firstChar = c;
            x = 0xffffff;
            // Hmm, --i or setting lineStart on next char too?
            continue;
        }

#ifdef DEBUG_FORMATTER
        kDebug(32500) << "c='" << QString(c->c) << "' i=" << i << "/" << len << " x=" << x << " ww=" << ww << " availableWidth=" << availableWidth << " (test is x+ww>aW) lastBreak=" << lastBreak << " isBreakable=" << settings->isBreakable(string, i) << endl;
#endif
        // Wrapping at end of line - one big if :)
        if (
             // Check if should break (i.e. we are after the max X for the end of the line)
             ( /*wrapAtColumn() == -1 &&*/ x + ww > availableWidth &&
               ( lastBreak != -1 || settings->allowBreakInWords() ) )

             // Allow two breakable chars next to each other (e.g. '  ') but not more
             && ( !settings->isBreakable( string, i ) ||
                  ( i > 1 && lastBreak == i-1 && settings->isBreakable( string, i-2 ) ) ||
                  lastBreak == -2 ) // ... used to be a special case...

             // No point in breaking just for the trailing space (testcase: page numbers in TOC)
             && ( i < len-1 )

             // Ensure that there is at least one char per line, otherwise, on
             // a very narrow document and huge chars, we could loop forever.
             // checkVerticalBreak takes care of moving down the lines where no
             // char should be, anyway.
             // Hmm, it doesn't really do so. To be continued...
             /////////// && ( firstChar != c )

             // Or maybe we simply encountered a '\n'
             || lastChr->c == '\n' && parag->isNewLinesAllowed() && lastBreak > -1 )
        {
#ifdef DEBUG_FORMATTER
            kDebug(32500) << "BREAKING" << endl;
#endif
            //if ( wrapAtColumn() != -1 )
            //    minw = qMax( minw, x + ww );

            bool hyphenated = false;
            // Hyphenation: check if we can break somewhere between lastBreak and i
            if ( settings->hyphenator() && !c->isCustom() )
            {
                int wordStart = qMax(0, lastBreak+1);
                // Breaking after i isn't possible, i is too far already
                int maxlen = i - wordStart; // we can't accept to break after maxlen
                QString word = string->mid( wordStart, maxlen );
                int wordEnd = i;
                // but we need to compose the entire word, to hyphenate it
                while ( wordEnd < len && !settings->isBreakable( string, wordEnd ) ) {
                    word += string->at(wordEnd).c;
                    wordEnd++;
                }
                if ( word.length() > 1 ) // don't call the hyphenator for empty or one-letter words
                {
                    QString lang = string->at(wordStart).format()->language();
                    char * hyphens = settings->hyphenator()->hyphens( word, lang );
#if defined(DEBUG_HYPHENATION)
                    kDebug(32500) << "Hyphenation: word=" << word << " lang=" << lang << " hyphens=" << hyphens << " maxlen=" << maxlen << endl;
                    kDebug(32500) << "Parag indexes: wordStart=" << wordStart << " lastBreak=" << lastBreak << " i=" << i << endl;
#endif
                    int hylen = strlen(hyphens);
                    Q_ASSERT( maxlen <= hylen );
                    // If this word was already hyphenated (at the previous line),
                    // don't break it there again. We can only break after firstChar.
                    int minPos = qMax( 0, int(firstChar - &string->at(0)) - wordStart );

                    // Check hyphenation positions from the end
                    for ( int hypos = maxlen-1 ; hypos >= minPos ; --hypos )
                        if ( ( hyphens[hypos] % 2 ) // odd number -> can break there...
                               && string->at(hypos + wordStart).format()->hyphenation() ) // ...if the user is ok with that
                        {
                            lineStart->hyphenated = true;
                            lastBreak = hypos + wordStart;
                            hyphenated = true;
#if defined(DEBUG_FORMATTER) || defined(DEBUG_FORMATTER_WIDTH) || defined(DEBUG_HYPHENATION)
                            kDebug(32500) << "Hyphenation: will break at " << lastBreak << " using tempworddata at position " << hypos << "/" << tempWordData.size() << endl;
#endif
                            if ( hypos < (int)tempWordData.size() )
                            {
                                const TemporaryWordData& twd = tempWordData[ hypos ];
                                lineStart->baseLine = twd.baseLine;
                                lineStart->h = twd.height;
                                tmpWused = twd.lineWidth;
                            }
                            break;
                        }
                    delete[] hyphens;
                }
            }

            // No breakable char found -> break at current char (i.e. before 'i')
            if ( lastBreak < 0 ) {
                // Remember if this is the start of a line; testing c->lineStart after breaking
                // is always true...
                const bool emptyLine = c->lineStart;
                if ( emptyLine ) // ouch, empty line
                {
                    // This happens when there is a very wide character (e.g. inline table),
                    // or a very narrow passage between frames. In the second case we'll
                    // have more room below, in the first case we might not.
                    // So we look below, and come back if we don't find better.

                    // Remember where the biggest availableWidth was, so that if we really
                    // find nowhere for this big character, we'll come back here.
                    if ( availableWidth > maxAvailableWidth.second )
                    {
                        maxAvailableWidth.first = y;
                        maxAvailableWidth.second = availableWidth;
                    }
                    // Check if we're at the bottom of the doc, we won't find better then
                    // (and the check further down would abort)
                    if ( ( maxY > -1 && parag->rect().y() + y >= maxY ) || tmph <= 0 )
                    {
                        // Here we have to distinguish "table wider than document" (#112269)
                        // and "not enough room for chars next to a big frame".
                        // In the first case a new page wouldn't help, in the second it would.
                        // ### Real fix: asking for the max width on a not-yet-created next page.
                        // For now I just approximate that with the flow width.
                        if ( c->width >= doc->flow()->width() )
                        {
                            // OK we go back to where there was most width for it.
                            kDebug(32500) << parag->rect().y() + y << " over maxY=" << maxY
                                           << " -> final choice for the line: y=" << maxAvailableWidth.first << endl;
                            y = maxAvailableWidth.first;
                            if ( availableWidth )
                                Q_ASSERT( maxAvailableWidth.second != 0 );
                            lineStart->y = y;
                            maxAvailableWidth = qMakePair( 0, 0 ); // clear it
                        }
                        else
                        {
                            // "small" chars and not enough width here, abort and hope for a new page.
#ifdef DEBUG_FORMATTER
                            if ( tmph <= 0 )
                                kDebug(32500) << "Line has a height of " << tmph << ", let's stop." << endl;
                            else
                                kDebug(32500) << "We're after maxY, time to stop." << endl;
#endif
                            // No solution for now. Hopefully KWord will create more pages...
                            abort = true;
                        }
                    }
                    else
                    {
                        // We don't know yet what to do with this line that needs to go down
                        // Ideally KWTextFrameSet would tell us how much we need to move
                        // ("validHeight" idea). For now we keep the old behavior:
                        y += tmph;
                        kDebug(32500) << "KoTextFormatter: moving down empty line by h=" << tmph << ": y=" << y << endl;

                        --i; // so that the ++i in for() is a noop
                        continue;
                    }
                }
                if ( !emptyLine && i > 0 )
                {
                    // (combine lineStart->baseLine/lineStart->h and tmpBaseLine/tmph)
                    int belowBaseLine = qMax( lineStart->h - lineStart->baseLine, tmph - tmpBaseLine );
                    lineStart->baseLine = qMax( (int)lineStart->baseLine, tmpBaseLine );
                    lineStart->h = lineStart->baseLine + belowBaseLine;
                    lineStart->w = dw;

                    KoTextParagLineStart *lineStart2 = koFormatLine( zh, parag, string, lineStart, firstChar, c-1, align, availableWidth - x );
                    y += lineStart->h;
                    lineStart = lineStart2;
#ifdef DEBUG_FORMATTER
                    int linenr = parag->lineStartList().count()-1;
                    kDebug(32500) << "line " << linenr << " done (breaking at current char). y now " << y << endl;
#endif
                    tmph = c->height();

                    initialRMargin = currentRightMargin;
                    x = left;
                    if ( doc )
                        doc->flow()->adjustMargins( y + parag->rect().y(), tmph,
                                                    ww, // ## correct?
                                                    x, initialRMargin, dw, parag );

                    pixelx = zh->layoutUnitToPixelX( x );
                    initialHeight = tmph;
                    initialLMargin = x;
                    availableWidth = dw - initialRMargin;
                    if ( parag->isNewLinesAllowed() && c->c == '\t' ) {
                        int nx = parag->nextTab( i, x, availableWidth );
                        if ( nx < x )
                            ww = availableWidth - x;
                        else
                            ww = nx - x;
                    }
                    if ( x != left || availableWidth != dw )
                        fullWidth = FALSE;
                    lineStart->y = y;
                    parag->insertLineStart( i, lineStart );
                    tempWordData.clear();
                    lineStart->baseLine = c->ascent();
                    lineStart->h = c->height();
                    c->lineStart = 1;
                    firstChar = c;
                    tmpBaseLine = lineStart->baseLine;
                    lastBreak = -1;
                    col = 0;
                    //tminw = marg; // not in QRT?
                    tmpWused = 0;
                }
                // recalc everything for 'i', it might still not be ok where it is...
                // (e.g. if there's no room at all on this line)
                // But we don't want to do this forever, so we check against maxY (if known)
                // [except if we come here after "final choice for empty line"!]
                if ( !emptyLine && maxY > -1 )
                {
                    if ( parag->rect().y() + y < maxY )
                    {
#ifdef DEBUG_FORMATTER
                        kDebug(32500) << "Re-checking formatting for character " << i << endl;
#endif
                        --i; // so that the ++i in for() is a noop
                        continue;
                    }
                    else // we're after maxY, time to stop.
                    {
#ifdef DEBUG_FORMATTER
                        kDebug(32500) << "We're after maxY, time to stop." << endl;
#endif
                        // No solution for now. Hopefully KWord will create more pages...
                        abort = true;
                    }
                }
                // maxY not known (or "final choice for empty line") -> keep going ('i' remains where it is)
                // (in case of maxY not known, this is the initial QRT behaviour)
            } else {
                // If breaking means we're after maxY, then we won't do it.
                // Hopefully KWord will create more pages.
                if ( maxY > -1 && parag->rect().y() + y + lineStart->h >= maxY ) {
#ifdef DEBUG_FORMATTER
                    kDebug(32500) << "We're after maxY, time to stop." << endl;
#endif
                    abort = true;
                }
                else
                {
                    // Break the line at the last breakable character
                    i = lastBreak;
                    c = &string->at( i ); // The last char in the last line
                    int spaceAfterLine = availableWidth - c->x;
                    // ?? AFAICS we should always deduce the char's width from the available space....
                    //if ( string->isRightToLeft() && lastChr->c == '\n' )
                    spaceAfterLine -= c->width;

                    //else
                    if ( c->c.unicode() == 0xad || hyphenated ) // soft hyphen or hyphenation
                    {
                        // Recalculate its width, the hyphen will appear finally (important for the parag rect)
                        int width = KoTextZoomHandler::ptToLayoutUnitPt( c->format()->refFontMetrics().width( QChar(0xad) ) );
                        if ( c->c.unicode() == 0xad )
                            c->width = width;
                        spaceAfterLine -= width;
                    }
                    KoTextParagLineStart *lineStart2 = koFormatLine( zh, parag, string, lineStart, firstChar, c, align, spaceAfterLine );
                    lineStart->w = dw;
                    y += lineStart->h;
                    lineStart = lineStart2;
#ifdef DEBUG_FORMATTER
                    kDebug(32500) << "Breaking at a breakable char (" << i << "). linenr=" << parag->lineStartList().count()-1 << " y=" << y << endl;
#endif

                    c = &string->at( i + 1 ); // The first char in the new line
#ifdef DEBUG_FORMATTER
                    kDebug(32500) << "Next line will start at i+1=" << i+1 << ", char=" << QString(c->c) << endl;
#endif
                    tmph = c->height();

                    initialRMargin = currentRightMargin;
                    x = left;
                    if ( doc )
                        doc->flow()->adjustMargins( y + parag->rect().y(), tmph,
                                                    c->width,
                                                    x, initialRMargin, dw, parag );

                    pixelx = zh->layoutUnitToPixelX( x );
                    initialHeight = tmph;
                    initialLMargin = x;
                    availableWidth = dw - initialRMargin;
                    if ( x != left || availableWidth != dw )
                        fullWidth = FALSE;
                    lineStart->y = y;
                    parag->insertLineStart( i + 1, lineStart );
                    tempWordData.clear();
                    lineStart->baseLine = c->ascent();
                    lineStart->h = c->height();
                    firstChar = c;
                    tmpBaseLine = lineStart->baseLine;
                    lastBreak = -1;
                    col = 0;
                    //tminw = marg;
                    tmpWused = 0;
                    c->lineStart = 1; // only do this if we will actually create a line for it
                    continue;
                }
            }
        } else if ( lineStart && ( settings->isBreakable( string, i ) || parag->isNewLinesAllowed() && c->c == '\n' ) ) {
            // Breakable character
            if ( len <= 2 || i < len - 1 ) {
#ifdef DEBUG_FORMATTER_VERT
                kDebug(32500) << " Breakable character (i=" << i << " len=" << len << "):"
                               << " combining " << tmpBaseLine << "/" << tmph
                               << " with " << c->ascent() << "/" << c->height() << endl;
#endif
                // (combine tmpBaseLine/tmph and this character)
                int belowBaseLine = qMax( tmph - tmpBaseLine, c->height() - c->ascent() );
                tmpBaseLine = qMax( tmpBaseLine, c->ascent() );
                tmph = tmpBaseLine + belowBaseLine;
#ifdef DEBUG_FORMATTER_VERT
                kDebug(32500) << " -> tmpBaseLine/tmph : " << tmpBaseLine << "/" << tmph << endl;
#endif
            }
            tempWordData.clear();
            //minw = qMax( minw, tminw );
            //tminw = marg + ww;
            wused = qMax( wused, tmpWused );
#ifdef DEBUG_FORMATTER_WIDTH
            kDebug(32500) << " Breakable character (i=" << i << " len=" << len << "): wused=" << wused << endl;
#endif
            tmpWused = 0;
            // (combine lineStart and tmpBaseLine/tmph)
#ifdef DEBUG_FORMATTER_VERT
            kDebug(32500) << "Breakable character: combining " << lineStart->baseLine << "/" << lineStart->h << " with " << tmpBaseLine << "/" << tmph << endl;
#endif
            int belowBaseLine = qMax( lineStart->h - lineStart->baseLine, tmph - tmpBaseLine );
            lineStart->baseLine = qMax( (int)lineStart->baseLine, tmpBaseLine );
            lineStart->h = lineStart->baseLine + belowBaseLine;
            lineStart->w = dw;
#ifdef DEBUG_FORMATTER_VERT
            kDebug(32500) << " -> line baseLine/height : " << lineStart->baseLine << "/" << lineStart->h << endl;
#endif
            // if h > initialHeight,  call adjustMargins, and if the result is != initial[LR]Margin,
            // format this line again
            if ( doc && lineStart->h > initialHeight )
            {
                bool firstLine = ( firstChar == &string->at( 0 ) );
                int newLMargin = leftMargin( firstLine );
                int newRMargin = rightMargin( firstLine );
                int newPageWidth = dw;
                initialHeight = lineStart->h;
                doc->flow()->adjustMargins( y + parag->rect().y(), initialHeight,
                                            firstChar->width,
                                            newLMargin, newRMargin, newPageWidth, parag );

#ifdef DEBUG_FORMATTER
                kDebug(32500) << "new height: " << initialHeight << " => left=" << left << " first-char=" << (firstChar==&string->at(0)) << " newLMargin=" << newLMargin << " newRMargin=" << newRMargin << endl;
#endif
                if ( newLMargin != initialLMargin || newRMargin != initialRMargin || newPageWidth != dw )
                {
#ifdef DEBUG_FORMATTER
                    kDebug(32500) << "formatting again" << endl;
#endif
                    i = (firstChar - &string->at(0));
                    x = newLMargin;
                    pixelx = zh->layoutUnitToPixelX( x );
                    availableWidth = dw - newRMargin;
                    initialLMargin = newLMargin;
                    initialRMargin = newRMargin;
                    dw = newPageWidth;
                    c = &string->at( i );
                    tmph = c->height();
                    tmpBaseLine = c->ascent();
                    lineStart->h = tmph;
                    lineStart->baseLine = tmpBaseLine;
                    lastBreak = -1;
                    col = 0;
                    //minw = x;
#ifdef DEBUG_FORMATTER
                    kDebug(32500) << "Restarting with i=" << i << " x=" << x << " y=" << y << " tmph=" << tmph << " initialHeight=" << initialHeight << " initialLMargin=" << initialLMargin << " initialRMargin=" << initialRMargin << " y=" << y << endl;
#endif
                    // ww and pixelww already calculated and stored, no need to duplicate
                    // code like QRT does.
                    ww = c->width;
#ifndef REF_IS_LU
                    pixelww = c->pixelwidth;
#endif
                    //tminw = x + ww;
                    tmpWused = 0;
                }
            }

            //kDebug(32500) << " -> lineStart->baseLine/lineStart->h : " << lineStart->baseLine << "/" << lineStart->h << endl;
            if ( i < len - 2 || c->c != ' ' )
                lastBreak = i;

        } else if ( i < len - 1 ) { // ignore height of trailing space
            // Non-breakable character
            //tminw += ww;
#ifdef DEBUG_FORMATTER_VERT
            kDebug(32500) << " Non-breakable character: combining " << tmpBaseLine << "/" << tmph << " with " << c->ascent() << "/" << c->height() << endl;
#endif
            // (combine tmpBaseLine/tmph and this character)
            int belowBaseLine = qMax( tmph - tmpBaseLine, c->height() - c->ascent() );
            tmpBaseLine = qMax( tmpBaseLine, c->ascent() );
            tmph = tmpBaseLine + belowBaseLine;
#ifdef DEBUG_FORMATTER_VERT
            kDebug(32500) << " -> tmpBaseLine/tmph : " << tmpBaseLine << "/" << tmph << endl;
#endif

            TemporaryWordData twd;
            twd.baseLine = tmpBaseLine;
            twd.height = tmph;
            twd.lineWidth = tmpWused;
            tempWordData.append( twd );
        }

        c->x = x;
        // pixelxadj is the adjustement to add to lu2pixel(x), to find pixelx
        // (pixelx would be too expensive to store directly since it would require an int)
        c->pixelxadj = pixelx - zh->layoutUnitToPixelX( x );
        //c->pixelwidth = pixelww; // done as pixelx - lastPixelx below
#ifdef DEBUG_FORMATTER
        kDebug(32500) << "LU: x=" << x << " [equiv. to pix=" << zh->layoutUnitToPixelX( x ) << "] ; PIX: x=" << pixelx << "  --> adj=" << c->pixelxadj << endl;
#endif

        x += ww;

        if ( i > 0 )
            lastChr->pixelwidth = pixelx - lastPixelx;
        if ( i < len - 1 )
            tmpWused = qMax( tmpWused, x );
        else // trailing space
            c->pixelwidth = zh->layoutUnitToPixelX( ww ); // was: pixelww;

        lastPixelx = pixelx;
#ifdef REF_IS_LU
        pixelx = zh->layoutUnitToPixelX( x ); // no accumulating rounding errors anymore
#else
        pixelx += pixelww;
#endif
#ifdef DEBUG_FORMATTER
        kDebug(32500) << "LU: added " << ww << " -> now x=" << x << " ; PIX: added " << pixelww << " -> now pixelx=" << pixelx << endl;
#endif
    }

    // ### hack. The last char in the paragraph is always invisible, and somehow sometimes has a wrong format. It changes between
    // layouting and printing. This corrects some layouting errors in BiDi mode due to this.
    if ( len > 1 /*&& !c->isAnchor()*/ ) {
        c->format()->removeRef();
        c->setFormat( string->at( len - 2 ).format() );
        c->format()->addRef();
    }

    // Finish formatting the last line
    if ( lineStart ) {
#ifdef DEBUG_FORMATTER
        kDebug(32500) << "Last Line.... linenr=" << (int)parag->lineStartList().count()-1 << endl;
#endif
#ifdef DEBUG_FORMATTER_VERT
        kDebug(32500) << "Last Line... Combining " << lineStart->baseLine << "/" << lineStart->h << " with " << tmpBaseLine << "/" << tmph << endl;
#endif
        // (combine lineStart and tmpBaseLine/tmph)
        int belowBaseLine = qMax( lineStart->h - lineStart->baseLine, tmph - tmpBaseLine );
        lineStart->baseLine = qMax( (int)lineStart->baseLine, tmpBaseLine );
        lineStart->h = lineStart->baseLine + belowBaseLine;
        lineStart->w = dw;
#ifdef DEBUG_FORMATTER_WIDTH
        kDebug(32500) << "Last line: w = dw = " << dw << endl;
#endif
#ifdef DEBUG_FORMATTER_VERT
        kDebug(32500) << " -> lineStart->baseLine/lineStart->h : " << lineStart->baseLine << "/" << lineStart->h << endl;
#endif
        // last line in a paragraph is not justified
        if ( align == Qt::AlignJustify )
            align = Qt::AlignLeft;
        int space = availableWidth - x + c->width; // don't count the trailing space (it breaks e.g. centering)
        KoTextParagLineStart *lineStart2 = koFormatLine( zh, parag, string, lineStart, firstChar, c, align, space );
        delete lineStart2;
    }

    //minw = qMax( minw, tminw );
    wused = qMax( wused, tmpWused );
#ifdef DEBUG_FORMATTER_WIDTH
    kDebug(32500) << "Done, wused=" << wused << endl;
#endif

    int m = parag->bottomMargin();
    // ##### Does OOo add margins or does it max them?
    //if ( parag->next() && doc && !doc->addMargins() )
    //  m = qMax( m, parag->next()->topMargin() );
    parag->setFullWidth( fullWidth );
    //if ( parag->next() && parag->next()->isLineBreak() )
    //    m = 0;
#ifdef DEBUG_FORMATTER_VERT
    kDebug(32500) << "Adding height of last line(" << lineStart->h << ") and bottomMargin(" << m << ") to y(" << y << ") => " << y+lineStart->h+m << endl;
#endif
    y += lineStart->h + m;

    tmpWused += currentRightMargin; // ### this can break with a variable right-margin
    //if ( wrapAtColumn() != -1  )
    //    minw = qMax(minw, wused);
    //thisminw = minw;

#ifdef DEBUG_FORMATTER
    // Sanity checking
    int numberOfLines = 0;
    QString charPosList;
    for ( int i = 0 ; i < len; ++i ) {
        KoTextStringChar *chr = &string->at( i );
        if ( i == 0 )
            assert( chr->lineStart );
        if ( chr->lineStart ) {
            ++numberOfLines;
            charPosList += QString::number(i) + " ";
        }
    }
    kDebug(32500) << parag->lineStartList().count() << " lines. " << numberOfLines << " chars with lineStart set: " << charPosList << endl;
    assert( numberOfLines == (int)parag->lineStartList().count() );
#endif
    return !abort;
}

// Helper for koFormatLine and koBidiReorderLine
void KoTextFormatterCore::moveChar( KoTextStringChar& chr, KoTextZoomHandler *zh,
                                    int deltaX, int deltaPixelX )
{
#ifndef REF_IS_LU
    int pixelx = chr.pixelxadj + zh->layoutUnitToPixelX( chr.x );
#endif
    chr.x += deltaX;
#ifndef REF_IS_LU
    chr.pixelxadj = pixelx + deltaPixelX - zh->layoutUnitToPixelX( chr.x );
#endif
}

KoTextParagLineStart *KoTextFormatterCore::koFormatLine(
    KoTextZoomHandler *zh,
    KoTextParag *parag, KoTextString *string, KoTextParagLineStart *line,
    KoTextStringChar *startChar, KoTextStringChar *lastChar, int align, int space )
{
    KoTextParagLineStart* ret = 0;
#if 0 // TODO RTL SUPPORT
    if( string->isBidi() ) {
        ret = koBidiReorderLine( zh, parag, string, line, startChar, lastChar, align, space );
    } else
#endif
    {
        int start = (startChar - &string->at(0));
        int last = (lastChar - &string->at(0) );

        if (space < 0)
            space = 0;

        // do alignment Auto == Left in this case
        if ( align & Qt::AlignHCenter || align & Qt::AlignRight ) {
            if ( align & Qt::AlignHCenter )
                space /= 2;
            int toAddPix = zh->layoutUnitToPixelX( space );
            for ( int j = last; j >= start; --j ) {
                KoTextStringChar &chr = string->at( j );
                moveChar( chr, zh, space, toAddPix );
            }
        } else if ( align & Qt::AlignJustify ) {
            int numSpaces = 0;
            // End at "last-1", the last space ends up with a width of 0
            for ( int j = last-1; j >= start; --j ) {
                //// Start at last tab, if any. BR #40472 specifies that justifying should start after the last tab.
                if ( string->at( j ).c == '\t' ) {
                    start = j+1;
                    break;
                }
                if( settings->isStretchable( string, j ) ) {
                    numSpaces++;
                }
            }
            int toAdd = 0;
            int toAddPix = 0;
            for ( int k = start + 1; k <= last; ++k ) {
                KoTextStringChar &chr = string->at( k );
                if ( toAdd != 0 )
                    moveChar( chr, zh, toAdd, toAddPix );
                if( settings->isStretchable( string, k ) && numSpaces ) {
                    int s = space / numSpaces;
                    toAdd += s;
                    toAddPix = zh->layoutUnitToPixelX( toAdd );
                    space -= s;
                    numSpaces--;
                    chr.width += s;
#ifndef REF_IS_LU
                    chr.pixelwidth += zh->layoutUnitToPixelX( s ); // ### rounding problem, recalculate
#endif
                }
            }
        }
        int current=0;
        int nc=0; // Not double, as we check it against 0 and to avoid gcc warnings
        KoTextFormat refFormat( *string->at(0).format() ); // we need a ref format, doesn't matter where it comes from
        for(int i=start;i<=last;++i)
        {
            KoTextFormat* format=string->at(i).format();
            // End of underline
            if ( (((!format->underline())&&
                   (!format->doubleUnderline())&&
                   (!format->waveUnderline())&&
                   (format->underlineType()!=KoTextFormat::U_SIMPLE_BOLD))
                  || i == last)
                 && nc )
            {
                double avg=static_cast<double>(current)/nc;
                avg/=18.0;
                // Apply underline width "avg" from i-nc to i
                refFormat.setUnderLineWidth( avg );
                parag->setFormat( i-nc, i, &refFormat, true, KoTextFormat::UnderLineWidth );
                nc=0;
                current=0;
            }
            // Inside underline
            else if(format->underline()||
                    format->waveUnderline()||
                    format->doubleUnderline()||
                    (format->underlineType() == KoTextFormat::U_SIMPLE_BOLD))
            {
                ++nc;
                current += format->pointSize(); //pointSize() is independent of {Sub,Super}Script in contrast to height()
            }
        }
#if 0
        if ( last >= 0 && last < string->length() ) {
            KoTextStringChar &chr = string->at( last );
            line->w = chr.x + chr.width; //string->width( last );
            // Add width of hyphen (so that it appears)
            if ( line->hyphenated )
                line->w += KoTextZoomHandler::ptToLayoutUnitPt( chr.format()->refFontMetrics().width( QChar(0xad) ) );
        } else
            line->w = 0;
#endif

        ret = new KoTextParagLineStart();
    }

    // Now calculate and add linespacing
    const int start = (startChar - &string->at(0));
    const int last = (lastChar - &string->at(0) );
    line->lineSpacing = parag->calculateLineSpacing( (int)parag->lineStartList().count()-1, start, last );
    line->h += line->lineSpacing;

    return ret;
}

// TODO RTL SUPPORT
#if 0
// collects one line of the paragraph and transforms it to visual order
KoTextParagLineStart *KoTextFormatterCore::koBidiReorderLine(
    KoTextZoomHandler *zh,
    KoTextParag * /*parag*/, KoTextString *text, KoTextParagLineStart *line,
    KoTextStringChar *startChar, KoTextStringChar *lastChar, int align, int space )
{
    // This comes from Qt (3.3.x) but seems wrong: the last space is where we draw
    // the "end of paragraph" sign, so it needs to be correctly positioned too.
#if 0
    // ignore white space at the end of the line.
    int endSpaces = 0;
    while ( lastChar > startChar && lastChar->whiteSpace ) {
        space += lastChar->format()->width( ' ' );
        --lastChar;
        ++endSpaces;
    }
#endif

    int start = (startChar - &text->at(0));
    int last = (lastChar - &text->at(0) );
#ifdef DEBUG_FORMATTER
    kDebug(32500) << "*KoTextFormatter::koBidiReorderLine from " << start << " to " << last << " space=" << space << " startChar->x=" << startChar->x << endl;
#endif
    KoBidiControl *control = new KoBidiControl( line->context(), line->status );
    QString str;
    str.setUnicode( 0, last - start + 1 );
    // fill string with logically ordered chars.
    KoTextStringChar *ch = startChar;
    QChar *qch = (QChar *)str.unicode();
    while ( ch <= lastChar ) {
        *qch = ch->c;
        qch++;
        ch++;
    }
    int x = startChar->x;

    Q3PtrList<KoTextRun> *runs;
    runs = KoComplexText::bidiReorderLine(control, str, 0, last - start + 1,
                                         (text->isRightToLeft() ? QChar::DirR : QChar::DirL) );

    // now construct the reordered string out of the runs...

    int numSpaces = 0;
    // set the correct alignment. This is a bit messy....
    if( align == Qt::AlignLeft ) {
        // align according to directionality of the paragraph...
        if ( text->isRightToLeft() )
            align = Qt::AlignRight;
    }

    if ( align & Qt::AlignHCenter ) {
        x += space/2;
    } else if ( align & Qt::AlignRight ) {
        x += space;
    } else if ( align & Qt::AlignJustify ) {
        for ( int j = last - 1; j >= start; --j ) {
            //// Start at last tab, if any. BR #40472 specifies that justifying should start after the last tab.
            if ( text->at( j ).c == '\t' ) {
                start = j+1;
                break;
            }
            if( settings->isStretchable( text, j ) ) {
                numSpaces++;
            }
        }
    }
// TODO #ifndef REF_IS_LU or remove
    int pixelx = zh->layoutUnitToPixelX( x );
    int toAdd = 0;
    int toAddPix = 0;
    bool first = TRUE;
    KoTextRun *r = runs->first();
    int xmax = -0xffffff;
    while ( r ) {
#ifdef DEBUG_FORMATTER
        kDebug(32500) << "koBidiReorderLine level: " << r->level << endl;
#endif
        if(r->level %2) {
            // odd level, need to reverse the string
            int pos = r->stop + start;
            while(pos >= r->start + start) {
                KoTextStringChar &chr = text->at(pos);
                if( numSpaces && !first && settings->isBreakable( text, pos ) ) {
                    int s = space / numSpaces;
                    toAdd += s;
                    toAddPix = zh->layoutUnitToPixelX( toAdd );
                    space -= s;
                    numSpaces--;
                    chr.width += s;
                    chr.pixelwidth += zh->layoutUnitToPixelX( s ); // ### rounding problem, recalculate
                } else if ( first ) {
                    first = FALSE;
                    if ( chr.c == ' ' ) // trailing space
                    {
                        //x -= chr.format()->width( ' ' );
                        x -= chr.width;
                        pixelx -= chr.pixelwidth;
                    }
                }
                chr.x = x + toAdd;
                chr.pixelxadj = pixelx + toAddPix - zh->layoutUnitToPixelX( chr.x );
#ifdef DEBUG_FORMATTER
                kDebug(32500) << "koBidiReorderLine: pos=" << pos << " x(LU)=" << x << " toAdd(LU)=" << toAdd << " -> chr.x=" << chr.x << " pixelx=" << pixelx << "+" << zh->layoutUnitToPixelX( toAdd ) << ", pixelxadj=" << pixelx+zh->layoutUnitToPixelX( toAdd )-zh->layoutUnitToPixelX( chr.x ) << endl;
#endif
                chr.rightToLeft = TRUE;
                chr.startOfRun = FALSE;
                int ww = chr.width;
                if ( xmax < x + toAdd + ww ) xmax = x + toAdd + ww;
                x += ww;
                pixelx += chr.pixelwidth;
#ifdef DEBUG_FORMATTER
                kDebug(32500) << "              ww=" << ww << " adding to x, now " << x << ". pixelwidth=" << chr.pixelwidth << " adding to pixelx, now " << pixelx << " xmax=" << xmax << endl;
#endif
                pos--;
            }
        } else {
            int pos = r->start + start;
            while(pos <= r->stop + start) {
                KoTextStringChar& chr = text->at(pos);
                if( numSpaces && !first && settings->isBreakable( text, pos ) ) {
                    int s = space / numSpaces;
                    toAdd += s;
                    toAddPix = zh->layoutUnitToPixelX( toAdd );
                    space -= s;
                    numSpaces--;
                } else if ( first ) {
                    first = FALSE;
                    if ( chr.c == ' ' ) // trailing space
                    {
                        //x -= chr.format()->width( ' ' );
                        x -= chr.width;
                        pixelx -= chr.pixelwidth;
                    }
                }
                chr.x = x + toAdd;
                chr.pixelxadj = pixelx + toAddPix - zh->layoutUnitToPixelX( chr.x );
                chr.rightToLeft = FALSE;
                chr.startOfRun = FALSE;
                int ww = chr.width;
                //kDebug(32500) << "setting char " << pos << " at pos " << chr.x << endl;
                if ( xmax < x + toAdd + ww ) xmax = x + toAdd + ww;
                x += ww;
                pixelx += chr.pixelwidth;
                pos++;
            }
        }
        text->at( r->start + start ).startOfRun = TRUE;
        r = runs->next();
    }

    //line->w = xmax /*+ 10*/; // Why +10 ?
    KoTextParagLineStart *ls = new KoTextParagLineStart( control->context, control->status );
    delete control;
    delete runs;
    return ls;
}
#endif

void KoTextFormatter::postFormat( KoTextParag* parag )
{
    parag->fixParagWidth( viewFormattingChars() );
}
