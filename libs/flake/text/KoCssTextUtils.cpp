/*
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoCssTextUtils.h"
#include "graphemebreak.h"
#include <QChar>
#include <kis_assert.h>

QVector<QPair<int, int>> positionDifference(QStringList a, QStringList b) {
    QVector<QPair<int, int>> positions;
    if (a.isEmpty() && b.isEmpty()) return positions;

    int countA = 0;
    int countB = 0;
    for (int i=0; i< a.size(); i++) {
        QString textA = a.at(i);
        QString textB = b.at(i);
        if (textA.size() > textB.size()) {
            for (int j=0; j < textA.size(); j++) {
                int k = j < textB.size()? countB+j: -1;
                positions.append(qMakePair(countA+j, k));
            }
        } else {
            for (int j=0; j < textB.size(); j++) {
                int k = j < textA.size()? countA+j: -1;
                positions.append(qMakePair(k, countB+j));
            }
        }
        countA += textA.size();
        countB += textB.size();
    }

    return positions;
}

QString KoCssTextUtils::transformTextToUpperCase(const QString &text, const QString &langCode, QVector<QPair<int, int> > &positions)
{
    if (text.isEmpty()) return text;
    QLocale locale(langCode.split("-").join("_"));
    QString transformedText = locale.toUpper(text);
    positions = positionDifference(textToUnicodeGraphemeClusters(text, langCode), textToUnicodeGraphemeClusters(transformedText, langCode));
    return transformedText;
}

QString KoCssTextUtils::transformTextToLowerCase(const QString &text, const QString &langCode, QVector<QPair<int, int> > &positions)
{
    if (text.isEmpty()) return text;
    QLocale locale(langCode.split("-").join("_"));
    QString transformedText = locale.toLower(text);
    positions = positionDifference(textToUnicodeGraphemeClusters(text, langCode), textToUnicodeGraphemeClusters(transformedText, langCode));
    return transformedText;
}

QString KoCssTextUtils::transformTextCapitalize(const QString &text, const QString langCode, QVector<QPair<int, int>> &positions)
{
    if (text.isEmpty()) return text;
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
    if (text.isEmpty()) return text;
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
        {QChar{0x3041}, QChar{0x3042}},
        {QChar{0x3043}, QChar{0x3044}},
        {QChar{0x3045}, QChar{0x3046}},
        {QChar{0x3047}, QChar{0x3048}},
        {QChar{0x3049}, QChar{0x304A}},
        {QChar{0x3095}, QChar{0x304B}},
        {QChar{0x3096}, QChar{0x3051}},
        {QChar{0x1B132}, QChar{0x3053}},
        {QChar{0x3063}, QChar{0x3064}},
        {QChar{0x3083}, QChar{0x3084}},
        {QChar{0x3085}, QChar{0x3086}},
        {QChar{0x3087}, QChar{0x3088}},
        {QChar{0x308E}, QChar{0x308F}},
        {QChar{0x1B150}, QChar{0x3090}},
        {QChar{0x1B151}, QChar{0x3091}},
        {QChar{0x1B152}, QChar{0x3092}},

        {QChar{0x30A1}, QChar{0x30A2}},
        {QChar{0x30A3}, QChar{0x30A4}},
        {QChar{0x30A5}, QChar{0x30A6}},
        {QChar{0x30A7}, QChar{0x30A8}},
        {QChar{0x30A9}, QChar{0x30AA}},
        {QChar{0x30F5}, QChar{0x30AB}},
        {QChar{0x31F0}, QChar{0x30AF}},
        {QChar{0x30F6}, QChar{0x30B1}},
        {QChar{0x1B155}, QChar{0x30B3}},
        {QChar{0x31F1}, QChar{0x30B7}},
        {QChar{0x31F2}, QChar{0x30B9}},
        {QChar{0x30C3}, QChar{0x30C4}},
        {QChar{0x31F3}, QChar{0x30C8}},
        {QChar{0x31F4}, QChar{0x30CC}},
        {QChar{0x31F5}, QChar{0x30CF}},
        {QChar{0x31F6}, QChar{0x30D2}},
        {QChar{0x31F7}, QChar{0x30D5}},
        {QChar{0x31F8}, QChar{0x30D8}},
        {QChar{0x31F9}, QChar{0x30DB}},
        {QChar{0x31FA}, QChar{0x30E0}},
        {QChar{0x30E3}, QChar{0x30E4}},
        {QChar{0x30E5}, QChar{0x30E6}},
        {QChar{0x30E7}, QChar{0x30E8}},
        {QChar{0x31FB}, QChar{0x30E9}},
        {QChar{0x31FC}, QChar{0x30EA}},
        {QChar{0x31FD}, QChar{0x30EB}},
        {QChar{0x31FE}, QChar{0x30EC}},
        {QChar{0x31FF}, QChar{0x30ED}},
        {QChar{0x30EE}, QChar{0x30EF}},
        {QChar{0x1B164}, QChar{0x30F0}},
        {QChar{0x1B165}, QChar{0x30F1}},
        {QChar{0x1B166}, QChar{0x30F2}},
        {QChar{0x1B167}, QChar{0x30F3}},

        {QChar{0xFF67}, QChar{0xFF71}},
        {QChar{0xFF68}, QChar{0xFF72}},
        {QChar{0xFF69}, QChar{0xFF73}},
        {QChar{0xFF6A}, QChar{0xFF74}},
        {QChar{0xFF6B}, QChar{0xFF75}},
        {QChar{0xFF6F}, QChar{0xFF82}},
        {QChar{0xFF6C}, QChar{0xFF94}},
        {QChar{0xFF6D}, QChar{0xFF95}},
        {QChar{0xFF6E}, QChar{0xFF96}},
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

QVector<bool> KoCssTextUtils::collapseSpaces(QString *text, QMap<int, KoSvgText::TextSpaceCollapse> collapseMethods)
{

    QString modifiedText = *text;
    QVector<bool> collapseList(modifiedText.size());
    collapseList.fill(false);
    int spaceSequenceCount = 0;
    KoSvgText::TextSpaceCollapse collapseMethod = collapseMethods.first();
    for (int i = 0; i < modifiedText.size(); i++) {
        bool firstOrLast = (i == 0 || i == modifiedText.size() - 1);
        bool collapse = false;
        if (collapseMethods.contains(i)) {
            // If a whitespace:collapse sequence is inside a pre-wrap sequence, the first space needs to be left alone.
            if (collapseMethods.value(i) != collapseMethod) spaceSequenceCount = 0;
            collapseMethod = collapseMethods.value(i);
        }
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
                case KoSvgText::BreakSpaces:
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
        if (collapseMethods.contains(i)) {
            int pos = collapseMethods.keys().indexOf(i);
            collapseMethod = collapseMethods.value(collapseMethods.keys().value(pos-1, 0), KoSvgText::Collapse);
            if (collapseMethod != KoSvgText::Collapse) break;
        }
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
        case KoSvgText::BreakSpaces:
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

const QString BIDI_CONTROL_LRE = "\u202a";
const QString BIDI_CONTROL_RLE = "\u202b";
const QString BIDI_CONTROL_PDF = "\u202c";
const QString BIDI_CONTROL_LRO = "\u202d";
const QString BIDI_CONTROL_RLO = "\u202e";
const QString BIDI_CONTROL_LRI = "\u2066";
const QString BIDI_CONTROL_RLI = "\u2067";
const QString BIDI_CONTROL_FSI = "\u2068";
const QString BIDI_CONTROL_PDI = "\u2069";
const QString UNICODE_BIDI_ISOLATE_OVERRIDE_LR_START = "\u2068\u202d";
const QString UNICODE_BIDI_ISOLATE_OVERRIDE_RL_START = "\u2068\u202e";
const QString UNICODE_BIDI_ISOLATE_OVERRIDE_END = "\u202c\u2069";
const QChar ZERO_WIDTH_JOINER = QChar{0x200d};

QString KoCssTextUtils::getBidiOpening(bool ltr, KoSvgText::UnicodeBidi bidi)
{
    using namespace KoSvgText;

    QString result;

    if (ltr) {
        if (bidi == BidiEmbed) {
            result = BIDI_CONTROL_LRE;
        } else if (bidi == BidiOverride) {
            result = BIDI_CONTROL_LRO;
        } else if (bidi == BidiIsolate) {
            result = BIDI_CONTROL_LRI;
        } else if (bidi == BidiIsolateOverride) {
            result = UNICODE_BIDI_ISOLATE_OVERRIDE_LR_START;
        } else if (bidi == BidiPlainText) {
            result = BIDI_CONTROL_FSI;
        }
    } else {
        if (bidi == BidiEmbed) {
            result = BIDI_CONTROL_RLE;
        } else if (bidi == BidiOverride) {
            result = BIDI_CONTROL_RLO;
        } else if (bidi == BidiIsolate) {
            result = BIDI_CONTROL_RLI;
        } else if (bidi == BidiIsolateOverride) {
            result = UNICODE_BIDI_ISOLATE_OVERRIDE_RL_START;
        } else if (bidi == BidiPlainText) {
            result = BIDI_CONTROL_FSI;
        }
    }

    return result;
}

QString KoCssTextUtils::getBidiClosing(KoSvgText::UnicodeBidi bidi)
{
    using namespace KoSvgText;

    QString result;

    if (bidi == BidiEmbed || bidi == BidiOverride) {
        result = BIDI_CONTROL_PDF;
    } else if (bidi == BidiIsolate || bidi == BidiPlainText) {
        result = BIDI_CONTROL_PDI;
    } else if (bidi == BidiIsolateOverride) {
        result = UNICODE_BIDI_ISOLATE_OVERRIDE_END;
    }

    return result;
}

bool isVariationSelector(uint val) {
    // Original set of VS
    if (val == 0xfe00 || (val > 0xfe00 && val <= 0xfe0f)) {
        return true;
    }
    // Extended set VS
    if (val == 0xe0100 || (val > 0xe0100 && val <= 0xe01ef)) {
        return true;
    }
    // Mongolian VS
    if (val == 0x180b || (val > 0x180b && val <= 0x180f)) {
        return true;
    }
    // Emoji skin tones
    if (val == 0x1f3fb || (val > 0x1f3fb && val <= 0x1f3ff)) {
        return true;
    }
    return false;
}

bool regionalIndicator(uint val) {
    if (val == 0x1f1e6 || (val > 0x1f1e6 && val <= 0x1f1ff)) {
        return true;
    }
    return false;
}

void KoCssTextUtils::removeText(QString &text, int &start, int length)
{
    int end = start+length;
    int j = 0;
    int v = 0;
    int lastCharZWJ = 0;
    int lastVS = 0;
    int vsClusterStart = 0;
    int regionalIndicatorCount = 0;
    bool startFound = false;
    bool addToEnd = true;
    Q_FOREACH(const uint i, text.toUcs4()) {
        v = QChar::requiresSurrogates(i)? 2: 1;
        int index = (j+v) -1;
        bool ZWJ = text.at(index) == ZERO_WIDTH_JOINER;
        if (isVariationSelector(i)) {
            lastVS += v;
        } else {
            lastVS = 0;
            vsClusterStart = j;
        }
        if (index >= start && !startFound) {
            startFound = true;
            if (v > 1) {
                start = j;
            }
            if (regionalIndicatorCount > 0 && regionalIndicator(i)) {
                start -= regionalIndicatorCount;
                regionalIndicatorCount = 0;
            }
            // Always delete any zero-width-joiners as well.
            if (ZWJ && index > start) {
                start = -1;
            }
            if (lastCharZWJ > 0) {
                start -= lastCharZWJ;
                lastCharZWJ = 0;
            }
            // remove any clusters too.
            if (lastVS > 0) {
                start = vsClusterStart;
            }
        }


        if (j >= end && addToEnd) {
            end = j;
            addToEnd =  ZWJ || isVariationSelector(i)
                    || (regionalIndicatorCount < 3 && regionalIndicator(i));
            if (addToEnd) {
                end += v;
            }
        }
        j += v;
        lastCharZWJ = ZWJ? lastCharZWJ + v: 0;
        regionalIndicatorCount = regionalIndicator(i)? regionalIndicatorCount + v: 0;
    }
    text.remove(start, end-start);
}

qreal KoCssTextUtils::cssSelectFontStyleValue(const QVector<qreal> &values, const qreal targetValue, const qreal defaultValue, const qreal defaultValueUpper, const bool shouldNotReturnDefault)
{
    if(values.isEmpty()) {
        return targetValue;
    }
    // follow the CSS Fonts selection mechanism.
    // See https://drafts.csswg.org/css-fonts-4/#font-style-matching
    QVector<qreal> sortedValues = values;
    std::sort(sortedValues.begin(), sortedValues.end());
    qreal selectedValue = defaultValue;
    auto upper = std::lower_bound(sortedValues.begin(), sortedValues.end(), targetValue);
    if (upper == sortedValues.end()) {
        upper--;
    }
    auto lower = upper;
    if (lower != sortedValues.begin() && *lower > targetValue) {
        lower--;
    }

    // ... Which wants to select the lower possible selection when the value is below the default.
    if (targetValue < defaultValue) {
        selectedValue = *lower;
    // ... the higher closest value when the value is higher than the default (upper bound)
    } else if (targetValue > defaultValueUpper) {
        selectedValue = *upper;
    } else {
        // ... and if the value is between the lower and upper default bounds, first higher (within bounds)
        // then lower, then higher.
        if (*upper <= defaultValueUpper && *lower != targetValue) {
            selectedValue = *upper;
        } else {
            selectedValue = *lower;
        }
    }
    if (qFuzzyCompare(selectedValue, defaultValue) && shouldNotReturnDefault) {
        return targetValue;
    }
    return selectedValue;
}
