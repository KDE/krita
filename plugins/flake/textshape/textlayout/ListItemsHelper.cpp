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
#include <KoParagraphStyle.h>
#include <KoTextDocument.h>
#include <KoList.h>

#include <TextLayoutDebug.h>
#include <klocalizedstring.h>
#include <QTextList>

using namespace Lists;

QString Lists::intToRoman(int n)
{
    static const QString RNUnits[] = {"", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"};
    static const QString RNTens[] = {"", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc"};
    static const QString RNHundreds[] = {"", "c", "cc", "ccc", "cd", "d", "dc", "dcc", "dccc", "cm"};
    static const QString RNThousands[] = {"", "m", "mm", "mmm", "mmmm", "mmmmm", "mmmmmm", "mmmmmmm", "mmmmmmmm", "mmmmmmmmm"};

    if (n <= 0) {
        warnTextLayout << "intToRoman called with negative number: n=" << n;
        return QString::number(n);
    }

    return RNThousands[(n / 1000)] +
           RNHundreds[(n / 100) % 10 ] +
           RNTens[(n / 10) % 10 ] +
           RNUnits[(n) % 10 ];
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
    static const char* const Abjad[] = { "أ", "ب", "ج", "د", "ﻫ", "و", "ز", "ح", "ط", "ي", "ك", "ل", "م",
                                   "ن", "س", "ع", "ف", "ص", "ق", "ر", "ش", "ت", "ث", "خ", "ذ", "ض", "ظ", "غ"
                                 };
    static const char* const Abjad2[] = { "ﺃ", "ﺏ", "ﺝ", "ﺩ", "ﻫ", "ﻭ", "ﺯ", "ﺡ", "ﻁ", "ﻱ", "ﻙ", "ﻝ", "ﻡ",
                                    "ﻥ", "ﺹ", "ﻉ", "ﻑ", "ﺽ", "ﻕ", "ﺭ", "ﺱ", "ﺕ", "ﺙ", "ﺥ", "ﺫ", "ﻅ", "ﻍ", "ﺵ"
                                  };
    static const char* const ArabicAlphabet[] = {"ا", "ب", "ت", "ث", "ج", "ح", "خ", "د", "ذ", "ر", "ز",
                                           "س", "ش", "ص", "ض", "ط", "ظ", "ع", "غ", "ف", "ق", "ك", "ل", "م", "ن", "ه", "و", "ي"
                                          };

    /*
    // see this page for the 10, 100, 1000 etc http://en.wikipedia.org/wiki/Chinese_numerals
    static const char* const chinese1[] = { '零','壹','貳','叄','肆','伍','陸','柒','捌','玖' };
    static const char* const chinese2[] = { '〇','一','二','三','四','五','六','七','八','九' };

    TODO: https://en.wikipedia.org/wiki/Korean_numerals
    https://en.wikipedia.org/wiki/Japanese_numerals
    'https://en.wikipedia.org/wiki/Hebrew_numerals'
    'https://en.wikipedia.org/wiki/Armenian_numerals'
    'https://en.wikipedia.org/wiki/Greek_numerals'
    'https://en.wikipedia.org/wiki/Cyrillic_numerals'
    'https://en.wikipedia.org/wiki/Sanskrit_numerals'
    'https://en.wikipedia.org/wiki/Ge%27ez_alphabet#Numerals'
    'https://en.wikipedia.org/wiki/Abjad_numerals'
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

QString Lists::intToNumberingStyle(int index, KoListStyle::Style listStyle, bool letterSynchronization)
{
    QString counterText;
    switch(listStyle) {
    case KoListStyle::DecimalItem:
        counterText = QString::number(index);
        break;
    case KoListStyle::AlphaLowerItem:
        counterText = intToAlpha(index, Lowercase, letterSynchronization);
        break;
    case KoListStyle::UpperAlphaItem:
        counterText = intToAlpha(index, Uppercase, letterSynchronization);
        break;
    case KoListStyle::RomanLowerItem:
        counterText = intToRoman(index);
        break;
    case KoListStyle::UpperRomanItem:
        counterText = intToRoman(index).toUpper();
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
        counterText = intToScript(index, listStyle);
        break;
    case KoListStyle::Abjad:
    case KoListStyle::ArabicAlphabet:
    case KoListStyle::AbjadMinor:
        counterText = intToScriptList(index, listStyle);
        break;
    default:
        counterText = QString::number(index);
    }

    return counterText;
}

QList<ListStyleItem> Lists::genericListStyleItems()
{
    QList<ListStyleItem> answer;
    answer.append(ListStyleItem(i18nc("Text list-style", "None"), KoListStyle::None));
    answer.append(ListStyleItem(i18n("Small Bullet"), KoListStyle::Bullet));
    answer.append(ListStyleItem(i18n("Circle Bullet"), KoListStyle::CircleItem));
    answer.append(ListStyleItem(i18n("Square Bullet"), KoListStyle::SquareItem));
    answer.append(ListStyleItem(i18n("Rhombus Bullet"), KoListStyle::RhombusItem));
    answer.append(ListStyleItem(i18n("Check Mark Bullet"), KoListStyle::HeavyCheckMarkItem));
    answer.append(ListStyleItem(i18n("Rightwards Arrow Bullet"), KoListStyle::RightArrowItem));
    answer.append(ListStyleItem(i18n("Arabic"), KoListStyle::DecimalItem));
    answer.append(ListStyleItem(i18n("Lower Alphabetical"), KoListStyle::AlphaLowerItem));
    answer.append(ListStyleItem(i18n("Upper Alphabetical"), KoListStyle::UpperAlphaItem));
    answer.append(ListStyleItem(i18n("Lower Roman"), KoListStyle::RomanLowerItem));
    answer.append(ListStyleItem(i18n("Upper Roman"), KoListStyle::UpperRomanItem));
    return answer;
}

QList<ListStyleItem> Lists::otherListStyleItems()
{
    QList<ListStyleItem> answer;
    answer.append(ListStyleItem(i18n("Large Bullet"), KoListStyle::BlackCircle));
    answer.append(ListStyleItem(i18n("Ballot X Bullet"), KoListStyle::BallotXItem));
    answer.append(ListStyleItem(i18n("Rightwards Arrow Head Bullet"), KoListStyle::RightArrowHeadItem));
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
    answer.append(ListStyleItem(i18n("Image"), KoListStyle::ImageItem));
    return answer;
}

// ------------------- ListItemsHelper ------------
/// \internal helper class for calculating text-lists prefixes and indents
ListItemsHelper::ListItemsHelper(QTextList *textList, const QFont &font)
        : m_textList(textList)
        , m_fm(font, textList->document()->documentLayout()->paintDevice())
{
}

void ListItemsHelper::recalculateBlock(QTextBlock &block)
{
    //warnTextLayout;
    const QTextListFormat format = m_textList->format();
    const KoListStyle::Style listStyle = static_cast<KoListStyle::Style>(format.style());

    const QString prefix = format.stringProperty(KoListStyle::ListItemPrefix);
    const QString suffix = format.stringProperty(KoListStyle::ListItemSuffix);
    const int level = format.intProperty(KoListStyle::Level);
    int dp = format.intProperty(KoListStyle::DisplayLevel);
    if (dp > level)
        dp = level;
    const int displayLevel = dp ? dp : 1;

    QTextBlockFormat blockFormat = block.blockFormat();

    // Look if we have a block that is inside a header. We need to special case them cause header-lists are
    // different from any other kind of list and they do build up there own global list (table of content).
    bool isOutline = blockFormat.intProperty(KoParagraphStyle::OutlineLevel) > 0;

    int startValue = 1;
    if (format.hasProperty(KoListStyle::StartValue))
        startValue = format.intProperty(KoListStyle::StartValue);

    int index = startValue;
    bool fixed = false;
    if (blockFormat.boolProperty(KoParagraphStyle::RestartListNumbering)) {
        index = format.intProperty(KoListStyle::StartValue);
        fixed = true;
    }
    const int paragIndex = blockFormat.intProperty(KoParagraphStyle::ListStartValue);
    if (paragIndex > 0) {
        index = paragIndex;
        fixed = true;
    }

    if (!fixed) {
        //if this is the first item then find if the list has to be continued from any other list
        KoList *listContinued = 0;
        if (m_textList->itemNumber(block) == 0 && KoTextDocument(m_textList->document()).list(m_textList) && (listContinued = KoTextDocument(m_textList->document()).list(m_textList)->listContinuedFrom())) {
            //find the previous list of the same level
            QTextList *previousTextList = listContinued->textLists().at(level - 1).data();
            if (previousTextList) {
                QTextBlock textBlock = previousTextList->item(previousTextList->count() - 1);
                if (textBlock.isValid()) {
                    index = KoTextBlockData(textBlock).counterIndex() + 1; //resume the previous list count
                }
            }
        } else if (m_textList->itemNumber(block) > 0) {
            QTextBlock textBlock = m_textList->item(m_textList->itemNumber(block) - 1);
            if (textBlock.isValid()) {
                index = KoTextBlockData(textBlock).counterIndex() + 1; //resume the previous list count
            }
        }
    }

    qreal width = 0.0;
    KoTextBlockData blockData(block);

    if (blockFormat.boolProperty(KoParagraphStyle::UnnumberedListItem)
        || blockFormat.boolProperty(KoParagraphStyle::IsListHeader)) {
        blockData.setCounterPlainText(QString());
        blockData.setCounterPrefix(QString());
        blockData.setCounterSuffix(QString());
        blockData.setPartialCounterText(QString());
        // set the counter for the current un-numbered list to the counter index of the previous list item.
        // index-1 because the list counter would have already incremented by one
        blockData.setCounterIndex(index - 1);
        if (blockFormat.boolProperty(KoParagraphStyle::IsListHeader)) {
            blockData.setCounterWidth(format.doubleProperty(KoListStyle::MinimumWidth));
            blockData.setCounterSpacing(0);
        }
        return;
    }

    QString item;
    if (displayLevel > 1) {
        int checkLevel = level;
        int tmpDisplayLevel = displayLevel;
        bool counterResetRequired = true;
        for (QTextBlock b = block.previous(); tmpDisplayLevel > 1 && b.isValid(); b = b.previous()) {
            if (b.textList() == 0)
                continue;
            QTextListFormat lf = b.textList()->format();
            if (lf.property(KoListStyle::StyleId) != format.property(KoListStyle::StyleId))
               continue; // uninteresting for us
            if (isOutline != bool(b.blockFormat().intProperty(KoParagraphStyle::OutlineLevel)))
                continue; // also uninteresting cause the one is an outline-listitem while the other is not

            if (! KoListStyle::isNumberingStyle(static_cast<KoListStyle::Style>(lf.style()))) {
                continue;
            }

            if (b.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem)) {
                continue; //unnumbered listItems are irrelevant
            }

            const int otherLevel  = lf.intProperty(KoListStyle::Level);
            if (isOutline && checkLevel == otherLevel) {
                counterResetRequired = false;
            }

            if (checkLevel <= otherLevel)
                continue;

            KoTextBlockData otherData(b);
            if (!otherData.hasCounterData()) {
                continue;
            }
            if (tmpDisplayLevel - 1 < otherLevel) { // can't just copy it fully since we are
                // displaying less then the full counter
                item += otherData.partialCounterText();
                tmpDisplayLevel--;
                checkLevel--;
                for (int i = otherLevel + 1; i < level; i++) {
                    tmpDisplayLevel--;
                    item += "." + intToNumberingStyle(index, listStyle,
                                                      m_textList->format().boolProperty(KoListStyle::LetterSynchronization)); // add missing counters.
                }
            } else { // just copy previous counter as prefix
                QString otherPrefix = lf.stringProperty(KoListStyle::ListItemPrefix);
                QString otherSuffix = lf.stringProperty(KoListStyle::ListItemSuffix);
                QString pureCounter = otherData.counterText().mid(otherPrefix.size());
                pureCounter = pureCounter.left(pureCounter.size() - otherSuffix.size());
                item += pureCounter;
                for (int i = otherLevel + 1; i < level; i++)
                    item += "." + intToNumberingStyle(index, listStyle,
                                                      m_textList->format().boolProperty(KoListStyle::LetterSynchronization)); // add missing counters.
                tmpDisplayLevel = 0;
                if (isOutline && counterResetRequired) {
                    index = 1;
                }
                break;
            }
        }
        for (int i = 1; i < tmpDisplayLevel; i++)
            item = intToNumberingStyle(index, listStyle,
                                       m_textList->format().boolProperty(KoListStyle::LetterSynchronization))
                    + "." + item; // add missing counters.
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
    if (KoListStyle::isNumberingStyle(listStyle)) {
        partialCounterText = intToNumberingStyle(index, listStyle,
                                m_textList->format().boolProperty(KoListStyle::LetterSynchronization));
    } else {
        switch (listStyle) {
        case KoListStyle::SquareItem:
        case KoListStyle::Bullet:
        case KoListStyle::BlackCircle:
        case KoListStyle::DiscItem:
        case KoListStyle::CircleItem:
        case KoListStyle::HeavyCheckMarkItem:
        case KoListStyle::BallotXItem:
        case KoListStyle::RightArrowItem:
        case KoListStyle::RightArrowHeadItem:
        case KoListStyle::RhombusItem:
        case KoListStyle::BoxItem: {
            calcWidth = false;
            if (format.intProperty(KoListStyle::BulletCharacter))
                item = QString(QChar(format.intProperty(KoListStyle::BulletCharacter)));
            width = m_fm.width(item);
            int percent = format.intProperty(KoListStyle::RelativeBulletSize);
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
            width =  0.0;
            break;
        case KoListStyle::ImageItem:
            calcWidth = false;
            width = qMax(format.doubleProperty(KoListStyle::Width), (qreal)1.0);
            break;
        default:  // others we ignore.
            calcWidth = false;
        }
    }

    blockData.setCounterIsImage(listStyle == KoListStyle::ImageItem);
    blockData.setPartialCounterText(partialCounterText);
    blockData.setCounterIndex(index);
    item += partialCounterText;
    blockData.setCounterPlainText(item);
    blockData.setCounterPrefix(prefix);
    blockData.setCounterSuffix(suffix);
    if (calcWidth)
        width = m_fm.width(item);
    index++;

    width += m_fm.width(prefix + suffix);

    qreal counterSpacing = 0;
    if (format.boolProperty(KoListStyle::AlignmentMode)) {
        // for AlignmentMode spacing should be 0
        counterSpacing = 0;
    } else {
        if (listStyle != KoListStyle::None) {
            // see ODF spec 1.2 item 20.422
            counterSpacing = format.doubleProperty(KoListStyle::MinimumDistance);
            if (width < format.doubleProperty(KoListStyle::MinimumWidth)) {
                counterSpacing -= format.doubleProperty(KoListStyle::MinimumWidth) - width;
            }
            counterSpacing = qMax(counterSpacing, qreal(0.0));
        }
        width = qMax(width, format.doubleProperty(KoListStyle::MinimumWidth));
    }
    blockData.setCounterWidth(width);
    blockData.setCounterSpacing(counterSpacing);
    //warnTextLayout;
}

// static
bool ListItemsHelper::needsRecalc(QTextList *textList)
{
    Q_ASSERT(textList);
    QTextBlock tb = textList->item(0);
    KoTextBlockData blockData(tb);
    return !blockData.hasCounterData();
}
