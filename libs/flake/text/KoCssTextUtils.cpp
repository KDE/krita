/*
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoCssTextUtils.h"
#include "graphemebreak.h"
#include <QChar>

QString KoCssTextUtils::transformTextCapitalize(const QString &text, const QString langCode)
{
    QLocale locale(langCode.split("-").join("_"));

    QStringList graphemes = textToUnicodeGraphemeClusters(text, langCode);
    bool capitalizeGrapheme = true;
    for (int i=0; i<graphemes.size(); i++) {
        QString grapheme = graphemes.at(i);
        if (grapheme.isEmpty() ||
                IsCssWordSeparator(grapheme) ) {
            capitalizeGrapheme = true;
        } else if (capitalizeGrapheme) {
            graphemes[i] = locale.toUpper(grapheme);
            if (i+1 < graphemes.size()) {
                /// While this is the only case I know of, make no mistake,
                /// "IJsbeer" (Polar bear) is much more readable than "Ijsbeer".
                if (locale == QLocale::Dutch
                        && grapheme.toLower().startsWith("i")
                        && graphemes.at(i+1).toLower().startsWith("j")) {
                    capitalizeGrapheme = true;
                    continue;
                }
            }
            capitalizeGrapheme = false;
        }
    }

    return graphemes.join("");
}

static QMap<QChar, QChar> proportionalToFullWidth() {
    QMap<QChar, QChar> map;

    // https://stackoverflow.com/questions/8326846/
    for (int i = 0x0021; i < 0x007F; i++) {
        map.insert(QChar(i), QChar(i+0xFF00-0x0020));
    }
    map.insert(QChar(0x0020), QChar(0x3000)); // Ideographic space.

    return map;
}

QString KoCssTextUtils::transformTextFullWidth(const QString &text)
{
    QMap<QChar, QChar> map = proportionalToFullWidth();
    QString transformedText;
    for (QChar c: text) {
        if (c.decompositionTag() == QChar::Narrow) {
            transformedText.append(c.decomposition());
        } else {
            transformedText.append(map.value(c, c));
        }
    }

    return transformedText;
}

static QMap<QChar, QChar> smallKanaToBigKana() {
    QMap<QChar, QChar> map;

    // NOTE: these are not fully sequential!
    map.insert(QChar(0x3041), QChar(0x3042));
    map.insert(QChar(0x3043), QChar(0x3044));
    map.insert(QChar(0x3045), QChar(0x3046));
    map.insert(QChar(0x3047), QChar(0x3048));
    map.insert(QChar(0x3049), QChar(0x304A));
    map.insert(QChar(0x3095), QChar(0x304B));
    map.insert(QChar(0x3096), QChar(0x3051));
    map.insert(QChar(0x3063), QChar(0x3064));
    map.insert(QChar(0x3083), QChar(0x3084));
    map.insert(QChar(0x3085), QChar(0x3086));
    map.insert(QChar(0x3087), QChar(0x3088));
    map.insert(QChar(0x308E), QChar(0x308F));

    map.insert(QChar(0x30A1), QChar(0x30A2));
    map.insert(QChar(0x30A3), QChar(0x30A4));
    map.insert(QChar(0x30A5), QChar(0x30A6));
    map.insert(QChar(0x30A7), QChar(0x30A8));
    map.insert(QChar(0x30A9), QChar(0x30AA));
    map.insert(QChar(0x30F5), QChar(0x30AB));
    map.insert(QChar(0x31F0), QChar(0x30AF));
    map.insert(QChar(0x30F6), QChar(0x30B1));
    map.insert(QChar(0x31F1), QChar(0x30B7));
    map.insert(QChar(0x31F2), QChar(0x30B9));
    map.insert(QChar(0x30C3), QChar(0x30C4));
    map.insert(QChar(0x31F3), QChar(0x30C8));
    map.insert(QChar(0x31F4), QChar(0x30CC));
    map.insert(QChar(0x31F5), QChar(0x30CF));
    map.insert(QChar(0x31F6), QChar(0x30D2));
    map.insert(QChar(0x31F7), QChar(0x30D5));
    map.insert(QChar(0x31F8), QChar(0x30D8));
    map.insert(QChar(0x31F9), QChar(0x30DB));
    map.insert(QChar(0x31FA), QChar(0x30E0));
    map.insert(QChar(0x30E3), QChar(0x30E4));
    map.insert(QChar(0x30E5), QChar(0x30E6));
    map.insert(QChar(0x30E7), QChar(0x30E8));
    map.insert(QChar(0x31FB), QChar(0x30E9));
    map.insert(QChar(0x31FC), QChar(0x30EA));
    map.insert(QChar(0x31FD), QChar(0x30EB));
    map.insert(QChar(0x31FE), QChar(0x30EC));
    map.insert(QChar(0x31FF), QChar(0x30ED));
    map.insert(QChar(0x30EE), QChar(0x30EF));

    map.insert(QChar(0xFF67), QChar(0xFF71));
    map.insert(QChar(0xFF68), QChar(0xFF72));
    map.insert(QChar(0xFF69), QChar(0xFF73));
    map.insert(QChar(0xFF6A), QChar(0xFF74));
    map.insert(QChar(0xFF6B), QChar(0xFF75));
    map.insert(QChar(0xFF6F), QChar(0xFF82));
    map.insert(QChar(0xFF6C), QChar(0xFF94));
    map.insert(QChar(0xFF6D), QChar(0xFF95));
    map.insert(QChar(0xFF6E), QChar(0xFF96));

    return map;
}
QString KoCssTextUtils::transformTextFullSizeKana(const QString &text)
{
    QMap<QChar, QChar> map = smallKanaToBigKana();
    QString transformedText;
    for (QChar c: text) {
        transformedText.append(map.value(c, c));
    }

    return transformedText;
}

QVector<bool> KoCssTextUtils::collapseSpaces(QString &text, KoSvgText::TextSpaceCollapse collapseMethod)
{
    QVector<bool> collapseList(text.size());
    collapseList.fill(false);
    int spaceSequenceCount = 0;
    for(int i=0; i < text.size(); i++) {
        bool collapse = false;
        QChar c = text.at(i);
        if (c.isSpace()) {
            bool isSegmentBreak = c == QChar::LineFeed;
            bool isTab = c == QChar::Tabulation;
            spaceSequenceCount +=1;
            if (spaceSequenceCount > 1) {
                switch (collapseMethod) {
                case KoSvgText::Collapse:
                case KoSvgText::Discard:
                    collapse = true;
                    break;
                case KoSvgText::Preserve:
                    collapse = false;
                    break;
                case KoSvgText::PreserveBreaks:
                    collapse = isSegmentBreak? false: true;
                    if (isTab) {text[i] = QChar::Space;}
                    break;
                case KoSvgText::PreserveSpaces:
                    collapse = isSegmentBreak? true: false;
                    break;
                }
            }
        } else {
            spaceSequenceCount = 0;
        }
        collapseList[i] = collapse;
    }
    return collapseList;
}

bool KoCssTextUtils::collapseLastSpace(const QChar c, KoSvgText::TextSpaceCollapse collapseMethod)
{
    bool collapse = false;
    if (c == QChar::LineFeed) {
        collapse = true;
    } else if (c.isSpace()) {
        switch (collapseMethod) {
        case KoSvgText::Collapse:
        case KoSvgText::Discard:
            collapse = true;
            break;
        case KoSvgText::Preserve:
            collapse = false;
            break;
        case KoSvgText::PreserveBreaks:
            collapse =  true;
            break;
        case KoSvgText::PreserveSpaces:
            collapse = false;
            break;
        }
    }
    return collapse;
}

bool KoCssTextUtils::characterCanHang(const QChar c, KoSvgText::HangingPunctuations hangType)
{
    if (hangType.testFlag(KoSvgText::HangFirst)) {
        if (c.category() == QChar::Punctuation_InitialQuote || //Pi
            c.category() == QChar::Punctuation_Open || //Ps
            c.category() == QChar::Punctuation_FinalQuote || //Pf
            c == "\u0027" || //Apostrophe
            c == "\uFF07" || //Fullwidth Apostrophe
            c == "\u0022" || //Quotation Mark
            c == "\uFF02") { //Fullwidth Quotation Mark
        return true;
        }
    }
    if (hangType.testFlag(KoSvgText::HangLast)) {
        if (c.category() == QChar::Punctuation_InitialQuote || //Pi
            c.category() == QChar::Punctuation_FinalQuote || //Pf
            c.category() == QChar::Punctuation_Close || //Pe
            c == "\u0027" || //Apostrophe
            c == "\uFF07" || //Fullwidth Apostrophe
            c == "\u0022" || //Quotation Mark
            c == "\uFF02") { //Fullwidth Quotation Mark
        return true;
        }
    }
    if (hangType.testFlag(KoSvgText::HangEnd)) {
        if (c == "\u002c" || // Comma
            c == "\u002e" || // Full Stop
            c == "\u060c" || // Arabic Comma
            c == "\u06d4" || // Arabic Full Stop
            c == "\u3001" || // Ideographic Comma
            c == "\u3002" || // Ideographic Full Stop
            c == "\uff0c" || // Fullwidth Comma
            c == "\uff0e" || // Fullwidth full stop
            c == "\ufe50" || // Small comma
            c == "\ufe51" || // Small ideographic comma
            c == "\ufe52" || // Small full stop
            c == "\uff61" || // Halfwidth ideographic full stop
            c == "\uff64"    // halfwidth ideographic comma
            ) {
            return true;
        }
    }
    return false;
}

bool KoCssTextUtils::IsCssWordSeparator(const QString grapheme)
{
    if (grapheme == "\u0020"  ||  // Space
        grapheme == "\u00A0"  ||  // No Break Space
        grapheme == "\u1361"  ||  // Ethiopic Word Space
        grapheme == "\u10100" ||  // Aegean Word Sepator Line
        grapheme == "\u10101" ||  // Aegean Word Sepator Dot
        grapheme == "\u1039F" ) { // Ugaric Word Divider
        return true;
    }
    return false;
}

QStringList KoCssTextUtils::textToUnicodeGraphemeClusters(const QString text, const QString langCode)
{
    QVector<char> graphemeBreaks(text.size());
    set_graphemebreaks_utf16(text.utf16(),
                             static_cast<size_t>(text.size()),
                             langCode.toUtf8().data(),
                             graphemeBreaks.data());
    QStringList graphemes;
    int graphemeLength = 0;
    int lastGrapheme = 0;
    for (int i = 0; i < text.size(); i++) {
        graphemeLength += 1;
        bool breakGrapheme = lastGrapheme+graphemeLength < text.size()? graphemeBreaks[i] == GRAPHEMEBREAK_BREAK : false;
        if (breakGrapheme) {
            graphemes.append(text.mid(lastGrapheme, graphemeLength));
            lastGrapheme += graphemeLength;
            graphemeLength = 0;
        }
    }
    graphemes.append(text.mid(lastGrapheme, text.size()-lastGrapheme));
    return graphemes;
}
