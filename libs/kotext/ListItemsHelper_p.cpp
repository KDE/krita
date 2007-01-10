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

    int index = format.intProperty(KoListStyle::StartValue);
    QString prefix = format.stringProperty( KoListStyle::ListItemPrefix );
    QString suffix = format.stringProperty( KoListStyle::ListItemSuffix );
    const int level = format.intProperty(KoListStyle::Level);
    int dp = format.intProperty(KoListStyle::DisplayLevel);
    if(dp > level || dp == 0)
        dp = level;
    const int displayLevel = dp;
// TODO define item and width only once for the non changing list styles
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
        KoListStyle::Style listStyle = static_cast<KoListStyle::Style> (
                m_textList->format().style() );
        if((listStyle == KoListStyle::DecimalItem || listStyle == KoListStyle::AlphaLowerItem ||
                    listStyle == KoListStyle::UpperAlphaItem ||
                    listStyle == KoListStyle::RomanLowerItem ||
                    listStyle == KoListStyle::UpperRomanItem) &&
                !(item.isEmpty() || item.endsWith('.') || item.endsWith(' '))) {
            item += '.';
        }
        switch( listStyle ) {
            case KoListStyle::DecimalItem: {
                QString i = QString::number(index);
                data->setPartialCounterText(i);
                item += i;
                width = qMax(width, m_fm.width(item));
                break;
            }
            case KoListStyle::AlphaLowerItem:
                item = intToAlpha(index, Lowercase);
                width = qMax(width, m_fm.width(item));
                break;
            case KoListStyle::UpperAlphaItem:
                item = intToAlpha(index, Uppercase);
                width = qMax(width, m_fm.width(item));
                break;
            case KoListStyle::RomanLowerItem:
                item = intToRoman(index);
                width = qMax(width, m_fm.width(item));
                break;
            case KoListStyle::UpperRomanItem:
                item = intToRoman(index).toUpper();
                width = qMax(width, m_fm.width(item));
                break;
            case KoListStyle::SquareItem:
            case KoListStyle::DiscItem:
            case KoListStyle::CircleItem:
            case KoListStyle::BoxItem: {
                item = ' ';
                width = m_displayFont.pointSizeF();
                int percent = format.intProperty(KoListStyle::BulletSize);
                if(percent > 0)
                    width = width * (percent / 100.0);
                break;
            }
            case KoListStyle::CustomCharItem:
                item = QString(QChar(format.intProperty(KoListStyle::BulletCharacter)));
                width = m_fm.width(item + ' ');
                break;
            case KoListStyle::NoItem:
                width =  10.0; // simple indenting
                break;
            default:; // others we ignore.
        }
        data->setCounterText(prefix + item + suffix);
        index++;
    }
    double counterSpacing = 1;
    if(suffix.isEmpty())
        counterSpacing = m_fm.width(' ');
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
