/*
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoCssTextUtils.h"
#include "graphemebreak.h"
#include <QChar>

QVector<QPair<int, int>> positionDifference(QStringList a, QStringList b) {
    QVector<QPair<int, int>> positions;

    int countA = 0;
    int countB = 0;
    for (int i=0; i< a.size(); i++) {
        QString textA = a.at(i);
        QString textB = b.at(i);
        if (textA.size() > textB.size()) {
            for (int j=0; j < textA.size(); j++) {
                int k = j < textB.size()? countB+j: -1;
                positions.append(QPair(countA+j, k));
            }
        } else {
            for (int j=0; j < textB.size(); j++) {
                int k = j < textA.size()? countA+j: -1;
                positions.append(QPair(k, countB+j));
            }
        }
        countA += textA.size();
        countB += textB.size();
    }

    return positions;
}

QString KoCssTextUtils::transformTextToUpperCase(const QString &text, const QString &langCode, QVector<QPair<int, int> > &positions)
{
    QLocale locale(langCode.split("-").join("_"));
    QString transformedText = locale.toUpper(text);
    positions = positionDifference(textToUnicodeGraphemeClusters(text, langCode), textToUnicodeGraphemeClusters(transformedText, langCode));
    return transformedText;
}

QString KoCssTextUtils::transformTextToLowerCase(const QString &text, const QString &langCode, QVector<QPair<int, int> > &positions)
{
    QLocale locale(langCode.split("-").join("_"));
    QString transformedText = locale.toLower(text);
    positions = positionDifference(textToUnicodeGraphemeClusters(text, langCode), textToUnicodeGraphemeClusters(transformedText, langCode));
    return transformedText;
};

QString KoCssTextUtils::transformTextCapitalize(const QString &text, const QString langCode, QVector<QPair<int, int>> &positions)
{
    QLocale locale(langCode);

    QStringList graphemes = textToUnicodeGraphemeClusters(text, langCode);
    QStringList oldGraphemes = graphemes;
    bool capitalizeGrapheme = true;
    for (int i = 0; i < graphemes.size(); i++) {
        QString grapheme = graphemes.at(i);
        if (grapheme.isEmpty() || IsCssWordSeparator(grapheme)) {
            capitalizeGrapheme = true;

        } else if (capitalizeGrapheme) {
            graphemes[i] = locale.toUpper(grapheme);
            if (i + 1 < graphemes.size()) {
                /// While this is the only case I know of, make no mistake,
                /// "IJsbeer" (Polar bear) is much more readable than "Ijsbeer".
                if (locale == QLocale::Dutch && grapheme.toLower().startsWith("i") && graphemes.at(i + 1).toLower().startsWith("j")) {
                    capitalizeGrapheme = true;
                    continue;
                }
            }
            capitalizeGrapheme = false;
        }
    }

    positions = positionDifference(oldGraphemes, graphemes);
    return graphemes.join("");
}

static QChar findProportionalToFullWidth(const QChar &value, const QChar &defaultValue)
{
    static QMap<QChar, QChar> map = []() {
        QMap<QChar, QChar> map;
        // https://stackoverflow.com/questions/8326846/
        for (int i = 0x0021; i < 0x007F; i++) {
            map.insert(QChar(i), QChar(i + 0xFF00 - 0x0020));
        }
        map.insert(QChar(0x0020), QChar(0x3000)); // Ideographic space.

        return map;
    }();

    return map.value(value, defaultValue);
}

QString KoCssTextUtils::transformTextFullWidth(const QString &text)
{
    QString transformedText;
    Q_FOREACH (const QChar &c, text) {
        if (c.decompositionTag() == QChar::Narrow) {
            transformedText.append(c.decomposition());
        } else {
            transformedText.append(findProportionalToFullWidth(c, c));
        }
    }

    return transformedText;
}

static QChar findSmallKanaToBigKana(const QChar &value, const QChar &defaultValue)
{
    static QMap<QChar, QChar> map = {
        // NOTE: these are not fully sequential!
        // clang-format off
        {{0x3041}, {0x3042}},
        {{0x3043}, {0x3044}},
        {{0x3045}, {0x3046}},
        {{0x3047}, {0x3048}},
        {{0x3049}, {0x304A}},
        {{0x3095}, {0x304B}},
        {{0x3096}, {0x3051}},
        {{0x3063}, {0x3064}},
        {{0x3083}, {0x3084}},
        {{0x3085}, {0x3086}},
        {{0x3087}, {0x3088}},
        {{0x308E}, {0x308F}},

        {{0x30A1}, {0x30A2}},
        {{0x30A3}, {0x30A4}},
        {{0x30A5}, {0x30A6}},
        {{0x30A7}, {0x30A8}},
        {{0x30A9}, {0x30AA}},
        {{0x30F5}, {0x30AB}},
        {{0x31F0}, {0x30AF}},
        {{0x30F6}, {0x30B1}},
        {{0x31F1}, {0x30B7}},
        {{0x31F2}, {0x30B9}},
        {{0x30C3}, {0x30C4}},
        {{0x31F3}, {0x30C8}},
        {{0x31F4}, {0x30CC}},
        {{0x31F5}, {0x30CF}},
        {{0x31F6}, {0x30D2}},
        {{0x31F7}, {0x30D5}},
        {{0x31F8}, {0x30D8}},
        {{0x31F9}, {0x30DB}},
        {{0x31FA}, {0x30E0}},
        {{0x30E3}, {0x30E4}},
        {{0x30E5}, {0x30E6}},
        {{0x30E7}, {0x30E8}},
        {{0x31FB}, {0x30E9}},
        {{0x31FC}, {0x30EA}},
        {{0x31FD}, {0x30EB}},
        {{0x31FE}, {0x30EC}},
        {{0x31FF}, {0x30ED}},
        {{0x30EE}, {0x30EF}},

        {{0xFF67}, {0xFF71}},
        {{0xFF68}, {0xFF72}},
        {{0xFF69}, {0xFF73}},
        {{0xFF6A}, {0xFF74}},
        {{0xFF6B}, {0xFF75}},
        {{0xFF6F}, {0xFF82}},
        {{0xFF6C}, {0xFF94}},
        {{0xFF6D}, {0xFF95}},
        {{0xFF6E}, {0xFF96}},
        // clang-format on
    };

    return map.value(value, defaultValue);
}

QString KoCssTextUtils::transformTextFullSizeKana(const QString &text)
{
    QString transformedText;
    Q_FOREACH (const QChar &c, text) {
        transformedText.append(findSmallKanaToBigKana(c, c));
    }

    return transformedText;
}

QVector<bool> KoCssTextUtils::collapseSpaces(QString *text, KoSvgText::TextSpaceCollapse collapseMethod)
{
    QString modifiedText = *text;
    QVector<bool> collapseList(modifiedText.size());
    collapseList.fill(false);
    int spaceSequenceCount = 0;
    for (int i = 0; i < modifiedText.size(); i++) {
        bool firstOrLast = (i == 0 || i == modifiedText.size() - 1);
        bool collapse = false;
        const QChar c = modifiedText.at(i);
        if (c == QChar::LineFeed || c == QChar::Tabulation) {
            if (collapseMethod == KoSvgText::Collapse ||
                    collapseMethod == KoSvgText::PreserveSpaces) {
                modifiedText[i] = QChar::Space;
                spaceSequenceCount += 1;
                collapseList[i] = spaceSequenceCount > 1 || firstOrLast? true: false;
                continue;
            }
        }
        if (c.isSpace()) {
            bool isSegmentBreak = c == QChar::LineFeed;
            bool isTab = c == QChar::Tabulation;
            spaceSequenceCount += 1;
            if (spaceSequenceCount > 1 || firstOrLast) {
                switch (collapseMethod) {
                case KoSvgText::Collapse:
                case KoSvgText::Discard:
                    collapse = true;
                    break;
                case KoSvgText::Preserve:
                case KoSvgText::PreserveSpaces:
                    collapse = false;
                    break;
                case KoSvgText::PreserveBreaks:
                    collapse = !isSegmentBreak;
                    if (isTab) {
                        modifiedText[i] = QChar::Space;
                    }
                    break;
                }
            }
        } else {
            spaceSequenceCount = 0;
        }
        collapseList[i] = collapse;
    }
    // go backward to ensure any dangling space characters are marked as collapsed.
    for (int i = modifiedText.size()-1; i>=0; i--) {
        if (modifiedText.at(i).isSpace()) {
            if (collapseMethod == KoSvgText::Collapse) {
                collapseList[i] = true;
            }
        } else {
            break;
        }
    }
    *text = modifiedText;
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
            collapse = true;
            break;
        case KoSvgText::PreserveSpaces:
            collapse = false;
            break;
        }
    }
    return collapse;
}

bool KoCssTextUtils::hangLastSpace(const QChar c,
                                   KoSvgText::TextSpaceCollapse collapseMethod,
                                   KoSvgText::TextWrap wrapMethod,
                                   bool &force,
                                   bool nextCharIsHardBreak)
{
    if (c.isSpace()) {
        if (collapseMethod == KoSvgText::Collapse || collapseMethod == KoSvgText::PreserveBreaks) {
            // [css-text-3] white-space is set to normal, nowrap, or pre-line; or
            // [css-text-4] white-space-collapse is collapse or preserve-breaks:
            // hang unconditionally.
            force = true;
            return true;
        } else if (collapseMethod == KoSvgText::Preserve && wrapMethod != KoSvgText::NoWrap) {
            // [css-text-3] white-space is set to pre-wrap; or
            // [css-text-4] white-space-collapse is preserve and text-wrap is not nowrap:
            // hang unconditionally, unless followed by a force line break,
            // in which case conditionally hang.

            if (nextCharIsHardBreak) {
                force = false;
            } else {
                force = true;
            }
            return true;
        }
    }

    return false;
}

bool KoCssTextUtils::characterCanHang(const QChar c, KoSvgText::HangingPunctuations hangType)
{
    if (hangType.testFlag(KoSvgText::HangFirst)) {
        if (c.category() == QChar::Punctuation_InitialQuote || // Pi
            c.category() == QChar::Punctuation_Open || // Ps
            c.category() == QChar::Punctuation_FinalQuote || // Pf
            c == "\u0027" || // Apostrophe
            c == "\uFF07" || // Fullwidth Apostrophe
            c == "\u0022" || // Quotation Mark
            c == "\uFF02") { // Fullwidth Quotation Mark
            return true;
        }
    }
    if (hangType.testFlag(KoSvgText::HangLast)) {
        if (c.category() == QChar::Punctuation_InitialQuote || // Pi
            c.category() == QChar::Punctuation_FinalQuote || // Pf
            c.category() == QChar::Punctuation_Close || // Pe
            c == "\u0027" || // Apostrophe
            c == "\uFF07" || // Fullwidth Apostrophe
            c == "\u0022" || // Quotation Mark
            c == "\uFF02") { // Fullwidth Quotation Mark
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
            c == "\uff64" // halfwidth ideographic comma
        ) {
            return true;
        }
    }
    return false;
}

bool KoCssTextUtils::IsCssWordSeparator(const QString grapheme)
{
    return (grapheme == "\u0020" || // Space
        grapheme == "\u00A0" || // No Break Space
        grapheme == "\u1361" || // Ethiopic Word Space
        grapheme == "\u10100" || // Aegean Word Sepator Line
        grapheme == "\u10101" || // Aegean Word Sepator Dot
        grapheme == "\u1039F");
}

QStringList KoCssTextUtils::textToUnicodeGraphemeClusters(const QString &text, const QString &langCode)
{
    QVector<char> graphemeBreaks(text.size());
    set_graphemebreaks_utf16(text.utf16(), static_cast<size_t>(text.size()), langCode.toUtf8().data(), graphemeBreaks.data());
    QStringList graphemes;
    int graphemeLength = 0;
    int lastGrapheme = 0;
    for (int i = 0; i < text.size(); i++) {
        graphemeLength += 1;
        bool breakGrapheme = lastGrapheme + graphemeLength < text.size() ? graphemeBreaks[i] == GRAPHEMEBREAK_BREAK : false;
        if (breakGrapheme) {
            graphemes.append(text.mid(lastGrapheme, graphemeLength));
            lastGrapheme += graphemeLength;
            graphemeLength = 0;
        }
    }
    graphemes.append(text.mid(lastGrapheme, text.size() - lastGrapheme));
    return graphemes;
}

static QVector<QChar::Script> blockScript {
    QChar::Script_Bopomofo,
    QChar::Script_Han,
    QChar::Script_Hangul,
    QChar::Script_Hiragana,
    QChar::Script_Katakana,
    QChar::Script_Yi
};

static QVector<QChar::Script> clusterScript {
    QChar::Script_Khmer,
    QChar::Script_Lao,
    QChar::Script_Myanmar,
    QChar::Script_NewTaiLue,
    QChar::Script_TaiLe,
    QChar::Script_TaiTham,
    QChar::Script_TaiViet,
    QChar::Script_Thai
};

QVector<QPair<bool, bool> > KoCssTextUtils::justificationOpportunities(QString text, QString langCode)
{
    QVector<QPair<bool, bool>> opportunities(text.size());
    opportunities.fill(QPair<bool, bool>(false, false));
    QStringList graphemes = textToUnicodeGraphemeClusters(text, langCode);
    for (int i = 0; i < graphemes.size(); i++) {
        QString grapheme = graphemes.at(i);
        if (IsCssWordSeparator(grapheme) || blockScript.contains(grapheme.at(0).script())
                || clusterScript.contains(grapheme.at(0).script())) {
            opportunities[i] = QPair<bool, bool>(true, true);
        }
    }
    return opportunities;
}
