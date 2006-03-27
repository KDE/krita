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

#ifndef kotextformatter_h
#define kotextformatter_h

#include "KoRichText.h"
class KoTextZoomHandler;
class KoHyphenator;

/**
 * We implement our own text formatter to implement WYSIWYG:
 * It is heavily based on KoTextFormatterBaseBreakWords, but stores the x position
 * of characters (and their width) in pixels, whereas all the rest is in L.U.
 * It also implements hyphenation.
 * @author David Faure <faure@kde.org>
 */
class KOTEXT_EXPORT KoTextFormatter : public KoTextFormatterBase
{
public:
    KoTextFormatter();
    virtual ~KoTextFormatter();

    virtual bool format( KoTextDocument *doc, KoTextParag *parag, int start, const QMap<int, KoTextParagLineStart*> &oldLineStarts, int& y, int& widthUsed );

    // Called after formatting a paragraph
    virtual void postFormat( KoTextParag* parag );

    KoHyphenator* hyphenator() {
        return m_hyphenator;
    }
private:
    KoHyphenator* m_hyphenator;
};

// Internal class for KoTextFormatter, holds all the temporary data
// KoTextFormatter is basically the settings and the algorithm being used
// KoTextFormatterCore is where the formatting really happens
class KoTextFormatterCore
{
public:
    KoTextFormatterCore( KoTextFormatter* settings, KoTextDocument *doc, KoTextParag *parag, int start );

    bool format();

    // widthUsed is the width of the wider line (with the current
    // word-breaking, margins included, but e.g. centering not included).
    // Unused in KWord currently, this is however used by KPresenter's
    // "resize object to fit contents" feature.
    int widthUsed() const { return wused; }
    int resultY() const { return y; }

protected:
    // Return ww (in LU) and pixelww (in pixels)
    // Should be called only once per char
    QPair<int, int> determineCharWidth();

    KoTextParagLineStart *koFormatLine(
        KoTextZoomHandler *zh,
        KoTextParag * /*parag*/, KoTextString *string, KoTextParagLineStart *line,
        KoTextStringChar *startChar, KoTextStringChar *lastChar, int align, int space );

    KoTextParagLineStart *koBidiReorderLine(
        KoTextZoomHandler *zh,
        KoTextParag * /*parag*/, KoTextString *text, KoTextParagLineStart *line,
        KoTextStringChar *startChar, KoTextStringChar *lastChar, int align, int space );

    void moveChar( KoTextStringChar& chr, KoTextZoomHandler *zh,
                   int deltaX, int deltaPixelX );

    // Total left margin for a given line
    // Takes into account parag's leftmargin, firstlinemargin and counter,
    // but not adjustMargins (application hook)
    int leftMargin( bool firstLine, bool includeFirstLineMargin = true ) const;
    int rightMargin( bool firstLine ) const;


private:
    KoTextFormatter* settings;
    KoTextDocument* doc;
    KoTextParag* parag;
    int start; // always 0 currently
    int wused; // see widthUsed
    int y;
    int availableWidth;
    int maxY;

    // When moving a big item down, we might want to rollback
    // to the 'best position for it' if we can't make it fit anywhere else.
    QPair<int,int> maxAvailableWidth; // first=y  second=available width

    // Information on current char
    KoTextStringChar *c;
    int i; // index number (in the paragraph)
    int x; // X position (in LU)
};

#endif
