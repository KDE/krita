/*
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoCssTextUtils.h"
#include <QChar>

QVector<bool> KoCssTextUtils::collapseSpaces(QString &text, KoSvgText::TextSpaceCollapse collapseMethod)
{
    QVector<bool> collapseList(text.size());
    collapseList.fill(false);
    int spaceSequenceCount = 0;
    for(int i=0; i<text.size(); i++) {
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
                    //if (isTab) {text[i] = QChar::Space;}
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
            c == "\u0027" ||
            c == "\u0022" ) {
        return true;
        }
    }
    if (hangType.testFlag(KoSvgText::HangLast)) {
        if (c.category() == QChar::Punctuation_InitialQuote || //Pi
            c.category() == QChar::Punctuation_FinalQuote || //Pf
            c.category() == QChar::Punctuation_Close || //Pe
            c == "\u0027" ||
            c == "\u0022" ) {
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
