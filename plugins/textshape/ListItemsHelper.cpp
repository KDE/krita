/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "ListItemsHelper.h"

#include <KoTextBlockData.h>
#include <KoListStyle.h>
#include <KoParagraphStyle.h>
#include <KoTextDocument.h>
#include <KoList.h>

#include <KDebug>
#include <KLocale>
#include <QTextList>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
using namespace Lists;

QString Lists::intToRoman(int n)
{
    static const QByteArray RNUnits[] = {"", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"};
    static const QByteArray RNTens[] = {"", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc"};
    static const QByteArray RNHundreds[] = {"", "c", "cc", "ccc", "cd", "d", "dc", "dcc", "dccc", "cm"};
    static const QByteArray RNThousands[] = {"", "m", "mm", "mmm"};

    if (n <= 0) {
        kWarning(32500) << "intToRoman called with negative number: n=" << n;
        return QString::number(n);
    }
    return QString::fromLatin1(RNThousands[(n / 1000)] + RNHundreds[(n / 100) % 10 ] +
                               RNTens[(n / 10) % 10 ] + RNUnits[(n) % 10 ]);
}

QString Lists::intToAlpha(int n, Capitalisation caps, bool letterSynchronization)
{
    const char offset = caps == Uppercase ? 'A' : 'a';
    QString answer;
    if (letterSynchronization) {
        int digits = 1;
        for (; n > 26; n -= 26)
            digits += 1;
        for (int i = 0; i < digits; i++)
            answer.prepend(QChar(offset + n - 1));
        return answer;
    } else {
        char bottomDigit;
        while (n > 26) {
            bottomDigit = (n - 1) % 26;
            n = (n - 1) / 26;
            answer.prepend(QChar(offset + bottomDigit));
        }
    }
    answer.prepend(QChar(offset + n - 1));
    return answer;
}

QString Lists::intToScript(int n, KoListStyle::Style type)
{
    // 10-base
    static const int bengali = 0x9e6;
    static const int gujarati = 0xae6;
    static const int gurumukhi = 0xa66;
    static const int kannada = 0xce6;
    static const int malayalam = 0xd66;
    static const int oriya = 0xb66;
    static const int tamil = 0x0be6;
    static const int telugu = 0xc66;
    static const int tibetan = 0xf20;
    static const int thai = 0xe50;

    int offset;
    switch (type) {
    case KoListStyle::Bengali:
        offset = bengali;
        break;
    case KoListStyle::Gujarati:
        offset = gujarati;
        break;
    case KoListStyle::Gurumukhi:
        offset = gurumukhi;
        break;
    case KoListStyle::Kannada:
        offset = kannada;
        break;
    case KoListStyle::Malayalam:
        offset = malayalam;
        break;
    case KoListStyle::Oriya:
        offset = oriya;
        break;
    case KoListStyle::Tamil:
        offset = tamil;
        break;
    case KoListStyle::Telugu:
        offset = telugu;
        break;
    case KoListStyle::Tibetan:
        offset = tibetan;
        break;
    case KoListStyle::Thai:
        offset = thai;
        break;
    default:
        return QString::number(n);
    }
    QString answer;
    while (n > 0) {
        answer.prepend(QChar(offset + n % 10));
        n = n / 10;
    }
    return answer;
}

QString Lists::intToScriptList(int n, KoListStyle::Style type)
{
    // 1 time Sequences
    // note; the leading X is to make these 1 based.
    static const char* Abjad[] = { "أ", "ب", "ج", "د", "ﻫ", "و", "ز", "ح", "ط", "ي", "ك", "ل", "م",
                                   "ن", "س", "ع", "ف", "ص", "ق", "ر", "ش", "ت", "ث", "خ", "ذ", "ض", "ظ", "غ"
                                 };
    static const char* Abjad2[] = { "ﺃ", "ﺏ", "ﺝ", "ﺩ", "ﻫ", "ﻭ", "ﺯ", "ﺡ", "ﻁ", "ﻱ", "ﻙ", "ﻝ", "ﻡ",
                                    "ﻥ", "ﺹ", "ﻉ", "ﻑ", "ﺽ", "ﻕ", "ﺭ", "ﺱ", "ﺕ", "ﺙ", "ﺥ", "ﺫ", "ﻅ", "ﻍ", "ﺵ"
                                  };
    static const char* ArabicAlphabet[] = {"ا", "ب", "ت", "ث", "ج", "ح", "خ", "د", "ذ", "ر", "ز",
                                           "س", "ش", "ص", "ض", "ط", "ظ", "ع", "غ", "ف", "ق", "ك", "ل", "م", "ن", "ه", "و", "ي"
                                          };

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

    switch (type) {
    case KoListStyle::Abjad:
        if (n > 22) return "*";
        return QString::fromUtf8(Abjad[n-1]);
    case KoListStyle::AbjadMinor:
        if (n > 22) return "*";
        return QString::fromUtf8(Abjad2[n-1]);
    case KoListStyle::ArabicAlphabet:
        if (n > 28) return "*";
        return QString::fromUtf8(ArabicAlphabet[n-1]);
    default:
        return QString::number(n);
    }
}

QList<ListStyleItem> Lists::genericListStyleItems()
{
    QList<ListStyleItem> answer;
    answer.append(ListStyleItem(i18nc("Text list-style", "None"), KoListStyle::None));
    answer.append(ListStyleItem(i18n("Arabic"), KoListStyle::DecimalItem));
    answer.append(ListStyleItem(i18n("Lower Alphabetical"), KoListStyle::AlphaLowerItem));
    answer.append(ListStyleItem(i18n("Upper Alphabetical"), KoListStyle::UpperAlphaItem));
    answer.append(ListStyleItem(i18n("Lower Roman"), KoListStyle::RomanLowerItem));
    answer.append(ListStyleItem(i18n("Upper Roman"), KoListStyle::UpperRomanItem));
    answer.append(ListStyleItem(i18n("Disc Bullet"), KoListStyle::DiscItem));
    answer.append(ListStyleItem(i18n("Square Bullet"), KoListStyle::SquareItem));
    answer.append(ListStyleItem(i18n("Box Bullet"), KoListStyle::BoxItem));
    answer.append(ListStyleItem(i18n("Rhombus Bullet"), KoListStyle::RhombusItem));
    answer.append(ListStyleItem(i18n("Circle Bullet"), KoListStyle::CircleItem));
    answer.append(ListStyleItem(i18n("Check Mark Bullet"), KoListStyle::HeavyCheckMarkItem));
    answer.append(ListStyleItem(i18n("Ballot X Bullet"), KoListStyle::BallotXItem));
    answer.append(ListStyleItem(i18n("Rightwards Arrow Bullet"), KoListStyle::RightArrowItem));
    answer.append(ListStyleItem(i18n("Rightwards Arrow Head Bullet"), KoListStyle::RightArrowHeadItem));
    return answer;
}

QList<ListStyleItem> Lists::otherListStyleItems()
{
    QList<ListStyleItem> answer;
    answer.append(ListStyleItem(i18n("Bengali"), KoListStyle::Bengali));
    answer.append(ListStyleItem(i18n("Gujarati"), KoListStyle::Gujarati));
    answer.append(ListStyleItem(i18n("Gurumukhi"), KoListStyle::Gurumukhi));
    answer.append(ListStyleItem(i18n("Kannada"), KoListStyle::Kannada));
    answer.append(ListStyleItem(i18n("Malayalam"), KoListStyle::Malayalam));
    answer.append(ListStyleItem(i18n("Oriya"), KoListStyle::Oriya));
    answer.append(ListStyleItem(i18n("Tamil"), KoListStyle::Tamil));
    answer.append(ListStyleItem(i18n("Telugu"), KoListStyle::Telugu));
    answer.append(ListStyleItem(i18n("Tibetan"), KoListStyle::Tibetan));
    answer.append(ListStyleItem(i18n("Thai"), KoListStyle::Thai));
    answer.append(ListStyleItem(i18n("Abjad"), KoListStyle::Abjad));
    answer.append(ListStyleItem(i18n("AbjadMinor"), KoListStyle::AbjadMinor));
    answer.append(ListStyleItem(i18n("ArabicAlphabet"), KoListStyle::ArabicAlphabet));
    return answer;
}

// ------------------- ListItemsHelper ------------
/// \internal helper class for calculating text-lists prefixes and indents
ListItemsHelper::ListItemsHelper(QTextList *textList, const QFont &font)
        : m_textList(textList),
        m_fm(font, textList->document()->documentLayout()->paintDevice()),
        m_displayFont(font)
{
}

void ListItemsHelper::recalculate()
{
    //kDebug(32500);
    const QTextListFormat format = m_textList->format();
    const KoListStyle::Style listStyle = static_cast<KoListStyle::Style>(m_textList->format().style());

    const QString prefix = format.stringProperty(KoListStyle::ListItemPrefix);
    const QString suffix = format.stringProperty(KoListStyle::ListItemSuffix);
    const int level = format.intProperty(KoListStyle::Level);
    int dp = format.intProperty(KoListStyle::DisplayLevel);
    if (dp > level)
        dp = level;
    const int displayLevel = dp ? dp : 1;

    int startValue = 1;
    if (format.hasProperty(KoListStyle::StartValue))
        startValue = format.intProperty(KoListStyle::StartValue);
    if (format.boolProperty(KoListStyle::ContinueNumbering)) {
        // Look for the index of a previous list of the same numbering style and level
        for (QTextBlock tb = m_textList->item(0).previous(); tb.isValid(); tb = tb.previous()) {
            if (!tb.textList() || tb.textList() == m_textList)
                continue; // no list here or it's the same list; keep looking

            QTextListFormat otherFormat = tb.textList()->format();
            if (otherFormat.intProperty(KoListStyle::Level) != level)
                break; // found a different list but of a different level

            if (otherFormat.style() == format.style()) {
                if (KoTextBlockData *data = dynamic_cast<KoTextBlockData *>(tb.userData()))
                    startValue = data->counterIndex() + 1; // Start from previous list value + 1
            }

            break;
        }
    }

    int index = startValue;
    QList<QTextList*> sublistsToRecalculate;
    qreal width = format.doubleProperty(KoListStyle::MinimumWidth);
    for (int i = 0; i < m_textList->count(); i++) {
        QTextBlock tb = m_textList->item(i);
        //kDebug(32500) <<" *" << tb.text();
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(tb.userData());
        if (!data) {
            data = new KoTextBlockData();
            tb.setUserData(data);
        }
        QTextBlockFormat blockFormat = tb.blockFormat();

        if (blockFormat.boolProperty(KoParagraphStyle::UnnumberedListItem)
            || blockFormat.boolProperty(KoParagraphStyle::IsListHeader)) {
            data->setCounterText(QString());
            data->setPartialCounterText(QString());
            continue;
        }

        if (blockFormat.boolProperty(KoParagraphStyle::RestartListNumbering))
            index = format.intProperty(KoListStyle::StartValue);
        const int paragIndex = blockFormat.intProperty(KoParagraphStyle::ListStartValue);
        if (paragIndex > 0)
            index = paragIndex;

        QTextBlock b = tb.previous();
        for (;b.isValid(); b = b.previous()) {
            if (b.textList() == m_textList)
                break; // all fine
            if (b.textList() == 0)
                continue;
            if (b.textList()->format().intProperty(KoListStyle::Level) < level) {
                index = startValue;
                break;
            }
        }

        QString item;
        if (displayLevel > 1) {
            int checkLevel = level;
            int tmpDisplayLevel = displayLevel;
            for (QTextBlock b = tb.previous(); tmpDisplayLevel > 1 && b.isValid(); b = b.previous()) {
                if (b.textList() == 0)
                    continue;
                QTextListFormat lf = b.textList()->format();
                const int otherLevel  = lf.intProperty(KoListStyle::Level);
                if (checkLevel <= otherLevel)
                    continue;
                /*if(needsRecalc(b->textList())) {
                      TODO
                  } */
                KoTextBlockData *otherData = dynamic_cast<KoTextBlockData*>(b.userData());
                if (! otherData) {
                    kWarning(32500) << "Missing KoTextBlockData, Skipping textblock";
                    continue;
                }
                if (tmpDisplayLevel - 1 < otherLevel) { // can't just copy it fully since we are
                    // displaying less then the full counter
                    item += otherData->partialCounterText();
                    tmpDisplayLevel--;
                    checkLevel--;
                    for (int i = otherLevel + 1; i < level; i++) {
                        tmpDisplayLevel--;
                        item += ".1"; // add missing counters.
                    }
                } else { // just copy previous counter as prefix
                    QString otherPrefix = lf.stringProperty(KoListStyle::ListItemPrefix);
                    QString otherSuffix = lf.stringProperty(KoListStyle::ListItemSuffix);
                    QString pureCounter = otherData->counterText().mid(otherPrefix.size());
                    pureCounter = pureCounter.left(pureCounter.size() - otherSuffix.size());
                    item += pureCounter;
                    for (int i = otherLevel + 1; i < level; i++)
                        item += ".1"; // add missing counters.
                    tmpDisplayLevel = 0;
                    break;
                }
            }
            for (int i = 1; i < tmpDisplayLevel; i++)
                item = "1." + item; // add missing counters.
        }

        if ((listStyle == KoListStyle::DecimalItem || listStyle == KoListStyle::AlphaLowerItem ||
                listStyle == KoListStyle::UpperAlphaItem ||
                listStyle == KoListStyle::RomanLowerItem ||
                listStyle == KoListStyle::UpperRomanItem) &&
                !(item.isEmpty() || item.endsWith('.') || item.endsWith(' '))) {
            item += '.';
        }
        bool calcWidth = true;
        QString partialCounterText;
        switch (listStyle) {
        case KoListStyle::DecimalItem:
            partialCounterText = QString::number(index);
            break;
        case KoListStyle::AlphaLowerItem:
            partialCounterText = intToAlpha(index, Lowercase,
                                            m_textList->format().boolProperty(KoListStyle::LetterSynchronization));
            break;
        case KoListStyle::UpperAlphaItem:
            partialCounterText = intToAlpha(index, Uppercase,
                                            m_textList->format().boolProperty(KoListStyle::LetterSynchronization));
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
        case KoListStyle::HeavyCheckMarkItem:
        case KoListStyle::BallotXItem:
        case KoListStyle::RightArrowItem:
        case KoListStyle::RightArrowHeadItem:
        case KoListStyle::RhombusItem:
        case KoListStyle::BoxItem: {
            calcWidth = false;
            item = ' ';
            width = m_displayFont.pointSizeF();
            int percent = format.intProperty(KoListStyle::BulletSize);
            if (percent > 0)
                width = width * (percent / 100.0);
            break;
        }
        case KoListStyle::CustomCharItem:
            calcWidth = false;
            if (format.intProperty(KoListStyle::BulletCharacter))
                item = QString(QChar(format.intProperty(KoListStyle::BulletCharacter)));
            width = m_fm.width(item);
            break;
        case KoListStyle::None:
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
            partialCounterText = intToScript(index, listStyle);
            break;
        case KoListStyle::Abjad:
        case KoListStyle::ArabicAlphabet:
        case KoListStyle::AbjadMinor:
            partialCounterText = intToScriptList(index, listStyle);
            break;
        case KoListStyle::ImageItem:
            calcWidth = false;
            width = qMax(format.doubleProperty(KoListStyle::Width), (qreal)1.0);
            break;
        default:  // others we ignore.
            calcWidth = false;
        }

        data->setCounterIsImage(listStyle == KoListStyle::ImageItem);
        data->setPartialCounterText(partialCounterText);
        data->setCounterIndex(index);
        item += partialCounterText;
        if (calcWidth)
            width = qMax(width, m_fm.width(item));
        data->setCounterText(prefix + item + suffix);
        index++;

        // have to recalculate any sublists under this element too
        QTextBlock nb = tb.next();
        while (nb.isValid() && nb.textList() == 0)
            nb = nb.next();
        if (nb.isValid()) {
            if (nb.textList()->format().intProperty(KoListStyle::Level) > level) {
                // this is a sublist
                // have to remember to recalculate this list after the current level is done
                // cant do it right away since the sublist's prefix text is dependant on this level
                sublistsToRecalculate.append(nb.textList());
            }
        }
    }

    for (int i = 0; i < sublistsToRecalculate.count(); i++) {
        ListItemsHelper lih(sublistsToRecalculate.at(i), m_displayFont);
        lih.recalculate();
    }

    qreal counterSpacing = m_fm.width(' ');
    counterSpacing = qMax(format.doubleProperty(KoListStyle::MinimumDistance), counterSpacing);
    width += m_fm.width(prefix + suffix); // same for all
    width = qMax(format.doubleProperty(KoListStyle::MinimumWidth), width);
    for (int i = 0; i < m_textList->count(); i++) {
        QTextBlock tb = m_textList->item(i);
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(tb.userData());
        Q_ASSERT(data);
        data->setCounterWidth(width);
        data->setCounterSpacing(counterSpacing);
        //kDebug(32500) << data->counterText() <<"" << tb.text();
        //kDebug(32500) <<"    setCounterWidth:" << width;
    }
    //kDebug(32500);
}

// static
bool ListItemsHelper::needsRecalc(QTextList *textList)
{
    Q_ASSERT(textList);
    QTextBlock tb = textList->item(0);
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(tb.userData());
    if (data == 0)
        return true;
    return !data->hasCounterData();
}
