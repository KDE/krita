/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "ListItemsHelper_p.h"

#include "KoTextBlockData.h"
#include "styles/KoListStyle.h"
#include "styles/KoParagraphStyle.h"

#include <kdebug.h>
#include <QTextList>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

enum Capitalisation { Lowercase, Uppercase };

static QString intToRoman( int n ) {
    static const QByteArray RNUnits[] = {"", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"};
    static const QByteArray RNTens[] = {"", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc"};
    static const QByteArray RNHundreds[] = {"", "c", "cc", "ccc", "cd", "d", "dc", "dcc", "dccc", "cm"};
    static const QByteArray RNThousands[] = {"", "m", "mm", "mmm"};

    if ( n <= 0 ) {
        kWarning(32500) << "intToRoman called with negative number: n=" << n << endl;
        return QString::number( n );
    }
    return QString::fromLatin1( RNThousands[ ( n / 1000 ) ] + RNHundreds[ ( n / 100 ) % 10 ] +
                                RNTens[ ( n / 10 ) % 10 ] + RNUnits[ ( n ) % 10 ] );
}

static QString intToAlpha( int n, Capitalisation caps ) {
    const char offset = caps == Uppercase?'A':'a';
    QString answer;
    char bottomDigit;
    while ( n > 26 ) {
        bottomDigit = (n-1) % 26;
        n = (n-1) / 26;
        answer.prepend( QChar( offset + bottomDigit  ) );
    }
    answer.prepend( QChar( offset + n -1 ) );
    return answer;
}

static QString intToScript(int n, KoListStyle::Style type) {
    // 10-base
    static const QByteArray bengali[] = { "০", "১", "২", "৩", "৪", "৫", "৬", "৭", "৮", "৯" };
    static const QByteArray gujarati[] = { "૦","૧","૨","૩","૪","૫","૬","૭","૮","૯" };
    static const QByteArray gurumukhi[] = { "੦","੧","੨","੩","੪","੫","੬","੭","੮","੯"};
    static const QByteArray kannada[] = { "೦","೧","೨","೩","೪","೫","೬","೭","೮","೯"};
    static const QByteArray malayalam[] = { "൦","൧","൨","൩","൪","൫","൬","൭","൮","൯"};
    static const QByteArray oriya[] = { "୦","୧","୨","୩","୪","୫","୬","୭","୮","୯"};
    static const QByteArray tamil[] = { "௦","௧","௨","௩","௪","௫","௬","௭","௮","௯"};
    static const QByteArray telugu[] = { "౦","౧","౨","౩","౪","౫","౬","౭","౮","౯"};
    static const QByteArray tibetan[] = { "༠","༡","༢","༣","༤","༥","༦","༧","༨","༩"};
    static const QByteArray thai[] = { "๐","๑","๒","๓","๔","๕","๖","๗","๘","๙" };
    // 1 time Sequences
    // note; the leading X is to make these 1 based.
    static const QByteArray Abjad[] = { "X", "أ", "ب", "ج", "د", "ﻫ", "و", "ز", "ح", "ط", "ي", "ك", "ل", "م",
        "ن", "س", "ع", "ف", "ص", "ق", "ر", "ش", "ت", "ث", "خ", "ذ", "ض", "ظ", "غ" };
    static const QByteArray Abjad2[] = { "X", "ﺃ", "ﺏ", "ﺝ", "ﺩ", "ﻫ", "ﻭ", "ﺯ", "ﺡ", "ﻁ", "ﻱ", "ﻙ", "ﻝ", "ﻡ",
        "ﻥ", "ﺹ", "ﻉ", "ﻑ", "ﺽ", "ﻕ", "ﺭ", "ﺱ", "ﺕ", "ﺙ", "ﺥ", "ﺫ", "ﻅ", "ﻍ", "ﺵ" };
    static const QByteArray ArabicAlphabet[] = { "X", "ا", "ب", "ت", "ث", "ج", "ح", "خ", "د", "ذ", "ر", "ز",
        "س", "ش", "ص", "ض", "ط", "ظ", "ع", "غ", "ف", "ق", "ك", "ل", "م", "ن", "ه", "و", "ي" };

/*
// see this page for the 10, 100, 1000 etc http://en.wikipedia.org/wiki/Chinese_numerals
static const char* chinese1[] = { '零','壹','貳','叄','肆','伍','陸','柒','捌','玖' };
static const char* chinese2[] = { '〇','一','二','三','四','五','六','七','八','九' };

TODO: http://en.wikipedia.org/wiki/Korean_numerals
http://en.wikipedia.org/wiki/Japanese_numerals
'http://en.wikipedia.org/wiki/Hebrew_numerals'
'http://en.wikipedia.org/wiki/Armenian_numerals'
'http://en.wikipedia.org/wiki/Greek_numerals'
'http://en.wikipedia.org/wiki/Cyrillic_numerals'
'http://en.wikipedia.org/wiki/Sanskrit_numerals'
'http://en.wikipedia.org/wiki/Ge%27ez_alphabet#Numerals'
'http://en.wikipedia.org/wiki/Abjad_numerals'
*/

    const QByteArray *numerals;
    switch(type) {
        case KoListStyle::Bengali:
            numerals = bengali;
            break;
        case KoListStyle::Gujarati:
            numerals = gujarati;
            break;
        case KoListStyle::Gurumukhi:
            numerals = gurumukhi;
            break;
        case KoListStyle::Kannada:
            numerals = kannada;
            break;
        case KoListStyle::Malayalam:
            numerals = malayalam;
            break;
        case KoListStyle::Oriya:
            numerals = oriya;
            break;
        case KoListStyle::Tamil:
            numerals = tamil;
            break;
        case KoListStyle::Telugu:
            numerals = telugu;
            break;
        case KoListStyle::Tibetan:
            numerals = tibetan;
            break;
        case KoListStyle::Thai:
            numerals = thai;
            break;
        case KoListStyle::Abjad:
            if( n > 22) return "*";
            return QString::fromUtf8(Abjad[n].data());
        case KoListStyle::AbjadMinor:
            if( n > 22) return "*";
            return QString::fromUtf8(Abjad2[n].data());
        case KoListStyle::ArabicAlphabet:
            if( n > 28) return "*";
            return QString::fromUtf8(ArabicAlphabet[n].data());
        default:
            return QString::number(n);
    }
    QString answer;
    while(n > 0) {
        answer.prepend( QString::fromUtf8(numerals[n %10].data()) );
        n = n / 10;
    }
    return answer;
}

// ------------------- ListItemsHelper ------------
/// \internal helper class for calculating text-lists prefixes and indents
ListItemsHelper::ListItemsHelper(QTextList *textList, const QFont &font)
    : m_textList(textList),
    m_fm( font, textList->document()->documentLayout()->paintDevice() ),
    m_displayFont(font)
{
}

void ListItemsHelper::recalculate() {
    //kDebug() << "ListItemsHelper::recalculate" << endl;
    double width = 0.0;
    QTextListFormat format = m_textList->format();
    const KoListStyle::Style listStyle = static_cast<KoListStyle::Style> (m_textList->format().style());

    int index = format.intProperty(KoListStyle::StartValue);
    QString prefix = format.stringProperty( KoListStyle::ListItemPrefix );
    QString suffix = format.stringProperty( KoListStyle::ListItemSuffix );
    const int level = format.intProperty(KoListStyle::Level);
    int dp = format.intProperty(KoListStyle::DisplayLevel);
    if(dp > level || dp == 0)
        dp = level;
    const int displayLevel = dp;

    for(int i=0; i < m_textList->count(); i++) {
        QTextBlock tb = m_textList->item(i);
        //kDebug() << " * " << tb.text() << endl;
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (tb.userData());
        if(!data) {
            data = new KoTextBlockData();
            tb.setUserData(data);
        }
        QTextBlockFormat blockFormat = tb.blockFormat();
        if(blockFormat.boolProperty( KoParagraphStyle::RestartListNumbering) )
            index = format.intProperty(KoListStyle::StartValue);
        const int paragIndex = blockFormat.intProperty( KoParagraphStyle::ExplicitListValue);
        if(paragIndex > 0)
            index = paragIndex;

        QTextBlock b = tb.previous();
        for(;b.isValid(); b = b.previous()) {
            if(b.textList() == m_textList)
                break; // all fine
            if(b.textList() == 0)
                continue;
            if(b.textList()->format().intProperty(KoListStyle::Level) < level) {
                index = format.intProperty(KoListStyle::StartValue);
                break;
            }
        }

        QString item("");
        if(displayLevel > 1) {
            int checkLevel = level;
            int tmpDisplayLevel = displayLevel;
            for(QTextBlock b = tb.previous(); tmpDisplayLevel > 1 && b.isValid(); b=b.previous()) {
                if(b.textList() == 0)
                    continue;
                QTextListFormat lf = b.textList()->format();
                const int otherLevel  = lf.intProperty(KoListStyle::Level);
                if(checkLevel <= otherLevel)
                    continue;
              /*if(needsRecalc(b->textList())) {
                    TODO
                } */
                KoTextBlockData *otherData = dynamic_cast<KoTextBlockData*> (b.userData());
Q_ASSERT(otherData);
                if(tmpDisplayLevel-1 < otherLevel) { // can't just copy it fully since we are
                                                  // displaying less then the full counter
                    item += otherData->partialCounterText();
                    tmpDisplayLevel--;
                    checkLevel--;
                    for(int i=otherLevel+1;i < level; i++) {
                        tmpDisplayLevel--;
                        item += ".0"; // add missing counters.
                    }
                }
                else { // just copy previous counter as prefix
                    item += otherData->counterText();
                    for(int i=otherLevel+1;i < level; i++)
                        item += ".0"; // add missing counters.
                    break;
                }
            }
        }
        if((listStyle == KoListStyle::DecimalItem || listStyle == KoListStyle::AlphaLowerItem ||
                    listStyle == KoListStyle::UpperAlphaItem ||
                    listStyle == KoListStyle::RomanLowerItem ||
                    listStyle == KoListStyle::UpperRomanItem) &&
                !(item.isEmpty() || item.endsWith('.') || item.endsWith(' '))) {
            item += '.';
        }
        bool calcWidth=true;
        QString partialCounterText;
        switch( listStyle ) {
            case KoListStyle::DecimalItem:
                partialCounterText = QString::number(index);
                break;
            case KoListStyle::AlphaLowerItem:
                partialCounterText = intToAlpha(index, Lowercase);
                break;
            case KoListStyle::UpperAlphaItem:
                partialCounterText = intToAlpha(index, Uppercase);
                break;
            case KoListStyle::RomanLowerItem:
                partialCounterText = intToRoman(index);
                break;
            case KoListStyle::UpperRomanItem:
                partialCounterText = intToRoman(index).toUpper();
                break;
            case KoListStyle::SquareItem:
            case KoListStyle::DiscItem:
            case KoListStyle::CircleItem:
            case KoListStyle::BoxItem: {
                calcWidth = false;
                item = ' ';
                width = m_displayFont.pointSizeF();
                int percent = format.intProperty(KoListStyle::BulletSize);
                if(percent > 0)
                    width = width * (percent / 100.0);
                break;
            }
            case KoListStyle::KoListStyle::CustomCharItem:
                calcWidth = false;
                item = QString(QChar(format.intProperty(KoListStyle::BulletCharacter)));
                width = m_fm.width(item);
                break;
            case KoListStyle::KoListStyle::NoItem:
                calcWidth = false;
                width =  10.0; // simple indenting
                break;
            case KoListStyle::Bengali:
            case KoListStyle::Gujarati:
            case KoListStyle::Gurumukhi:
            case KoListStyle::Kannada:
            case KoListStyle::Malayalam:
            case KoListStyle::Oriya:
            case KoListStyle::Tamil:
            case KoListStyle::Telugu:
            case KoListStyle::Tibetan:
            case KoListStyle::Thai:
            case KoListStyle::Abjad:
            case KoListStyle::ArabicAlphabet:
            case KoListStyle::AbjadMinor:
                partialCounterText = intToScript(index, listStyle);
                break;
            default:  // others we ignore.
                calcWidth = false;
        }
        data->setPartialCounterText(partialCounterText);
        item += partialCounterText;
        if(calcWidth)
            width = qMax(width, m_fm.width(item));
        data->setCounterText(prefix + item + suffix);
        index++;
    }
    double counterSpacing = m_fm.width(' ');
    width += counterSpacing + m_fm.width(prefix + suffix); // same for all
    for(int i=0; i < m_textList->count(); i++) {
        QTextBlock tb = m_textList->item(i);
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (tb.userData());
        data->setCounterWidth(width);
        data->setCounterSpacing(counterSpacing);
        //kDebug() << data->counterText() << " " << tb.text() << endl;
        //kDebug() << "    setCounterWidth: " << width << endl;
    }
    //kDebug() << endl;
}

// static
bool ListItemsHelper::needsRecalc(QTextList *textList) {
    Q_ASSERT(textList);
    QTextBlock tb = textList->item(0);
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (tb.userData());
    if(data == 0)
        return true;
    return !data->hasCounterData();
}
